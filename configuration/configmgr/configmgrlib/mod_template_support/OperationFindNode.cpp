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
#include "EnvironmentMgr.hpp"
#include "EnvironmentNode.hpp"
#include "TemplateExecutionException.hpp"
#include "Status.hpp"
#include "OperationNode.hpp"
#include "Operation.hpp"


void OperationFindNode::doExecute(std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables)
{
    //
    // The main reason to use a find nodes action is to either save the IDs of the matching nodes and/or attribute
    // values from the matching nodes. Create the variable regardless as there could be a test for empty.
    std::shared_ptr<Variable> pSaveNodeIdVar;
    if (!m_saveNodeIdName.empty())
    {
        pSaveNodeIdVar = createVariable(pVariables->doValueSubstitution(m_saveNodeIdName), "string", pVariables, m_accumulateSaveNodeIdOk, m_saveNodeIdAsGlobalValue, m_saveNodeIdClear);
    }

    //
    // Any parent node IDs found?
    if (!m_parentNodeIds.empty())
    {
        //
        // Saving the ID or any attribute values?
        if (createAttributeSaveInputs(pVariables) || pSaveNodeIdVar)
        {
            for (auto &parentNodeId: m_parentNodeIds)
            {
                std::string nodeId = pVariables->doValueSubstitution(parentNodeId);
                auto pEnvNode = pEnvMgr->findEnvironmentNodeById(nodeId);
                if (pEnvNode)
                {
                    if (pSaveNodeIdVar)
                    {
                        pSaveNodeIdVar->addValue(nodeId);
                    }

                    //
                    // Attributes should only be present if values were to be saved
                    if (!m_attributes.empty())
                    {
                        saveAttributeValues(pVariables, pEnvNode);
                    }

                    //
                    // Process node value
                    processNodeValue(pVariables, pEnvNode);
                }
            }
        }
    }

    //
    // Otherwise, no nodes found, go create? Can only create if the target type was a path
    else if (m_createIfNotFound && !m_path.empty())
    {
        //
        // To create the node, use the node type. If no node type was specified, use the last part of the path.
        if (m_nodeType.empty())
        {
            std::size_t lastSlashPos = m_path.find_last_of('/');
            if (lastSlashPos != std::string::npos)
            {
                m_nodeType = m_path.substr(lastSlashPos + 1);
                m_path = m_path.substr(0, lastSlashPos);
                m_parentNodeIds = getNodeIds(pEnvMgr, pVariables, m_parentNodeId, m_path);  // reset these since the parent path has changed
            }
            else
            {
                throw TemplateExecutionException("Invalid path for find operation, unable to create node");
            }
        }
        OperationCreateNode::doExecute(pEnvMgr, pVariables);
    }
    else if (m_throwOnEmpty)
    {
        throw TemplateExecutionException("Unable to find parent node");
    }
}
