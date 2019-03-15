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

#include "OperationIncludeTemplate.hpp"
#include "EnvModTemplate.hpp"
#include "ParameterValue.hpp"

class EnvironmentMgr;

bool OperationIncludeTemplate::execute(EnvironmentMgr &envMgr, std::shared_ptr<Variables> pVariables)
{
    initializeForExecution(pVariables);

    //
    // Now execute the operation count times
    for (size_t idx=0; idx < m_executionCount; ++idx)
    {
        pVariables->setIterationInfo(m_executionStartIndex + idx, idx);

        //
        // Create a vector of prepared parameter values based on this execution of the template
        std::vector<ParameterValue> preparedValues;

        //
        // Add any parameter values to the local variables for this template
        for (auto &parm: m_parameters)
        {
            ParameterValue preparedValue;
            preparedValue.name = parm.name;
            for (auto &parmValue: parm.values)
            {
                preparedValue.values.emplace_back(pVariables->doValueSubstitution(parmValue));
            }
            preparedValues.emplace_back(preparedValue);
        }

        //
        // Go execute it!
        m_pEnvModTemplate->execute((idx==0), preparedValues);
    }

    return true;
}
