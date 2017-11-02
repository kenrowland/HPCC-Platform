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

#include "CfgValue.hpp"
#include "EnvValue.hpp"


bool CfgValue::isValueValid(const std::string &newValue) const
{
    return m_pType->isValueValid(newValue);
}


void CfgValue::validate(Status &status, const std::string &id, const std::string &value) const
{

    // call pType with value to see if good
    // call pType->limits->toString(value) if bad to get message about whats bad
    // add to status

}


void CfgValue::resetEnvironment() 
{ 
    m_envValues.clear();
}


// replicates the new value throughout the environment
void CfgValue::mirrorValue(const std::string &oldValue, const std::string &newValue)
{
    for (auto mirrorCfg = m_mirrorToCfgValues.begin(); mirrorCfg != m_mirrorToCfgValues.end(); ++mirrorCfg)
    {
        (*mirrorCfg)->setMirroredEnvValues(oldValue, newValue);
    }

}


// Worker method for replicating a mirrored value to the environment values for this config value
void CfgValue::setMirroredEnvValues(const std::string &oldValue, const std::string &newValue)
{
    for (auto envIt = m_envValues.begin(); envIt != m_envValues.end(); ++envIt)
    {
        std::shared_ptr<EnvValue> pEnvValue = (*envIt).lock();
        if (pEnvValue && pEnvValue->getValue() == oldValue)
        {
            pEnvValue->setValue(newValue, nullptr, true);  
        }
    }
}


std::vector<std::string> CfgValue::getEnvValues() const
{
    std::vector<std::string> values;
    for (auto it = m_envValues.begin(); it != m_envValues.end(); ++it)
    {
        values.push_back((*it).lock()->getValue());
    }
    return values;
}