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

#include <vector>
#include <memory>
#include <map>
#include "IMetricSet.hpp"
#include "IMetricSink.hpp"
#include <platform.h>
#include <dlfcn.h>

namespace hpccMetrics
{


typedef hpccMetrics::IMetricSink* (*getMetricSinkInstance)(const std::map<std::string, std::string> &parms);


class MetricSink : public IMetricSink
{
    public:

        explicit MetricSink(const std::map<std::string, std::string> &parms) {  }
        virtual ~MetricSink() = default;
        void addMetricSet(const std::shared_ptr<IMetricSet> &pSet) override
        {
            metricSets[pSet->getName()] = pSet;
        }

        // not sure if this is the best place for this or not, but wanted a static kind of factory that would load
        // a sink from a .so  (see the metrics/sinks folder for available sink libs)
        static IMetricSink *getMetricSinkFromLib(const char *sinkLibName, const std::map<std::string, std::string> &parms)
        {
            std::string libName = "libhpccmetrics_";
            libName.append(sinkLibName).append(".so");

            HINSTANCE libHandle = dlopen(libName.c_str(), RTLD_NOW|RTLD_GLOBAL);
            if (libHandle != nullptr)
            {
                auto getInstanceProc = (getMetricSinkInstance) GetSharedProcedure(libHandle, "getMetricSinkInstance");
                if (getInstanceProc != nullptr)
                {
                    IMetricSink *pSink = getInstanceProc(parms);
                    return pSink;
                }
            }
            // todo throw an exception here, or return false?
            return nullptr;
        }


    protected:

        std::map<std::string, std::shared_ptr<IMetricSet>> metricSets;
};





}
