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
                m_count{0}
                { }

            ~RateMetric() override = default;


            void initializeForCollection() override
            {
                m_periodStart = std::chrono::high_resolution_clock::now();
            }


            void update()
            {
                //
                // For performance, update only operates on the atomic
                ++m_count;
            }


            bool report(std::vector<std::shared_ptr<MetricValueBase>> &values) override
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> now, start;
                unsigned numEvents;

                now = std::chrono::high_resolution_clock::now();
                start = m_periodStart;

                // Synchronized with update of count
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    numEvents = m_count.exchange(0);
                    m_periodStart = now;
                }


                auto seconds = (std::chrono::duration_cast<std::chrono::seconds>(now - start)).count();
                float rate = static_cast<float>(numEvents) / static_cast<float>(seconds);
                values.emplace_back(std::make_shared<MetricValue<float>>(m_metricName, rate));
                return true;
            }


        protected:

            std::mutex m_mutex;
            std::atomic<uint32_t> m_count;
            std::chrono::time_point<std::chrono::high_resolution_clock> m_periodStart;
    };
}
