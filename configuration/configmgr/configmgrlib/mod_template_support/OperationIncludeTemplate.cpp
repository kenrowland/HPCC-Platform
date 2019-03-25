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


// input env name is that which is being used by the current template. Use it if no name is set


bool OperationIncludeTemplate::execute(std::shared_ptr<Environments> pEnvironments, const std::string &environmentName, std::shared_ptr<Variables> pVariables)
{
    //
    // Set the environment name for the template about to execute. If no name has been set for the template in the
    // operation object (defined in the currently executing template), then use the input name. Note that the input
    // name may be the empty string which is the defaut, in which case it is essentially a noop. If there is a defined
    // override environment name defined in this operation object, then set it as the environment target fot execution
    // of the template. Note that in all cases, the template about to execute may have it's own defined environment
    // that overriees any attempt here to set it.
    if (m_environmentName.empty())
    {
        m_pEnvModTemplate->setTargetEnvironment(environmentName);
    }
    else
    {
        m_pEnvModTemplate->setTargetEnvironment(m_environmentName);
    }

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
