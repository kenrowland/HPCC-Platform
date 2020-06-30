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
#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>
#include "Metric.hpp"


namespace hpcc_metrics
{
    class RateMetric : public Metric
    {
        public:

            explicit RateMetric(std::string metricName) :
                    Metric{std::move(metricName)},
                    count{0}
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


            bool collect(std::vector<std::shared_ptr<MeasurementBase>> &values) override
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> now, start;
                unsigned numEvents;

                now = std::chrono::high_resolution_clock::now();
                start = periodStart;

                // Synchronized with update of count
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    numEvents = count.exchange(0);
                    periodStart = now;
                }


                auto seconds = (std::chrono::duration_cast<std::chrono::seconds>(now - start)).count();
                float rate = static_cast<float>(numEvents) / static_cast<float>(seconds);
                values.emplace_back(std::make_shared<Measurement<float>>(metricName, rate));
                return true;
            }


        protected:

            std::mutex mutex;
            std::atomic<uint32_t> count;
            std::chrono::time_point<std::chrono::high_resolution_clock> periodStart;
    };
}
