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


#ifndef HPCCSYSTEMS_PLATFORM_OPERATIONINCLUDETEMPLATE_HPP
#define HPCCSYSTEMS_PLATFORM_OPERATIONINCLUDETEMPLATE_HPP

#include "Operation.hpp"
#include "ParameterValue.hpp"

class EnvModTemplate;

class OperationIncludeTemplate : public Operation
{
    public:

        OperationIncludeTemplate() = default;
        ~OperationIncludeTemplate() override = default;
        bool execute(EnvironmentMgr &envMgr, std::shared_ptr<Variables> pVariables) override;
        void addParameterValue(const ParameterValue &parmValue) { m_parameters.emplace_back(parmValue); }


    protected:

        std::shared_ptr<EnvModTemplate> m_pEnvModTemplate;
        std::string m_path;
        std::vector<ParameterValue> m_parameters;

        friend class EnvModTemplate;
};

#endif //HPCCSYSTEMS_PLATFORM_OPERATIONINCLUDETEMPLATE_HPP
