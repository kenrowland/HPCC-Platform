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
#include <vector>
#include <memory>
#include "IMeasurement.hpp"


namespace hpccMetrics
{

    //
    // Interface for all metrics
    class IMetric
    {
        public:

            //
            // Return the name of the metric. Note this name may be further decorated by a higher level component.
            virtual const std::string &getName() const = 0;

            //
            // Optional method to set the type of the reported value.
            virtual void setType(const std::string &_type) = 0;

            //
            // Returns the type (empty string if no type set).
            virtual std::string getType() const = 0;

            //
            // Indicates if the metric is typed or not.
            virtual bool isTyped() const = 0;

            //
            // Indicates if the metric has been added to a metric set or not.
            virtual bool isInMetricSet() const = 0;

            //
            // Set whether the metric has been added to a metric set or not
            virtual void setInMetricSet(bool val) = 0;

            //
            // Initialize the metric for collection and reporting.
            virtual void init() = 0;

            //
            // Collect the value(s) of the metric. The prefix is used to decorate the name of the reported metric
            // value.
            virtual bool collect(std::vector<std::shared_ptr<IMeasurement>> &values, const std::string &prefix) = 0;
    };

}
