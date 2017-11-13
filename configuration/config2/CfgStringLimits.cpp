/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2017 HPCC Systems®.

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

#include "CfgStringLimits.hpp"
#include "EnvValue.hpp"

std::string CfgStringLimits::getString() const
{

    std::string limitStr = "";

    if (m_length != 0)
    {
        //limitStr += "string " + m_length + " charcters in length.";
    }
    else 
    {
        //limitStr += "string between " + getMin() + " and ";
        if (getMax() == INT_MAX)
        {
            limitStr += " unlimited ";
        }
        else
        {
            limitStr += getMax();
        }
        limitStr += "characters in length.";

    }

    return limitStr;
}


std::vector<AllowedValue> CfgStringLimits::getEnumeratedKeyValues(const std::shared_ptr<EnvValue> &pEnvValue) const
{
    std::vector<AllowedValue> allowedValues;
    std::vector<std::string> allValues = pEnvValue->getAllValues();

    for (auto it = allValues.begin(); it != allValues.end(); ++it)
    {
        allowedValues.push_back(AllowedValue(*it));
    }
    return allowedValues;
}