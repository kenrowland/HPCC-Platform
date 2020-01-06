/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2018 HPCC Systems®.

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

#ifndef HPCCSYSTEMS_PLATFORM_OPERATIONDELETENODE_HPP
#define HPCCSYSTEMS_PLATFORM_OPERATIONDELETENODE_HPP

#include "OperationNode.hpp"

class OperationDeleteNode :public OperationNode
{
    public:

        OperationDeleteNode() = default;
        ~OperationDeleteNode() override = default;


    protected:

        void doExecute(std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables, const std::string &nodeId) override;

    friend class EnvModTemplate;
};


#endif //HPCCSYSTEMS_PLATFORM_OPERATIONDELETENODE_HPP
