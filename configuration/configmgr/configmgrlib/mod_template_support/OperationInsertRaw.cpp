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
##############################################################################*/


#include "OperationInsertRaw.hpp"
#include "TemplateExecutionException.hpp"

void OperationInsertRaw::doExecute(std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables, const std::string &nodeId)
{
    Status status;

    std::string parentNodeId = pVariables->doValueSubstitution(nodeId);
    std::shared_ptr<EnvironmentNode> pParentEnvNode = pEnvMgr->findEnvironmentNodeById(parentNodeId);
    if (pParentEnvNode)
    {
        pEnvMgr->insertRawEnvironmentData(pParentEnvNode, m_nodeType, m_rawData);
    }
    else
    {
        throw TemplateExecutionException("Unable to find parent node");
    }
}
