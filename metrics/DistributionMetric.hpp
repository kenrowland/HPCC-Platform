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
#include "Metric.hpp"

namespace hpcc_metrics
{

    template <typename T>
    class DistributionMetric : public Metric
    {
        public:

            DistributionMetric(std::string &name, const std::vector<T> &bucketLevels);
            void inc(T level, uint32_t val);
            bool collect(std::vector<std::shared_ptr<MeasurementBase>> &values) override;


        protected:

            std::map<T, std::atomic<uint32_t> *> buckets;
    };


    template <typename T>
    DistributionMetric<T>::DistributionMetric(std::string &name, const std::vector<T> &bucketLevels) :
        Metric(name)
    {
        if (bucketLevels[0] != std::numeric_limits<T>::min())
            buckets[0] = new std::atomic<uint32_t>();

        for (auto const &distValue : bucketLevels)
        {
            buckets.insert(std::pair<T, std::atomic<unsigned> *>(distValue, new std::atomic<unsigned>()));
        }

        if (bucketLevels[bucketLevels.size() - 1] != std::numeric_limits<T>::max())
            buckets.insert(std::pair<T, std::atomic<uint32_t> *>(std::numeric_limits<T>::max(), new std::atomic<uint32_t>()));
    }


    template <typename T>
    void DistributionMetric<T>::inc(T level, uint32_t val)
    {
        auto it = buckets.lower_bound(level);
        if (it != buckets.end())
        {
            it.second += val;
        }
    }


    template<typename T>
    bool DistributionMetric<T>::collect(std::vector<std::shared_ptr<MeasurementBase>> &values) {
        return false;
    }
}
