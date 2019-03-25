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
#include "Environments.hpp"

class EnvironmentMgr;

class Operation
{
    public:

        Operation() : m_count("1"), m_startIndex("0"), m_executionCount(0), m_executionStartIndex(0) {}
        virtual ~Operation() = default;
        virtual bool execute(std::shared_ptr<Environments> pEnvironments, const std::string &environmentName, std::shared_ptr<Variables> pVariables) = 0;


    protected:

        void initializeForExecution(std::shared_ptr<Variables> pVariables);


    protected:

        std::string m_count;
        std::string m_startIndex;

        size_t m_executionCount;
        size_t m_executionStartIndex;
};


#endif //HPCCSYSTEMS_PLATFORM_OPERATION_HPP
