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

#include "Condition.hpp"
#include "Variable.hpp"


bool Condition::testCondition(const std::shared_ptr<Variables> &pVariables) const
{
    //
    // Get the variable
    auto pVar = pVariables->getVariable(pVariables->doValueSubstitution(m_varName), false, false);

    if (m_type == "defined")
    {
        return pVar != nullptr;
    }
    else if (pVar)
    {
        if (m_type == "not_empty")
        {
            return pVar->getNumValues() > 0;
        }
        else if (m_type == "one_of" || m_type == "all_of")
        {
            auto numVariableValues = pVar->getNumValues();
            for (auto const &preTestValue: m_values)
            {
                std::string testValue = pVariables->doValueSubstitution(preTestValue);

                bool match = false;
                for (size_t i = 0; i<numVariableValues && !match; ++i)
                {
                    match = testValue == pVariables->doValueSubstitution(pVar->getValue(i));
                }

                if (m_type == "all_of" && !match)
                {
                    return false;
                }
                else if (m_type == "one_of")
                {
                    return true;
                }
            }
            return true;  // this is the case for "all_of" when all match
        }
    }
    return false;
}
