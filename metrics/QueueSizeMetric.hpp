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
#include <vector>
#include <mutex>
#include "Metric.hpp"


namespace hpcc_metrics
{
    class QueueSizeMetric : public Metric
    {
        public:

            explicit QueueSizeMetric(std::string metricName) : Metric{std::move(metricName)}, m_queueSize(0) {}
            ~QueueSizeMetric() override = default;
            void addElement() { m_queueSize.fetch_add(1); }
            virtual void removeElement()
            {
                uint32_t curQueueSize = m_queueSize.load();
                uint32_t newQueueSize;
                do
                {
                    if (curQueueSize > 1)
                        newQueueSize = curQueueSize - 1;
                    else
                        break;
                } while (!m_queueSize.compare_exchange_weak(curQueueSize, newQueueSize));
            }

            bool report(std::vector<std::shared_ptr<MetricValueBase>> &values) override
            {
                values.emplace_back(std::make_shared<MetricValue<uint32_t>>(m_metricName, m_queueSize.load()));
                return true;
            }

        protected:

            std::atomic<uint32_t> m_queueSize;
    };
}
