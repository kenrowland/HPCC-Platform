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


void OperationModifyNode::doExecute(std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables)
{
    std::shared_ptr<EnvironmentNode> pEnvNode;

    //
    // If there are nodes...
    if (!m_parentNodeIds.empty())
    {
        //
        // The main reason to use a find nodes action is to either save the IDs of the matching nodes and/or attribute
        // values from the matching nodes
        std::shared_ptr<Variable> pSaveNodeIdVar;
        if (!m_saveNodeIdName.empty())
        {
            pSaveNodeIdVar = createVariable(pVariables->doValueSubstitution(m_saveNodeIdName), "string", pVariables,
                    m_accumulateSaveNodeIdOk, m_saveNodeIdAsGlobalValue, m_saveNodeIdClear);
        }

        //
        // Process each node
        for (auto &parentNodeId: m_parentNodeIds)
        {
            Status status;

            std::string nodeId = pVariables->doValueSubstitution(parentNodeId);
            pEnvNode = pEnvMgr->findEnvironmentNodeById(nodeId);
            if (pEnvNode)
            {
                if (pSaveNodeIdVar)
                {
                    pSaveNodeIdVar->addValue(nodeId);
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
    }
    else if (m_throwOnEmpty)
    {
        throw TemplateExecutionException("No nodes selected.");
    }
}
