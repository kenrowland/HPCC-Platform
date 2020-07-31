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
#include "IMeasurement.hpp"

namespace hpccMetrics
{

    //
    // Interface for a metric sink which represents a store for metric values
    class IMetricSink
    {
        public:

            //
            // Initialize the sink
            virtual void init() = 0;

            //
            // Send a set of measurements for the named metric set to the sind's store
            virtual void send(const std::vector<std::shared_ptr<IMeasurement>> &values, const std::string &setName) = 0;

            //
            // Add a metric set to the set that is being reported by this sink. This should only be done
            // during configuration and is informational purposes in case the sink needs to know
            // information about the set and its metrics in order to properly report values.
            virtual void addMetricSet(const std::shared_ptr<IMetricSet> &pSet) = 0;
    };

}
