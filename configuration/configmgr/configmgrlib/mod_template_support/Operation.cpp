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

#include "Operation.hpp"
#include "TemplateExecutionException.hpp"


void Operation::initializeForExecution(std::shared_ptr<Variables> pVariables)
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
