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

#include <memory>
#include <vector>
#include "MetricSet.hpp"
#include "MetricSink.hpp"

namespace hpcc_metrics
{
    class MetricsReporter
    {
        public:

            virtual ~MetricsReporter() = default;
            void addSink(const std::shared_ptr<MetricSink>& pSink)
            {
                m_sinks.emplace_back(pSink);
            }


            void addSink(const std::string &sinkName)
            {

            }


            void addMetricSet(const std::shared_ptr<MetricSet>& pSet)
            {
                m_metricSets.emplace_back(pSet);
            }


            virtual bool report()
            {
                //
                // Collect all the values
                std::vector<std::shared_ptr<MetricValueBase>> values;
                for (const auto &pMetricSet : m_metricSets)
                {
                    pMetricSet->report(values);
                }

                //
                // Send them
                for (const auto &pSink : m_sinks)
                {
                    pSink->send(values);
                }
                return true;
            }


        protected:

            MetricsReporter() = default;
            virtual void initializeForCollection()
            {
                //
                // Tell each metric that collection is beginning
                for (const auto& pMetricSet : m_metricSets)
                {
                    pMetricSet->initializeForCollection();
                }
            }


        protected:

            std::vector<std::shared_ptr<MetricSet>> m_metricSets;
            std::vector<std::shared_ptr<MetricSink>> m_sinks;
    };
}
