/*##############################################################################

HPCC SYSTEMS software Copyright (C) 2018 HPCC Systems®.

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

#include "EnvironmentEvents.hpp"
#include "EnvironmentNode.hpp"
#include "EnvironmentValue.hpp"

void AttributeDependencyCreateEvent::addDependency(const std::string &attrName, const std::string &attrVal, const std::string &depAttr, const std::string &depVal)
{
    auto valSetMapIt = m_depAttrVals.find(attrName);
    if (valSetMapIt == m_depAttrVals.end())
    {
        std::map<std::string, std::pair<std::string, std::string>> keyValMap;
        m_depAttrVals[attrName] = keyValMap;
    }

    m_depAttrVals[attrName][attrVal] = std::pair<std::string, std::string>(depAttr, depVal);
}


bool CreateEnvironmentEvent::handleEvent(const std::string &eventType, std::shared_ptr<EnvironmentNode> pEnvNode)
{
    return pEnvNode->getSchemaItem()->getItemType() == m_itemType;
}


bool AttributeDependencyCreateEvent::handleEvent(const std::string &eventType, std::shared_ptr<EnvironmentNode> pEnvNode)
{
    bool rc = false;
    if (CreateEnvironmentEvent::handleEvent(eventType, pEnvNode))
    {
        for (auto attrIt = m_depAttrVals.begin(); attrIt != m_depAttrVals.end(); ++attrIt)
        {
            std::shared_ptr<EnvironmentValue> pAttr = pEnvNode->getAttribute(attrIt->first);
            if (pAttr && pAttr->getSchemaValue()->getType()->isEnumerated())
            {
                rc = true;   // we handled at least one
                for (auto valueIt = attrIt->second.begin(); valueIt != attrIt->second.end(); ++valueIt)
                {
                    pAttr->getSchemaValue()->getType()->getLimits()->addDependentAttributeValue(valueIt->first, valueIt->second.first, valueIt->second.second);
                }
            }
        }
    }
    return rc;
}
