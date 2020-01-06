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


#include "OperationCreateNode.hpp"
#include "EnvironmentMgr.hpp"
#include "EnvironmentNode.hpp"
#include "EnvironmentValue.hpp"
#include "TemplateExecutionException.hpp"
#include "Status.hpp"


void OperationCreateNode::doExecute(std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables, const std::string &nodeId)
{
    std::string parentNodeId = pVariables->doValueSubstitution(nodeId);

    Status status;
    std::shared_ptr<EnvironmentNode> pNewEnvNode = pEnvMgr->getNewEnvironmentNode(parentNodeId, m_nodeType, status);
    if (pNewEnvNode)
    {
        //
        // Build a vector of name value pairs of the attributes to set for the node. Also, if any attribute
        // indicates that its set value shall be saved, ensure an input exists.
        std::vector<NameValue> attrValues;
        for (auto &attr: m_attributes)
        {
            if (!attr.doNotSet && (!attr.cookedValue.empty() || !attr.optional))
            {
                attrValues.emplace_back(NameValue(attr.getName(), attr.cookedValue));
            }
        }

        //
        // Add the new node to the environment
        pNewEnvNode = pEnvMgr->addNewEnvironmentNode(parentNodeId, pVariables->doValueSubstitution(m_nodeType), attrValues, status, true, true, m_populateChildren);
        if (pNewEnvNode)
        {
            // construct a status for just this new node's ID so we can see if there is an error
            Status newNodeStatus(status, pNewEnvNode->getId());
            if (newNodeStatus.isError())
            {
                throw TemplateExecutionException("There was a problem adding the new node, status returned an error");
            }

            //
            // save the node id if indicated
            if (m_pSaveNodeIdVar)
            {
                m_pSaveNodeIdVar->addValue(pNewEnvNode->getId());
            }

            //
            // Save any attribute values to inputs for use later
            saveAttributeValues(pVariables, pNewEnvNode);

            //
            // Process node value
            processNodeValue(pVariables, pNewEnvNode);
        }
        else
        {
            throw TemplateExecutionException("There was a problem adding the new node");
        }
    }
    else
    {
        throw TemplateExecutionException("Unable to get new node");
    }
}
