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

// idea, make template execution a runtime event. Simply load this object at parse time with the name of the templates to run. Then when execute is called
// load each template and parse it, executing each as you go. This would make template names and even lists of templates dynamic. Would, however, require
// the execute step to know the template object that owns it. This could be a weak pointer assigned at Operation object instantiation.

bool OperationIncludeTemplate::execute(std::shared_ptr<Environments> pEnvironments, std::shared_ptr<Environment> pEnv, std::shared_ptr<Variables> pVariables)
{
    if (m_breakExecution)
    {
        int i = 5;
    }

    if (m_pCondition && !m_pCondition->testCondition(pVariables))
    {
        return false;
    }

    initializeForExecution(pVariables);

    //
    // Now execute the operation count times
    for (size_t idx=0; idx < m_executionCount; ++idx)
    {
        pVariables->setIterationInfo(m_executionStartIndex + idx, idx);

        //
        // Create a vector of parameter values based on this execution of the template
        std::vector<ParameterValue> templateParameters;

        //
        // Add any parameter values to the local variables for this template
        for (auto &parm: m_parameters)
        {
            ParameterValue parameterValue;
            parameterValue.name = parm.name;
            for (auto &parmValue: parm.values)
            {
                try
                {
                    parameterValue.values.emplace_back(pVariables->doValueSubstitution(parmValue));
                }
                catch (TemplateException &te)
                {
                    if (!parm.m_errorIfEmpty)
                    {
                        throw;    // it's an error if the parameter is not conditional, just rethrow the exception
                    }
                }
            }

            //
            // Add the parameter if there are values assigned
            if (!parameterValue.values.empty())
            {
                templateParameters.emplace_back(parameterValue);
            }
        }

        //
        // Note, pEnv is not used here. When the template was instantiated, the default environment was passed from the
        // parent environment during parsing. That's the way templates work.

        //
        // Go execute it each template
        for (auto &pEnvModTemplate: m_envModTemplates)
        {
            pEnvModTemplate->execute((idx == 0), templateParameters);
        }
    }

    return true;
}
