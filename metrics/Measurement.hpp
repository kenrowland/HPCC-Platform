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

namespace hpcc_metrics
{
    class MeasurementBase {

        public:

            explicit MeasurementBase(std::string name) : name{std::move(name)} {}
            virtual ~MeasurementBase() = default;

            virtual std::string toString() const = 0;
            virtual const std::string &getName() const { return name; }


        protected:

            std::string name;
    };


    template<typename T>
    class Measurement : public MeasurementBase {
        public:
            Measurement(const std::string &name, T value) :
                    value{value},
                    MeasurementBase(name) {}

            std::string toString() const override  {
                return std::to_string(value);
            }

        protected:

            T value;
    };

}
