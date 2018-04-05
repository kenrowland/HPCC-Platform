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


#ifndef _CONFIG2_CONFIGEVENTS_HPP_
#define _CONFIG2_CONFIGEVENTS_HPP_

#include <string>
#include <map>
#include <memory>

class EnvironmentNode;

class EnvironmentEvent
{
    public:

        EnvironmentEvent(const std::string &type) : m_eventType(type) {}
        virtual ~EnvironmentEvent() {}
        virtual bool handleEvent(const std::string &eventType, std::shared_ptr<EnvironmentNode> pEnvNode) { return false; }


    protected:

        std::string m_eventType;
};


class CreateEnvironmentEvent : public EnvironmentEvent
{
    public:

        CreateEnvironmentEvent() : EnvironmentEvent("create") {}
        virtual ~CreateEnvironmentEvent() {}
        void setItemType(const std::string &type) { m_itemType = type; }
        virtual bool handleEvent(const std::string &eventType, std::shared_ptr<EnvironmentNode> pEnvNode);
 

    protected:

        std::string m_itemType;
};


class AttributeDependencyCreateEvent : public CreateEnvironmentEvent
{
    public: 

        AttributeDependencyCreateEvent() {}
        virtual ~AttributeDependencyCreateEvent() {}
        void addDependency(const std::string &attrName, const std::string &attrValr, const std::string &depAttr, const std::string &depVal);
        virtual bool handleEvent(const std::string &eventType, std::shared_ptr<EnvironmentNode> pEnvNode);


    protected:

        std::map<std::string, std::map<std::string, std::pair<std::string, std::string>>> m_depAttrVals;
};




#endif
