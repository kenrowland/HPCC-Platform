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

#include "OperationModifyNode.hpp"

#include "OperationNode.hpp"
#include "Status.hpp"
#include "EnvironmentMgr.hpp"
#include "TemplateExecutionException.hpp"


void OperationModifyNode::doExecute(std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables, const std::string &nodeId)
{
    std::shared_ptr<EnvironmentNode> pEnvNode;

    Status status;

    std::string parentNodeId = pVariables->doValueSubstitution(nodeId);
    pEnvNode = pEnvMgr->findEnvironmentNodeById(nodeId);
    if (pEnvNode)
    {
        if (m_pSaveNodeIdVar)
        {
            m_pSaveNodeIdVar->addValue(nodeId);
        }

        //
        // Set the indicated attribute values
        std::vector<NameValue> attrValues;
        for (auto &attr: m_attributes)
        {
            if (!attr.doNotSet)
            {
                attrValues.emplace_back(NameValue(attr.getName(), attr.cookedValue));
            }
        }
        pEnvNode->setAttributeValues(attrValues, status, true, true);

        //
        // Process node value
        processNodeValue(pVariables, pEnvNode);
    }
    else
    {
        throw TemplateExecutionException("There was an error retrieving a node for modification");
    }
}
