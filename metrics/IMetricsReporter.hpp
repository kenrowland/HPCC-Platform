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

#include "MetricSink.hpp"

namespace hpccMetrics
{
    //
    // Interface for reporting metrics
    class IMetricsReporter
    {
        public:

            //
            // Add a sink to the set of sinks to which the defined metric set's values are reported.
            virtual void addSink(IMetricSink *pSink) = 0;

            //
            // Add a metric set for collection and reporting.
            virtual void addMetricSet(const std::shared_ptr<IMetricSet>& pSet) = 0;

            //
            // Initialize the defined sinks and metric sets for collection and reporting
            virtual void init() = 0;


            //
            // Collect and report values for the defined metric sets to the defined metric sinks.
            virtual bool report() = 0;
    };


}
