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


#include "OperationFindNode.hpp"
#include "TemplateExecutionException.hpp"


void OperationFindNode::doExecute(std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables, const std::string &nodeId)
{

    if (m_saveAttributeValues || m_pSaveNodeIdVar)
    {
        std::string EnvNodeId = pVariables->doValueSubstitution(nodeId);
        auto pEnvNode = pEnvMgr->findEnvironmentNodeById(EnvNodeId);
        if (pEnvNode)
        {
            if (m_pSaveNodeIdVar)
            {
                m_pSaveNodeIdVar->addValue(nodeId);
            }

            //
            // Save attributes
            if (m_saveAttributeValues)
            {
                saveAttributeValues(pVariables, pEnvNode);
            }

            //
            // Process node value
            processNodeValue(pVariables, pEnvNode);
        }
    }
}
