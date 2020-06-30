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

namespace hpcc_metrics
{

    class MeasurementBase;
    class MetricSet;

    class MetricSink
    {
        public:

            explicit MetricSink(const std::map<std::string, std::string> &parms) {  }
            virtual ~MetricSink() = default;
            virtual void init(const std::vector<std::shared_ptr<MetricSet>> &metricSets) = 0;
            virtual void send(const std::vector<std::shared_ptr<MeasurementBase>> &values, const std::string &setName) = 0;
    };
}
