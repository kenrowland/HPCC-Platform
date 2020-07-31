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
#include <map>
#include <platform.h>
#include <dlfcn.h>
#include "IMetricSet.hpp"
#include "IMetricSink.hpp"
#include "IMetricsReporter.hpp"

typedef hpccMetrics::MetricSink* (*getMetricSinkInstance)(const std::map<std::string, std::string> &parms);

namespace hpccMetrics
{

class MetricsReporter : public IMetricsReporter
{
    public:

        virtual ~MetricsReporter() = default;


        void addSink(IMetricSink *pSink) override
        {
            sinks.emplace_back(pSink);
        }

        void addMetricSet(const std::shared_ptr<IMetricSet>& pSet) override
        {
            metricSets.emplace_back(pSet);
        }


        void init() override
        {
            //
            // Initialization consists of initializing each sink, informing
            // each sink about the metric sets for which it shall report
            // measurements, and initializing each metric set.
            for (auto *pSink : sinks)
            {
                for (const auto& pMetricSet : metricSets)
                {
                    pSink->addMetricSet(pMetricSet);
                }
                pSink->init();
            }

            //
            // Tell each metric that collection is beginning
            for (const auto& pMetricSet : metricSets)
            {
                pMetricSet->init();
            }
        }


        bool report() override
        {
            //
            // vectors of measurements for each metric set
            std::map<std::string, std::vector<std::shared_ptr<IMeasurement>>> metricSetReportValues;

            //
            // Collect all the values
            for (auto &pMetricSet : metricSets)
            {
                metricSetReportValues[pMetricSet->getName()] = std::vector<std::shared_ptr<IMeasurement>>();
                pMetricSet->collect(metricSetReportValues[pMetricSet->getName()]);
            }

            //
            // Send them
            for (auto *pSink : sinks)
            {
                for (auto const &valueIt : metricSetReportValues)
                {
                    pSink->send(valueIt.second, valueIt.first);
                }
            }
            return true;
        }


    protected:

        // MetricsReporter not to be instantiated directly
        MetricsReporter() = default;




    protected:
        std::vector<std::shared_ptr<IMetricSet>> metricSets;
        std::vector<IMetricSink *> sinks;
};

}
