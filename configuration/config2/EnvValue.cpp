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

#include "EnvValue.hpp"
#include "EnvironmentNode.hpp"

bool EnvValue::setValue(const std::string &value, Status &status, bool force)
{ 
    bool rc = true;
    std::string oldValue = m_value;
    if (m_pCfgValue)
    {
        if (m_pCfgValue->isValueValid(value))
        {
            m_value = value;
            m_pCfgValue->mirrorValue(oldValue, value);
        }
        else if (force)
        {
            m_value = value;
            m_pCfgValue->mirrorValue(oldValue, value);
            status.addStatusMsg(ok, m_pMyEnvNode.lock()->getId(), m_name, "", "Attribute forced to invalid value");
            rc = true;
        }
    }
    return rc;
}


bool EnvValue::checkCurrentValue()
{
    bool rc = true;
    if (m_pCfgValue)
    {
        if (!m_pCfgValue->isValueValid(m_value))
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


bool EnvValue::isValueValid(const std::string &value) const
{
    bool rc = true;
    if (m_pCfgValue)
    {
        //
        // Check the value against the type
        if (m_pCfgValue->isValueValid(value))
        {
            //
            // If this is a key, make sure it's unique
            if (m_pCfgValue->isKey())
            {
                std::string fieldName = m_pCfgValue->getName();
                std::shared_ptr<EnvironmentNode> pMyEnvNode = m_pMyEnvNode.lock();
                if (pMyEnvNode)
                {
                    // todo: use getAllFieldValues

                    //
                    // Is this a key value? If so, make sure our value is unique for all the values. Note that we are likely an attribute here
                    //if (m_pCfgValue->isKey())
                    //{
                    //    pMyEnvNode
                    //}

                    // need the parent of myenvnode
                    // then get from that parent, get all fieldNames for children with name myenvnode->getName()
                    // get all values for fieldname from pParent
                }
            }
        }

        //
        // If this is a key, then we 
    }
    return rc;
}


bool EnvValue::validate() const
{
    return isValueValid(m_value);
}

