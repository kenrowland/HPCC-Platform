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

#include "EnvironmentValue.hpp"
#include "EnvironmentNode.hpp"

bool EnvironmentValue::setValue(const std::string &value, Status *pStatus, bool forceSet)
{ 
    bool rc = true;
    std::string oldValue = m_value;
    if (m_pSchemaValue)
    {
        m_forcedSet = false;
        if (m_pSchemaValue->isValueValid(value))
        {
            m_value = value;
            m_valueSet = true;
            m_pSchemaValue->mirrorValueToEnvironment(oldValue, value);
        }
        else if (forceSet)
        {
            m_value = value;
            m_valueSet = true;
            m_forcedSet = true;
            m_pSchemaValue->mirrorValueToEnvironment(oldValue, value);
            if (pStatus != nullptr)
                pStatus->addStatusMsg(statusMsg::info, m_pMyEnvNode.lock()->getId(), m_name, "", "Attribute forced to invalid value");
            rc = true;
        }
        else
        {
            if (pStatus != nullptr)
                pStatus->addStatusMsg(statusMsg::error, m_pMyEnvNode.lock()->getId(), m_name, "", "Value not set. New value(" + value + ") not valid");
            //todo, use the cfgValue->cfgType->getstring or whatever to get a status message as to why it's not valid (in line after the not valid above)
        }
    }
    return rc;
}


bool EnvironmentValue::checkCurrentValue()
{
    bool rc = true;
    if (m_pSchemaValue)
    {
        if (!m_pSchemaValue->isValueValid(m_value))
        {
            rc = false;
        }
    }
    else
    {
        rc = false;
    }
    return rc;
}


std::vector<std::string> EnvironmentValue::getAllValues() const
{
    std::shared_ptr<EnvironmentNode> pEnvNode = m_pMyEnvNode.lock();
    return pEnvNode->getAllFieldValues(m_pSchemaValue->getName());
}


bool EnvironmentValue::isValueValid(const std::string &value) const
{
    return m_pSchemaValue->isValueValid(value, this);
}


void EnvironmentValue::validate(Status &status, const std::string &myId) const
{

    if (!m_pSchemaValue->isDefined())
        status.addStatusMsg(statusMsg::warning, myId, m_name, "", "No type information exists");

    if (m_forcedSet)
        status.addStatusMsg(statusMsg::warning, myId, m_name, "", "Current value was force set to an invalid value");

    // Will generate status based on current value and type
    m_pSchemaValue->validate(status, myId, this);
}


// Called when a new value has been created, not read from existing environment, but created and added 
void EnvironmentValue::initialize()
{
    //
    // Is there an auto generated value we should process:
    const std::string &type = m_pSchemaValue->getAutoGenerateType();
    if (type != "")
    {
        if (type == "prefix")
        {
            std::string newName;
            const std::string &prefix = m_pSchemaValue->getAutoGenerateValue();
            std::vector<std::string> curValues = m_pMyEnvNode.lock()->getAllFieldValues(m_name);
            size_t count = curValues.size();
            for (size_t n = 1; n <= count + 1; ++n)
            {
                newName = prefix + std::to_string(n);
                bool found = false;
                for (auto it = curValues.begin(); it != curValues.end() && !found; ++it)
                {
                    if ((*it) == newName)
                        found = true;
                }

                if (!found)
                {
                    setValue(newName, nullptr);
                    break;
                }
            }
        }
        else if (type == "configProperty")
        {
            std::string value;
            value = m_pMyEnvNode.lock()->getSchemaItem()->getProperty("componentName");
            setValue(value, nullptr);
        }
        // todo throw here
    }

}