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
#include "MetricSet.hpp"
#include "MetricSink.hpp"

typedef hpcc_metrics::MetricSink* (*getMetricSinkInstance)(const std::map<std::string, std::string> &parms);

namespace hpcc_metrics
{

    struct SinkInfo {
        explicit SinkInfo(MetricSink *_pSink, HINSTANCE handle = nullptr) : pSink{_pSink}, libHandle{handle} { }
        ~SinkInfo()
        {

            if (libHandle != nullptr)
                dlclose(libHandle);

            if (pSink != nullptr)
                free(pSink);
        }
        HINSTANCE libHandle;
        MetricSink *pSink;
    };

    class MetricsReporter
    {
        public:

            virtual ~MetricsReporter() = default;


            void addSink(MetricSink *pSink)
            {
                std::shared_ptr<SinkInfo> pSi = std::make_shared<SinkInfo>(pSink, nullptr);
                sinks.emplace_back(pSi);
            }


            void addSink(const std::string &sinkName, const std::map<std::string, std::string> &parms)
            {
                std::string libName = "libhpccmetrics_" + sinkName + ".so";

                HINSTANCE libHandle = dlopen(libName.c_str(), RTLD_NOW|RTLD_GLOBAL);
                if (libHandle != nullptr)
                {
                    auto getInstanceProc = (getMetricSinkInstance) GetSharedProcedure(libHandle, "getMetricSinkInstance");
                    if (getInstanceProc != nullptr)
                    {
                        MetricSink *pSink = getInstanceProc(parms);
                        std::shared_ptr<SinkInfo> pSi = std::make_shared<SinkInfo>(pSink, libHandle);
                        sinks.emplace_back(pSi);
                    }
                }
                // todo throw an exception here, or return false?
            }


            void addMetricSet(const std::shared_ptr<MetricSet>& pSet)
            {
                metricSets.emplace_back(pSet);
            }


            virtual bool report()
            {
                std::map<std::string, std::vector<std::shared_ptr<MeasurementBase>>> values;
                //
                // Collect all the values
                //std::vector<std::shared_ptr<MeasurementBase>> values;
                for (const auto &pMetricSet : metricSets)
                {
                    values[pMetricSet->getName()] = std::vector<std::shared_ptr<MeasurementBase>>();
                    pMetricSet->collect(values[pMetricSet->getName()]);
                }

                //
                // Send them
                for (auto &pSinkInfo : sinks)
                {
                    for (auto const &valueIt : values)
                    {
                        pSinkInfo->pSink->send(valueIt.second, valueIt.first);
                    }
                }
                return true;
            }


        protected:

            MetricsReporter() = default;
            virtual void init()
            {
                //
                // Initialization consists of initializing each sink, informing
                // each sink about the metric sets for which it shall report
                // measurements, and initializing each metric set.
                for (auto &pSinkInfo : sinks)
                {
                    pSinkInfo->pSink->init(metricSets);
                }

                //
                // Tell each metric that collection is beginning
                for (const auto& pMetricSet : metricSets)
                {
                    pMetricSet->init();
                }
            }


        protected:
            std::vector<std::shared_ptr<MetricSet>> metricSets;
            std::vector<std::shared_ptr<SinkInfo>> sinks;
    };
}
