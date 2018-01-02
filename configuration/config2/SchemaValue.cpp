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

#include "SchemaValue.hpp"
#include "EnvironmentValue.hpp"


bool SchemaValue::isValueValid(const std::string &value, const EnvironmentValue *pEnvValue) const
{
    bool isValid = true;   // assume valid

    //
    // Check the type
    isValid = m_pType->isValueValid(value);

    //
    // Keyed ?, then value must NOT be in the current list.
    if (isValid && isUniqueValue() && pEnvValue != nullptr)
    {
        bool found = false;
        std::vector<std::string> allValues = pEnvValue->getAllValues();
        for (auto it = allValues.begin(); it != allValues.end() && !found; ++it)
            found = *it == value;

        isValid = !found;
    }

    //
    // Keyref ?, then the value must be from another set
    if (isValid && isFromUniqueValueSet() && pEnvValue != nullptr)
    {
        bool found = false;
        std::vector<std::string> allValues = getAllKeyRefValues(pEnvValue);
        for (auto it = allValues.begin(); it != allValues.end() && !found; ++it)
            found = *it == value;
        isValid = found;  
    }
    return isValid;
}


void SchemaValue::validate(Status &status, const std::string &id, const EnvironmentValue *pEnvValue) const
{
    // get currentvalue from pEnvValue
    // for keyed, make sure all values are unique
    // call pType with value to see if good
    // call pType->limits->toString(value) if bad to get message about whats bad
    // add to status

}


void SchemaValue::resetEnvironment()
{ 
    m_envValues.clear();
}


// replicates the new value throughout the environment
void SchemaValue::mirrorValueToEnvironment(const std::string &oldValue, const std::string &newValue)
{
    for (auto mirrorCfgIt = m_mirrorToCfgValues.begin(); mirrorCfgIt != m_mirrorToCfgValues.end(); ++mirrorCfgIt)
    {
        (*mirrorCfgIt)->setMirroredEnvValues(oldValue, newValue);
    }
}


// Worker method for replicating a mirrored value to the environment values for this config value
void SchemaValue::setMirroredEnvValues(const std::string &oldValue, const std::string &newValue)
{
    for (auto envIt = m_envValues.begin(); envIt != m_envValues.end(); ++envIt)
    {
        std::shared_ptr<EnvironmentValue> pEnvValue = (*envIt).lock();
        if (pEnvValue && pEnvValue->getValue() == oldValue)
        {
            pEnvValue->setValue(newValue, nullptr, true);  
        }
    }
}


std::vector<std::string> SchemaValue::getAllEnvValues() const
{
    std::vector<std::string> values;
    for (auto it = m_envValues.begin(); it != m_envValues.end(); ++it)
    {
        values.push_back((*it).lock()->getValue());
    }
    return values;
}


std::vector<AllowedValue> SchemaValue::getAllowedValues(const EnvironmentValue *pEnvValue) const
{
    std::vector<AllowedValue> allowedValues;

    //
    // Either the type is enumerated, or there is a keyref.
    if (m_pType->isEnumerated())
    {
        allowedValues = m_pType->getAllowedValues();
    }
    else if (isFromUniqueValueSet() && pEnvValue != nullptr)
    {
        std::vector<std::string> refValues = getAllKeyRefValues(pEnvValue);
        for (auto it = refValues.begin(); it != refValues.end(); ++it)
        {
            allowedValues.push_back({ *it, "" });
        }
    }
    return allowedValues;
}


std::vector<std::string> SchemaValue::getAllKeyRefValues(const EnvironmentValue *pEnvValue) const
{
    std::vector<std::string> keyRefValues;
    std::vector<std::weak_ptr<SchemaValue>> refCfgValues = getUniqueValueSetRefs();
    for (auto refCfgValueIt = refCfgValues.begin(); refCfgValueIt != refCfgValues.end(); ++refCfgValueIt)
    {
        std::shared_ptr<SchemaValue> pRefCfgValue = (*refCfgValueIt).lock();
        std::vector<std::string> allValues = pRefCfgValue->getAllEnvValues();
        keyRefValues.insert(keyRefValues.end(), allValues.begin(), allValues.end());
    }
    return keyRefValues;
}