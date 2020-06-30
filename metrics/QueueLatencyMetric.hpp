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
#include <queue>
#include <chrono>
#include <mutex>
#include "Metric.hpp"


namespace hpcc_metrics
{
    class QueueLatencyMetric : public Metric
    {
        public:

            explicit QueueLatencyMetric(std::string queueName) :
                Metric{std::move(queueName)},
                m_totalCompleteElements{0},
                m_totalDurationUs{0}
                {
                    m_queueNameLatency = metricName;
                    m_queueNameLatency.append("elements.latency.avg_us");
                    m_queueNameElementsProcessed = metricName;
                    m_queueNameElementsProcessed.append("elements.count");
                }

            ~QueueLatencyMetric() override = default;

            void add()
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_queue.push(std::chrono::high_resolution_clock::now());
            }


            void remove()
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                if (!m_queue.empty())
                {
                    m_totalCompleteElements++;
                    std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
                    m_totalDurationUs += std::chrono::duration_cast<std::chrono::microseconds>(now - m_queue.front());
                }
            }


            bool collect(std::vector<std::shared_ptr<MeasurementBase>> &values) override
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                uint32_t avgLatencyUs = m_totalDurationUs.count() / m_totalCompleteElements;  // may truncate a us, but should be good enough
                values.emplace_back(std::make_shared<Measurement<uint32_t>>(m_queueNameLatency, avgLatencyUs));
                values.emplace_back(std::make_shared<Measurement<uint32_t>>(m_queueNameElementsProcessed, m_totalCompleteElements));
                m_totalDurationUs = std::chrono::microseconds(0);
                m_totalCompleteElements = 0;
                return true;
            }


        protected:

            std::string m_queueNameLatency, m_queueNameElementsProcessed, m_queueNameSize;
            uint32_t m_totalCompleteElements;
            std::chrono::microseconds m_totalDurationUs;
            std::mutex m_mutex;
            std::queue<std::chrono::time_point<std::chrono::high_resolution_clock>> m_queue;
    };
}
