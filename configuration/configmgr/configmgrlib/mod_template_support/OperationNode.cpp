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

#include "OperationNode.hpp"
#include "TemplateExecutionException.hpp"
#include "EnvironmentNode.hpp"
#include "EnvironmentValue.hpp"


void OperationNode::addAttribute(modAttribute &newAttribute)
{
    m_attributes.emplace_back(newAttribute);
}


bool OperationNode::execute(EnvironmentMgr *pEnvMgr, std::shared_ptr<Variables> pVariables)
{
    bool rc = true;
    size_t count, startIndex;

    pVariables->setInputIndex(0);

    //
    // Determine the count of iterations to do
    std::string countStr = pVariables->doValueSubstitution(m_count);
    try
    {
        count = std::stoul(countStr);
    }
    catch (...)
    {
        throw TemplateExecutionException("Non-numeric count found for count");
    }

    //
    // Determine the starting index
    std::string startIdxStr = pVariables->doValueSubstitution(m_startIndex);
    try
    {
        startIndex = std::stoul(startIdxStr);
    }
    catch (...)
    {
        throw TemplateExecutionException("Non-numeric count found for start index");
    }

    //
    // Select the nodes for execution
    getParentNodeIds(pEnvMgr, pVariables);

    //
    // If count is > 1, then the number of selected nodes must be 1
    if (count > 1 && m_parentNodeIds.size() != 1)
    {
        throw TemplateExecutionException("Operation execution count > 1 attempted when selected node count is not 1");
    }

    //
    // Now execute the operation count times
    for (size_t idx=0; idx < count; ++idx)
    {
        pVariables->setInputIndex(startIndex + idx);
        assignAttributeCookedValues(pVariables);
        doExecute(pEnvMgr, pVariables);
    }
    return rc;
}


void OperationNode::assignAttributeCookedValues(std::shared_ptr<Variables> pVariables)
{
    //
    // go through the ones defined by the operation and set each (by name)
    for (auto &attr: m_attributes)
    {
        attr.cookedValue = pVariables->doValueSubstitution(attr.value);
    }
}


void OperationNode::getParentNodeIds(EnvironmentMgr *pEnvMgr, std::shared_ptr<Variables> pVariables)
{
    //
    // Find the parent node(s). Either was input, or based on a path, which may match more than one node
    if (!m_parentNodeId.empty())
    {
        std::shared_ptr<Variable> pInput = pVariables->getVariable(m_parentNodeId);
        if (pInput)
        {
            std::size_t numIds = pInput->getNumValues();
            for (std::size_t idx = 0; idx < numIds; ++idx)
            {
                m_parentNodeIds.emplace_back(pVariables->doValueSubstitution(pInput->getValue(idx)));
            }
        }
        else
        {
            m_parentNodeIds.emplace_back(pVariables->doValueSubstitution(m_parentNodeId));
        }
    }
    else
    {
        std::vector<std::shared_ptr<EnvironmentNode>> envNodes;
        std::string path = pVariables->doValueSubstitution(m_path);
        pEnvMgr->fetchNodes(path, envNodes);
        for (auto &envNode: envNodes)
            m_parentNodeIds.emplace_back(envNode->getId());
    }
}


std::shared_ptr<Variable> OperationNode::createVariable(std::string varName, const std::string &varType,
                                                    std::shared_ptr<Variables> pVariables, bool existingOk, bool global)
{
    std::shared_ptr<Variable> pVariable;

    //
    // If creating a local variable...
    if (!global)
    {
        pVariable = pVariables->getVariable(varName, !global, false);
        if (!pVariable)
        {
            pVariable = variableFactory(varType, varName);
            pVariables->add(pVariable, false);
        }
        else if (!existingOk)
        {
            std::string msg = "Attempt to create local variable that already exists: " + varName;
            throw TemplateExecutionException(msg);
        }
    }

    //
    // Otherwise creating a global variable
    else
    {
        pVariable = pVariables->getGlobalVariable(varName, false);
        if (!pVariable)
        {
            pVariable = variableFactory(varType, varName);
            pVariables->add(pVariable, true);
        }
        else if (!existingOk)
        {
            std::string msg = "Attempt to create global variable that already exists: " + varName;
            throw TemplateExecutionException(msg);
        }
    }

    return pVariable;
}


bool OperationNode::createAttributeSaveInputs(std::shared_ptr<Variables> pVariables)
{
    bool rc = false;
    for (auto &attr: m_attributes)
    {
        //
        // If this is a saved attribute value, make sure the input exists
        if (!attr.saveVariableName.empty())
        {
            createVariable(attr.saveVariableName, "string", pVariables, attr.accumulateValuesOk, attr.saveValueGlobal);
            rc = true;
        }
    }
    return rc;
}


void OperationNode::saveAttributeValues(std::shared_ptr<Variables> pVariables, const std::shared_ptr<EnvironmentNode> &pEnvNode)
{
    for (auto &attr: m_attributes)
    {
        if (!attr.saveVariableName.empty())
        {
            bool found=false, set=false;
            std::shared_ptr<Variable> pInput = pVariables->getVariable(attr.saveVariableName);
            std::size_t numNames = attr.getNumNames();
            for (std::size_t idx=0; idx<numNames; ++idx)
            {
                auto pAttr = pEnvNode->getAttribute(attr.getName(idx));
                if (pAttr)
                {
                    found = true;
                    std::string attrValue = pAttr->getValue();
                    if (!attrValue.empty())
                    {
                        pInput->addValue(pAttr->getValue());
                        set = true;
                        break;
                    }
                }
            }

            if (attr.errorIfNotFound && !found)
            {
                throw TemplateExecutionException("Unable to find attribute starting with name=" + attr.getName());
            }

            if (attr.errorIfEmpty && !set)
            {
                throw TemplateExecutionException("No value found for attribute starting with name=" + attr.getName());
            }
        }
    }
}


void OperationNode::processNodeValue(std::shared_ptr<Variables> pVariables, const std::shared_ptr<EnvironmentNode> &pEnvNode)
{
    if (m_nodeValueValid)
    {
        //
        // Need to assign a new value?
        if (!m_nodeValue.value.empty())
        {
            Status status;
            m_nodeValue.cookedValue = pVariables->doValueSubstitution(m_nodeValue.value);
            pEnvNode->setLocalValue(m_nodeValue.cookedValue, status, true);
        }

        //
        // Need to save the value?
        if (!m_nodeValue.saveVariableName.empty())
        {
            std::shared_ptr<Variable> pVar = createVariable(m_nodeValue.saveVariableName, "string", pVariables, m_nodeValue.accumulateValuesOk, m_nodeValue.saveValueGlobal);
            if (pEnvNode->isLocalValueSet())
            {
                pVar->addValue(pEnvNode->getLocalValue());
            }
            else if (m_nodeValue.errorIfEmpty)
            {
                throw TemplateExecutionException("No value found for attribute starting with name=" + m_nodeValue.getName());
            }
        }
    }
}
