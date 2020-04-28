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


#include <map>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include "Metric.hpp"
#include "MetricSink.hpp"

#include <cstdio>
#include <utility>


namespace hpcc_metrics
{

    class MetricSet
    {
        public:

            MetricSet() = default;
            virtual ~MetricSet() = default;

            bool addMetric(const std::shared_ptr<Metric> &pMetric)
            {
                auto rc = m_metrics.insert({pMetric->getName(), pMetric});
                return rc.second;
            }

            void initializeForCollection()
            {
                for (const auto& metricIt : m_metrics)
                {
                    metricIt.second->initializeForCollection();
                }
            }

            void report(std::vector<std::shared_ptr<MetricValueBase>> &values)
            {
                for (const auto &metricIt : m_metrics)
                {
                    metricIt.second->report(values);
                }
            }


        protected:

            std::map<std::string, std::shared_ptr<Metric>> m_metrics;
    };
}
