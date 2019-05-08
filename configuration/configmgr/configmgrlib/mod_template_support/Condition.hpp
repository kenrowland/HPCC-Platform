/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2019 HPCC Systems®.

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

#ifndef HPCCSYSTEMS_PLATFORM_CONDITION_HPP
#define HPCCSYSTEMS_PLATFORM_CONDITION_HPP

#include <vector>
#include "Variables.hpp"

class Condition {

    public:

        Condition() = default;
        ~Condition() = default;
        bool testCondition(const std::shared_ptr<Variables> &pVariables) const;


    protected:

        std::string m_varName;
        std::string m_type;
        std::vector<std::string> m_values;

    friend class EnvModTemplate;
};


#endif //HPCCSYSTEMS_PLATFORM_CONDITION_HPP
