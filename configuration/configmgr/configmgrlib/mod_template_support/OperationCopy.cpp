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
    //
    // Init for execution
    initializeForExecution(pEnvironments, pEnv, pVariables);

    //
    // Setup destination environment
    m_pDestEnvMgr = m_pOpEnvMgr;
    if (!m_destEnvironmentName.empty())
    {
        m_pDestEnvMgr = pEnvironments->get(pVariables->doValueSubstitution(m_destEnvironmentName))->m_pEnvMgr;
    }

    //
    // If indicated, create a variable to save the copy from node IDs
    std::shared_ptr<Variable> pSaveNodeIdInput;
    if (!m_saveNodeIdName.empty())
    {
        pSaveNodeIdInput = createVariable(pVariables->doValueSubstitution(m_saveNodeIdName), "string", pVariables, m_accumulateSaveNodeIdOk, m_saveNodeIdAsGlobalValue, m_saveNodeIdClear);
    }

    //
    // Create variables for saving attribute values (if any)
    for (auto &attr: m_saveAttributes)
    {
        createVariable(attr.first, "string", pVariables, attr.second.accumulateOk, attr.second.global, attr.second.clear);
        // todo make this name/save info a separate class and encapsulate in the OperationNode modAttribute class
    }

    //
    // Now execute the operation count times
    for (size_t idx=0; idx < m_executionCount; ++idx)
    {
        pVariables->setIterationInfo(m_executionStartIndex + idx, idx);

        //
        // Get parent node Id for copy source based on execution index. Only one is supported as a source
        m_parentNodeIds = getNodeIds(m_pOpEnvMgr, pVariables, m_parentNodeId, m_path);
        if (m_parentNodeIds.empty() && m_throwOnEmpty)
        {
            throw TemplateExecutionException("No nodes selected for copy");
        }
        else if (m_parentNodeIds.size() > 1)
        {
            throw TemplateExecutionException("Only a single source node may be selected for a copy");
        }

        //
        // Get the destination nodeIds. todo add check to allow for an empty set, ie don't fail the copy if no destination selected (support error_if_not_found flag)
        auto destNodeIds = getNodeIds(m_pDestEnvMgr, pVariables, m_destParentNodeId, m_destPath);
        if (destNodeIds.empty())
        {
            throw TemplateExecutionException("Unable to find the destination parent node");
        }

        //
        // Copy the nodes
        for (auto const &sourceNodeId: m_parentNodeIds)
        {
            //
            // If indicated, save the node ID
            if (pSaveNodeIdInput)
            {
                pSaveNodeIdInput->addValue(sourceNodeId);
            }
            doNodeCopy(sourceNodeId, destNodeIds[pVariables->getCurIndex()], false);
        }
    }

//    //
//    // Get parent node Id for copy source. Only one is supported as a source
//    m_parentNodeIds = getNodeIds(m_pOpEnvMgr, pVariables, m_parentNodeId, m_path);
//    if (m_parentNodeIds.empty() && m_throwOnEmpty)
//    {
//        throw TemplateExecutionException("No nodes selected for copy");
//    }
//    else if (m_parentNodeIds.size() > 1)
//    {
//        throw TemplateExecutionException("Only a single source node may be selected for a copy");
//    }
//
//    //
//    // Get the destination nodeIds. todo add check to allow for an empty set, ie don't fail the copy if no destination selected (support error_if_not_found flag)
//    auto destNodeIds = getNodeIds(m_pDestEnvMgr, pVariables, m_destParentNodeId, m_destPath);
//    if (destNodeIds.empty())
//    {
//        throw TemplateExecutionException("Unable to find the destination parent node");
//    }
////    else if (destNodeIds.size() > 1)
////    {
////        throw TemplateExecutionException("Only a single destination parent node may be selected");
////    }
////    auto destNodeId = destNodeIds[0];
//
//    //
//    // Create variables for saving attribute values (if any)
//    for (auto &attr: m_saveAttributes)
//    {
//        createVariable(attr.first, "string", pVariables, attr.second.accumulateOk, attr.second.global, attr.second.clear);
//        // todo make this name/save info a separate class and encapsulate in the OperationNode modAttribute class
//    }
//
//    //
//    // Copy the nodes
//    for (auto const &sourceNodeId: m_parentNodeIds)
//    {
//        //
//        // If indicated, save the node ID
//        if (pSaveNodeIdInput)
//        {
//            pSaveNodeIdInput->addValue(sourceNodeId);
//        }
//        doNodeCopy(sourceNodeId, destNodeIds[pVariables->getCurIndex()], false);
//    }

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
    std::string newNodeType = (!isChildNode && !m_destNodeType.empty()) ? m_destNodeType : pSourceEnvNode->getSchemaItem()->getItemType();
    std::shared_ptr<EnvironmentNode> pNewEnvNode = m_pDestEnvMgr->getNewEnvironmentNode(destParentNodeId, newNodeType, status);

    //
    // Build the attribute values for initializing the new node (if indicated). An empty vector is required otherwise
    std::vector<NameValue> copyAttributeValues;
    getAttributeValues(pSourceEnvNode, copyAttributeValues, isChildNode ? "all" : m_copyAttributeType);

    auto pNewDestEnvNode = m_pDestEnvMgr->addNewEnvironmentNode(destParentNodeId, newNodeType, copyAttributeValues, status, true, true, false);

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
            doNodeCopy(pChildEnvNode, pNewDestEnvNode->getId(), true);
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
    else if (copyMpde == "except")
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
