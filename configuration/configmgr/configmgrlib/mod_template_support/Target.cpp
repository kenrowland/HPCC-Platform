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

#include "Target.hpp"
#include "EnvironmentMgr.hpp"
#include "Variables.hpp"
#include "Variable.hpp"
#include "TemplateException.hpp"


bool Target::getTargetNodeIds(const std::shared_ptr<EnvironmentMgr> &pEnvMgr, const std::shared_ptr<Variables> &pVariables, std::vector<std::string> &nodeIds)
{
    //
    // If the parent node ID is valid, then it is either a variable name or an actual ID (the latter unlikely).
    if (!m_isPath)
    {
        std::shared_ptr<Variable> pInput = pVariables->getVariable(m_target, false, false);
        if (pInput)
        {
            std::size_t numIds = pInput->getNumValues();
            for (std::size_t idx = 0; idx < numIds; ++idx)
            {
                nodeIds.emplace_back(pVariables->doValueSubstitution(pInput->getValue(idx)));
            }
        }
        else
        {
            nodeIds.emplace_back(pVariables->doValueSubstitution(m_target));
        }
    }
    else
    {
        std::vector<std::shared_ptr<EnvironmentNode>> envNodes;
        size_t idx = 0;
        bool done = !pVariables->hasMultiValueVariableReference(m_target);
        do
        {
            try
            {
                pVariables->setIterationInfo(idx, idx);
                pEnvMgr->fetchNodes(pVariables->doValueSubstitution(m_target), envNodes);
            }
            catch (TemplateExceptionVarIndexOutOfBounds &e)
            {
                done = true;
            }
            ++idx;
        } while(!done);

        for (auto &envNode: envNodes)
            nodeIds.emplace_back(envNode->getId());
    }

    return !nodeIds.empty();
}


//std::string Target::getTargetNodeId(const std::shared_ptr<EnvironmentMgr> &pEnvMgr, const std::shared_ptr<Variables> &pVariables)
//{
//    std::string nodeId;
//    //
//    // If the parent node ID is valid, then it is either a variable name or an actual ID (the latter unlikely).
//    if (!m_isPath)
//    {
//        std::shared_ptr<Variable> pInput = pVariables->getVariable(m_target, false, false);
//        if (pInput)
//        {
//            nodeId = pInput->getValue(pVariables->getCurIndex());
//        }
//        else
//        {
//            nodeId = m_target;
//        }
//    }
//    else
//    {
//        std::vector<std::shared_ptr<EnvironmentNode>> envNodes;
//        pEnvMgr->fetchNodes(pVariables->doValueSubstitution(m_target), envNodes);
//        //f (  need to know how to handle multiple nodes here, maybe the get source nodes above is just fine, maybe need to make sure that only one is selected)
//
//        size_t idx = 0;
//        bool done = !pVariables->hasMultiValueVariableReference(m_target);
//        do
//        {
//            try
//            {
//                pVariables->setIterationInfo(idx, idx);
//                pEnvMgr->fetchNodes(pVariables->doValueSubstitution(m_target), envNodes);
//            }
//            catch (TemplateExceptionVarIndexOutOfBounds &e)
//            {
//                done = true;
//            }
//            ++idx;
//        } while(!done);
//
//        for (auto &envNode: envNodes)
//            nodeIds.emplace_back(envNode->getId());
//    }
//
//    return !nodeIds.empty();
//}
