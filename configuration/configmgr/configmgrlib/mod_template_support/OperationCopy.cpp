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


#include "OperationCopy.hpp"
#include "Operation.hpp"
#include "TemplateExecutionException.hpp"
#include "TemplateException.hpp"
#include <algorithm>


bool OperationCopy::execute(std::shared_ptr<Environments> pEnvironments, std::shared_ptr<Environment> pEnv, std::shared_ptr<Variables> pVariables)
{
    if (m_breakExecution)
    {
        int i = 4;
    }

    if (m_pCondition && !m_pCondition->testCondition(pVariables))
    {
        return false;
    }

    //
    // Init for execution
    initializeForExecution(pEnvironments, pEnv, pVariables);

    //
    // Setup destination environment
    m_pDestEnvMgr = m_pOpEnvMgr;
    if (!m_destTarget.getEnvironmentName().empty())
    {
        m_pDestEnvMgr = pEnvironments->get(pVariables->doValueSubstitution(m_destTarget.getEnvironmentName()))->m_pEnvMgr;
    }


    //
    // Select the target source nodes
    m_target.getTargetNodeIds(m_pOpEnvMgr, pVariables, m_parentNodeIds);

    preExecute(pVariables);

    //
    // Create destination node ID save variable if needed
    if (!m_destSaveNodeIdName.empty())
    {
        m_pDestSaveNodeIdVar = createVariable(pVariables->doValueSubstitution(m_destSaveNodeIdName), "string", pVariables, m_destSaveNodeIdAccumulateOk, m_destSaveNodeIdAsGlobalValue, m_destSaveNodeIdClear);
    }

    //
    // Create variables for saving attribute values (if any)
    for (auto &attr: m_saveAttributes)
    {
        createVariable(attr.first, "string", pVariables, attr.second.accumulateOk, attr.second.global, attr.second.clear);
    }

    //
    // Get the destination nodeIds.
    std::vector<std::string> destNodeIds;
    m_destTarget.getTargetNodeIds(m_pDestEnvMgr, pVariables, destNodeIds);
    if (destNodeIds.empty())
    {
        if (m_throwIfDestEmpty)
        {
            throw TemplateExecutionException("Unable to find any destination node(s)");
        }
        return true;   // not an error if non destination found, so just treat as a noop and leave now
    }

    //
    // If the copy type is many_to_one, then only a single destination node is allowed
    if (m_copyType == "many_to_one" && destNodeIds.size() > 1)
    {
        throw TemplateExecutionException("Copy many_to_one requires a single destination node.");
    }


    size_t idx = 0;
    for (auto const &sourceNodeId: m_parentNodeIds)
    {
        pVariables->setIterationInfo(idx, idx);

        //
        // Save source node ids if needed
        if (m_pSaveNodeIdVar)
        {
            m_pSaveNodeIdVar->addValue(sourceNodeId);
        }

        //
        // If a many_to_one, then the source node is copied as a child beneath the destination node
        if (m_copyType == "many_to_one")
        {
            doNodeCopy(sourceNodeId, destNodeIds[0], false);
            ++idx;
        }
        else  // many_to_many
        {
            if (idx < destNodeIds.size())
            {
                doNodeCopy(sourceNodeId, destNodeIds[pVariables->getCurIndex()], false);
                ++idx;
            }
            else if (m_throwIfDestEmpty)
            {
                throw TemplateExecutionException("Fewer destination nodes than source nodes during copy operation");
            }
            else
            {
                return true;  // only copied enough to fill the found destination nodes
            }
        }
    }

    return true;
}


void OperationCopy::doNodeCopy(const std::string &sourceNodId, const std::string &destParentNodeId, bool isChildNode)
{

    Status status;
    std::shared_ptr<EnvironmentNode> pSourceEnvNode = m_pOpEnvMgr->findEnvironmentNodeById(sourceNodId);
    doNodeCopy(pSourceEnvNode, destParentNodeId, isChildNode);
}


void OperationCopy::doNodeCopy(const std::shared_ptr<EnvironmentNode> &pSourceEnvNode, const std::string &destParentNodeId, bool isChildNode)
{
    Status status;
    std::string newNodeType;
    // todo - Make destNodeType optional. If absent and !isChildNode, don't create a new child node as the des, use destParentNodeId as the destination node

    if (!isChildNode)
    {
        newNodeType = m_destNodeType;
    }
    else
    {
        newNodeType = pSourceEnvNode->getSchemaItem()->getItemType();
    }

    //
    // Create new node or get the dest parent node
    std::shared_ptr<EnvironmentNode> pDestEnvNode;
    if (newNodeType.empty())
    {
        pDestEnvNode = m_pDestEnvMgr->findEnvironmentNodeById(destParentNodeId);
    }
    else
    {
        std::vector<NameValue> emptyAttributes;
        pDestEnvNode = m_pDestEnvMgr->addNewEnvironmentNode(destParentNodeId, newNodeType, emptyAttributes, status, true, true, false);
    }

    //
    // Get source node's attribute values
    std::vector<NameValue> copyAttributeValues;
    getAttributeValues(pSourceEnvNode, copyAttributeValues, isChildNode ? "all" : m_copyAttributeType);

    //
    // Assign attribute values
    pDestEnvNode->setAttributeValues(copyAttributeValues, status, true, true);


    //std::string newNodeType = (!isChildNode && !m_destNodeType.empty()) ? m_destNodeType : pSourceEnvNode->getSchemaItem()->getItemType();
    //std::shared_ptr<EnvironmentNode> pNewEnvNode = m_pDestEnvMgr->getNewEnvironmentNode(destParentNodeId, newNodeType, status);

    //
    // Build the attribute values for initializing the new node (if indicated). An empty vector is required otherwise
    //std::vector<NameValue> copyAttributeValues;
    //getAttributeValues(pSourceEnvNode, copyAttributeValues, isChildNode ? "all" : m_copyAttributeType);

    //auto pNewDestEnvNode = m_pDestEnvMgr->addNewEnvironmentNode(destParentNodeId, newNodeType, copyAttributeValues, status, true, true, false);



    if (!isChildNode && m_pDestSaveNodeIdVar)
    {
        m_pDestSaveNodeIdVar->addValue(pDestEnvNode->getId());
    }

    //
    // If not a child node, save attribute values (if needed)
    if (!isChildNode)
    {
        saveAttributeValues(pSourceEnvNode);
    }

    //
    // Copy children if indicated.
    if (m_includeChildren)
    {
        std::vector<std::shared_ptr<EnvironmentNode>> childEnvNodes;
        pSourceEnvNode->getChildren(childEnvNodes);
        for (auto const &pChildEnvNode: childEnvNodes)
        {
            bool doCopy = true;
            if (m_strictCopyMode)
            {
                std::string sourceItemType = pChildEnvNode->getSchemaItem()->getProperty("itemType");
                std::vector<std::shared_ptr<SchemaItem>> childSchemaNodes;
                pDestEnvNode->getSchemaItem()->getChildren(childSchemaNodes, "", sourceItemType);
                doCopy = !childSchemaNodes.empty();
            }

            if (doCopy)
            {
                doNodeCopy(pChildEnvNode, pDestEnvNode->getId(), true);
            }
        }
    }
}


void OperationCopy::addCopyAttribute(std::string attrStr)
{
    CopyAttributeInfo_t copyAttribute;

    //
    // See if there is a default value
    std::size_t colonPos = attrStr.find_first_of(':');
    if (colonPos != std::string::npos)
    {
        copyAttribute.defaultValue = attrStr.substr(colonPos+1);
        attrStr.erase(colonPos, std::string::npos);
    }

    std::size_t arrowPos = attrStr.find_first_of("->");
    if (arrowPos != std::string::npos)
    {
        copyAttribute.inputAttributeName = attrStr.substr(0, arrowPos);
        copyAttribute.outputAttributeName = attrStr.substr(arrowPos + 2);
    }
    else
    {
        copyAttribute.inputAttributeName = copyAttribute.outputAttributeName = attrStr;
    }

    m_copyAttributes.emplace_back(copyAttribute);
}


void OperationCopy::addSaveAttributeValue(const std::string &attrName, const std::string &varName, bool global, bool accuulateOk, bool clear)
{
    SaveAttributeInfo_t saveInfo;
    saveInfo.attributeName = attrName;
    saveInfo.varName = varName;
    saveInfo.global = global;
    saveInfo.accumulateOk = accuulateOk;
    saveInfo.clear = clear;
    auto insertRc = m_saveAttributes.insert({attrName, saveInfo});
    if (!insertRc.second)
    {
        std::string reason = "Duplicate save attribute name detected, attribute name=" + attrName;
        throw TemplateException(reason);
    }
}


void OperationCopy::saveAttributeValues(const std::shared_ptr<EnvironmentNode> &pEnvNode)
{
    for (auto const &it: m_saveAttributes)
    {
        std::string attrValue = pEnvNode->getAttributeValue(it.second.attributeName);
        it.second.pVar->addValue(attrValue);
    }
}


void OperationCopy::getAttributeValues(const std::shared_ptr<EnvironmentNode> &pSourceNode, std::vector<NameValue> &initAttributes,
        const std::string &copyMpde)
{
    std::vector<std::shared_ptr<EnvironmentValue>> attrEnvValues;
    if (copyMpde == "all")
    {
        pSourceNode->getAttributes(attrEnvValues);
        for (auto const &attrEnvValue: attrEnvValues)
        {
            initAttributes.emplace_back(NameValue(attrEnvValue->getName(), attrEnvValue->getValue()));
        }
    }
    else if (copyMpde == "exclude")
    {
        pSourceNode->getAttributes(attrEnvValues);
        for (auto const &attrEnvValue: attrEnvValues)
        {
            auto it = std::find(m_copyAttributes.begin(), m_copyAttributes.end(), attrEnvValue->getName());
            if (it == m_copyAttributes.end())
            {
                initAttributes.emplace_back(NameValue(attrEnvValue->getName(), attrEnvValue->getValue()));
            }
        }
    }
    else if (copyMpde == "list")
    {
        for (auto const &copyAttrInfo: m_copyAttributes)
        {
            auto pEnvValue = pSourceNode->getAttribute(copyAttrInfo.inputAttributeName);
            if (pEnvValue)
            {
                initAttributes.emplace_back(NameValue(copyAttrInfo.outputAttributeName, pEnvValue->getValue()));
            }
            else if (!copyAttrInfo.defaultValue.empty())
            {
                initAttributes.emplace_back(NameValue(copyAttrInfo.outputAttributeName, copyAttrInfo.defaultValue));
            }
        }
    }
}
