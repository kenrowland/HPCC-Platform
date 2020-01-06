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

#include "EnvironmentValue.hpp"
#include "EnvironmentNode.hpp"
#include "EnvironmentMgr.hpp"
#include "OperationNode.hpp"
#include "Operation.hpp"
#include "TemplateExecutionException.hpp"


void Operation::initializeForExecution(const std::shared_ptr<Variables> &pVariables)
{
    //
    // Determine the count of iterations to do
    std::string countStr = pVariables->doValueSubstitution(m_count);
    try
    {
        m_executionCount = std::stoul(countStr);
    }
    catch (...)
    {
        throw TemplateExecutionException("Non-numeric count found for count");
    }


    //
    // Starting index
    std::string startIdxStr = pVariables->doValueSubstitution(m_startIndex);
    try
    {
        m_executionStartIndex = std::stoul(startIdxStr);
    }
    catch (...)
    {
        throw TemplateExecutionException("Non-numeric count found for start index");
    }

    pVariables->setIterationLimits(m_executionCount, m_executionCount);
}


//std::vector<std::string> Operation::getNodeIds(const std::shared_ptr<EnvironmentMgr> &pEnvMgr, const std::shared_ptr<Variables> &pVariables,
//                                                   const std::string &nodeId, const std::string &path)
//{
//    std::vector<std::string> nodeIds;
//
//    //
//    // If the parent node ID is valid, then it is either a variable name or an actual ID (the latter unlikely).
//    if (!nodeId.empty())
//    {
//        std::shared_ptr<Variable> pInput = pVariables->getVariable(nodeId, false, false);
//        if (pInput)
//        {
//            std::size_t numIds = pInput->getNumValues();
//            for (std::size_t idx = 0; idx < numIds; ++idx)
//            {
//                nodeIds.emplace_back(pVariables->doValueSubstitution(pInput->getValue(idx)));
//            }
//        }
//        else
//        {
//            nodeIds.emplace_back(pVariables->doValueSubstitution(nodeId));
//        }
//    }
//    else
//    {
//        std::vector<std::shared_ptr<EnvironmentNode>> envNodes;
//        pEnvMgr->fetchNodes(pVariables->doValueSubstitution(path), envNodes);
//        for (auto &envNode: envNodes)
//            nodeIds.emplace_back(envNode->getId());
//    }
//
//    return nodeIds;
//}
