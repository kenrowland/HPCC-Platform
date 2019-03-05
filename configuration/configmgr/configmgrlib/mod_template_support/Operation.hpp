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

#ifndef HPCCSYSTEMS_PLATFORM_OPERATION_HPP
#define HPCCSYSTEMS_PLATFORM_OPERATION_HPP

#include "Variable.hpp"
#include "Variables.hpp"

class EnvironmentMgr;

class Operation
{
    public:

        Operation() = default;
        virtual ~Operation() = default;
        virtual bool execute(EnvironmentMgr *pEnvMgr, std::shared_ptr<Variables> pVariables) = 0;

};


#endif //HPCCSYSTEMS_PLATFORM_OPERATION_HPP
