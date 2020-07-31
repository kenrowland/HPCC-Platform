/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2020 HPCC Systems®.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
############################################################################## */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <chrono>
#include <mutex>
#include <memory>
#include "IMetric.hpp"
#include "Measurement.hpp"

namespace hpccMetrics
{

class Metric : public IMetric
{
    public:

        virtual ~Metric() = default;
        const std::string &getName() const override { return metricName; }
        void setType(const std::string &_type) override { type = _type; }
        std::string getType() const override { return type; }
        bool isTyped() const override { return !type.empty(); }
        bool isInMetricSet() const override { return inMetricSet; }
        void setInMetricSet(bool val) override { inMetricSet = val; }
        void init() override { }


    protected:

        // No one should be able to create one of these
        explicit Metric(std::string name) :
                metricName(std::move(name))  { }


    protected:

        std::string metricName;
        std::string type;         // optional
        bool inMetricSet = false;
};


class CountMetric : public Metric
{
    public:

        explicit CountMetric(const std::string& name) : Metric{name} {}
        ~CountMetric() override = default;
        void inc(uint32_t val)
        {
            count.fetch_add(val);
        }

        bool collect(std::vector<std::shared_ptr<IMeasurement>> &values, const std::string &prefix) override
        {
            values.emplace_back(std::make_shared<Measurement<uint32_t>>(metricName, prefix, count.exchange(0)));
            return true;
        }

    protected:

        std::atomic<uint32_t> count{0};
};


class RateMetric : public Metric
{
    public:

        explicit RateMetric(std::string name) :
                Metric{std::move(name)}
        { }

        ~RateMetric() override = default;


        void init() override
        {
            periodStart = std::chrono::high_resolution_clock::now();
        }


        void inc(uint32_t val)
        {
            count += val;
        }


        bool collect(std::vector<std::shared_ptr<IMeasurement>> &values, const std::string &prefix) override
        {
            std::chrono::time_point<std::chrono::high_resolution_clock> now, start;
            unsigned numEvents;

            now = std::chrono::high_resolution_clock::now();
            start = periodStart;

            // Synchronize with update of count
            {
                std::unique_lock<std::mutex> lock(mutex);
                numEvents = count.exchange(0);
                periodStart = now;
            }

            auto seconds = (std::chrono::duration_cast<std::chrono::seconds>(now - start)).count();
            float rate = static_cast<float>(numEvents) / static_cast<float>(seconds);
            std::string measurementName(prefix);
            measurementName.append(metricName);
            values.emplace_back(std::make_shared<Measurement<float>>(metricName, prefix, rate));
            return true;
        }


    protected:

        std::mutex mutex;
        std::atomic<uint32_t> count{0};
        std::chrono::time_point<std::chrono::high_resolution_clock> periodStart;
};

template<typename T>
class GaugeMetric : public Metric {
    public:

        explicit GaugeMetric(std::string name) : Metric{std::move(name)} {}

        ~GaugeMetric() override = default;

        void inc(T inc)
        {
            value += inc;
        }

        void dec(T dec)
        {
            value -= dec;
        }

        bool collect(std::vector<std::shared_ptr<IMeasurement>> &values, const std::string &prefix) override
        {
            values.emplace_back(std::make_shared<Measurement<T>>(metricName, prefix, value.load()));
            return true;
        }


    protected:

        std::atomic<T> value{0};
};


// This distribution metric is still being worked out
template <typename T>
class DistributionMetric : public Metric {
    public:

        DistributionMetric(std::string &name, const std::vector<T> &bucketLevels) :
                Metric{std::move(name)} {
            if (bucketLevels[0] != std::numeric_limits<T>::min())
                buckets[0] = new std::atomic<uint32_t>();

            for (auto const &distValue : bucketLevels)
            {
                buckets.insert(std::pair<T, std::atomic<unsigned> *>(distValue, new std::atomic<unsigned>()));
            }

            if (bucketLevels[bucketLevels.size() - 1] != std::numeric_limits<T>::max())
                buckets.insert(std::pair<T, std::atomic<uint32_t> *>(std::numeric_limits<T>::max(),
                                                                     new std::atomic<uint32_t>()));
        }


        void inc(T level, uint32_t val) {
            auto it = buckets.lower_bound(level);
            if (it != buckets.end())
            {
                it.second += val;
            }
        }

        bool collect(std::vector<std::shared_ptr<IMeasurement>> &values, const std::string &prefix) override {
            return false;
        }


    protected:

        std::map<T, std::atomic<uint32_t> *> buckets;
};

}
