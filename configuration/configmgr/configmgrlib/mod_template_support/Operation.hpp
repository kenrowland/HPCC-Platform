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

#include <string>
#include <vector>
#include "Variable.hpp"
#include "Variables.hpp"
#include "Environments.hpp"
#include "Condition.hpp"

class EnvironmentMgr;

class Operation
{
    public:

        Operation() : m_count("1"), m_startIndex("0"), m_executionCount(0), m_executionStartIndex(0), m_breakExecution(false) {}
        virtual ~Operation() = default;
        virtual bool execute(std::shared_ptr<Environments> pEnvironments, std::shared_ptr<Environment> pEnv, std::shared_ptr<Variables> pVariables) = 0;


    protected:

        void initializeForExecution(const std::shared_ptr<Variables> &pVariables);


//        std::vector<std::string> getNodeIds(const std::shared_ptr<EnvironmentMgr> &pEnvMgr, const std::shared_ptr<Variables> &pVariables,
//                                            const std::string &nodeId, const std::string &path);

    protected:

        std::string m_count;
        std::string m_startIndex;
        size_t m_executionCount;
        size_t m_executionStartIndex;
        std::shared_ptr<Condition> m_pCondition;
        bool m_breakExecution;

    friend class EnvModTemplate;
};


#endif //HPCCSYSTEMS_PLATFORM_OPERATION_HPP
