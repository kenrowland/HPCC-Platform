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


bool OperationCopy::execute(std::shared_ptr<Environments> pEnvironments, std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables)
{
    //
    // Init for execution
    initializeForExecution(pEnvironments, pEnvMgr, pVariables);

    //
    // Setup destination environment
    m_pDestEnv = pEnvMgr;
    if (!m_destEnvironmentName.empty())
    {
        m_pDestEnv = pEnvironments->get(pVariables->doValueSubstitution(m_destEnvironmentName))->m_pEnvMgr;
    }

    //
    // If indicated, create a variable to save the copy from node IDs
    std::shared_ptr<Variable> pSaveNodeIdInput;
    if (!m_saveNodeIdName.empty())
    {
        pSaveNodeIdInput = createVariable(m_saveNodeIdName, "string", pVariables, m_accumulateSaveNodeIdOk, m_saveNodeIdAsGlobalValue);
    }

    //
    // Get parent node Ids for copy source
    // question as to whether >1 nodes to copy should be supported (I think yes right now)
    m_parentNodeIds = getNodeIds(m_pSourceEnv, pVariables, m_parentNodeId, m_path);
    if (m_parentNodeIds.empty() && m_throwOnEmpty)
    {
        throw TemplateExecutionException("No nodes selected for copy");
    }

    //
    // Get the destination nodeId. Only one is allowed
    auto destNodeIds = getNodeIds(m_pDestEnv, pVariables, m_destParentNodeId, m_destPath);
    if (destNodeIds.empty())
    {
        throw TemplateExecutionException("Unable to find the destination parent node");
    }
    else if (destNodeIds.size() > 1)
    {
        throw TemplateExecutionException("Only a single destination parent node may be selected");
    }
    auto destNodeId = destNodeIds[0];

    //
    // Create variables for saving attribute values
    createAttributeSaveVariables(pVariables);

    //
    // Copy the nodes
    for (auto const &sourceNodeId: m_parentNodeIds)
    {
        if (pSaveNodeIdInput)
        {
            pSaveNodeIdInput->addValue(sourceNodeId);
        }
        doNodeCopy(sourceNodeId, destNodeId);
    }

    return true;
}


void OperationCopy::doNodeCopy(const std::string &sourceNodId, const std::string &destParentNodeId)
{
    Status status;
    std::shared_ptr<EnvironmentNode> pSourceEnvNode = m_pSourceEnv->findEnvironmentNodeById(sourceNodId);
    std::string newNodeType = (!m_destNodeType.empty()) ? m_destNodeType : pSourceEnvNode->getSchemaItem()->getItemType();
    std::shared_ptr<EnvironmentNode> pNewEnvNode = m_pDestEnv->getNewEnvironmentNode(destParentNodeId, newNodeType, status);

    std::vector<NameValue> copyAttributeValues;
    getAttributeValues(pSourceEnvNode, copyAttributeValues);

    m_pDestEnv->addNewEnvironmentNode(destParentNodeId, newNodeType, copyAttributeValues, status, true, true, false);
}


// a copy children



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


void OperationCopy::addSaveAttributeValue(const std::string &attrName, const std::string &varName, bool global, bool accuulateOk)
{
    SaveAttributeInfo_t saveInfo;
    saveInfo.attributeName = attrName;
    saveInfo.varName = varName;
    saveInfo.global = global;
    saveInfo.accumulateOk = accuulateOk;
    auto insertRc = m_saveAttributes.insert({attrName, saveInfo});
    if (!insertRc.second)
    {
        std::string reason = "Duplicate save attribute name detected, attribute name=" + attrName;
        throw TemplateException(reason);
    }
}


void OperationCopy::createAttributeSaveVariables(const std::shared_ptr<Variables> &pVariables)
{
    for (auto const &it: m_saveAttributes)
    {
        createVariable(it.second.varName, "string", pVariables, it.second.accumulateOk, it.second.global);
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


void OperationCopy::getAttributeValues(const std::shared_ptr<EnvironmentNode> &pSourceNode, std::vector<NameValue> &initAttributes)
{
    std::vector<std::shared_ptr<EnvironmentValue>> attrEnvValues;
    if (m_copyAttributeType == "all")
    {
        pSourceNode->getAttributes(attrEnvValues);
        for (auto const &attrEnvValue: attrEnvValues)
        {
            initAttributes.emplace_back(NameValue(attrEnvValue->getName(), attrEnvValue->getValue()));
        }
    }
    else if (m_copyAttributeType == "except")
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
    else if (m_copyAttributeType == "list")
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
