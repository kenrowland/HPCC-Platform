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

#include "OperationNode.hpp"
#include "TemplateExecutionException.hpp"
#include "TemplateException.hpp"
#include "EnvironmentMgr.hpp"
#include "EnvironmentNode.hpp"
#include "EnvironmentValue.hpp"
#include "Operation.hpp"


void OperationNode::addAttribute(modAttribute &newAttribute)
{
    m_attributes.emplace_back(newAttribute);
}


bool OperationNode::execute(std::shared_ptr<Environments> pEnvironments, std::shared_ptr<Environment> pEnv, std::shared_ptr<Variables> pVariables)
{
    bool rc = true;

    if (m_pCondition && !m_pCondition->testCondition(pVariables))
    {
        return false;
    }

    //
    // Get ready to execute the operation
    initializeForExecution(pEnvironments, pEnv, pVariables);

    //
    // Select the nodes for execution
    m_parentNodeIds = getNodeIds(m_pOpEnvMgr, pVariables, m_parentNodeId, m_path);

    //
    // If count is > 1, then the number of selected nodes must be 1
    if (m_executionCount > 1 && m_parentNodeIds.size() != 1)
    {
        throw TemplateExecutionException("Operation execution count > 1 attempted when selected node count is not 1");
    }

    //
    // Now execute the operation count times
    for (size_t idx=0; idx < m_executionCount; ++idx)
    {
        pVariables->setIterationInfo(m_executionStartIndex + idx, idx);
        assignAttributeCookedValues(pVariables);
        doExecute(m_pOpEnvMgr, pVariables);
    }
    return rc;
}


void OperationNode::assignAttributeCookedValues(const std::shared_ptr<Variables> &pVariables)
{
    //
    // go through the ones defined by the operation and set each (by name)
    for (auto &attr: m_attributes)
    {
        try
        {
            attr.cookedValue = pVariables->doValueSubstitution(attr.value);
        }
        catch (TemplateException &te)
        {
            if (!attr.optional)
            {
                throw;
            }
            attr.cookedValue.clear();  // empty string
        }
    }
}


std::shared_ptr<Variable> OperationNode::createVariable(std::string varName, const std::string &varType,
                                                    std::shared_ptr<Variables> pVariables, bool accumulateOk, bool global, bool clear)
{
    std::shared_ptr<Variable> pVariable;
    bool existingOk = accumulateOk | clear;

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
        else if (!existingOk && pVariables->getCurIteration() == 0)
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
        else if (!existingOk && pVariables->getCurIteration() == 0)
        {
            std::string msg = "Attempt to create global variable that already exists: " + varName;
            throw TemplateExecutionException(msg);
        }
    }

    if (clear)
    {
        pVariable->clear();
    }

    return pVariable;
}


bool OperationNode::createAttributeSaveInputs(const std::shared_ptr<Variables> &pVariables)
{
    bool rc = false;
    for (auto &attr: m_attributes)
    {
        //
        // If this is a saved attribute value, make sure the input exists
        if (!attr.saveVariableName.empty())
        {
            createVariable(attr.saveVariableName, "string", pVariables, attr.accumulateValuesOk, attr.saveValueGlobal, attr.clear);
            rc = true;
        }
    }
    return rc;
}


void OperationNode::saveAttributeValues(const std::shared_ptr<Variables> &pVariables, const std::shared_ptr<EnvironmentNode> &pEnvNode)
{
    for (auto &attr: m_attributes)
    {
        if (!attr.saveVariableName.empty())
        {
            bool found=false, set=false;
            std::shared_ptr<Variable> pInput = pVariables->getVariable(attr.saveVariableName, false);
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


void OperationNode::processNodeValue(const std::shared_ptr<Variables> &pVariables, const std::shared_ptr<EnvironmentNode> &pEnvNode)
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
            std::shared_ptr<Variable> pVar = createVariable(m_nodeValue.saveVariableName, "string", pVariables, m_nodeValue.accumulateValuesOk,
                    m_nodeValue.saveValueGlobal, m_nodeValue.clear);
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


void OperationNode::initializeForExecution(const std::shared_ptr<Environments> &pEnvironments, std::shared_ptr<Environment> pEnv,
                                           const std::shared_ptr<Variables> &pVariables)
{
    Operation::initializeForExecution(pVariables);

    //
    // If there is an override for the environment used by the node, get it (it may be a variable)
    m_pOpEnvMgr = pEnv->m_pEnvMgr;
    if (!m_environmentName.empty())
    {
        m_pOpEnvMgr = pEnvironments->get(pVariables->doValueSubstitution(m_environmentName))->m_pEnvMgr;
    }
}
