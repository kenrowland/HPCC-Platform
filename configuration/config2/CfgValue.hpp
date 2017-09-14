/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2015 HPCC Systems®.

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

#ifndef _CONFIG2_VALUE_HPP_
#define _CONFIG2_VALUE_HPP_

#include <memory>
#include "CfgType.hpp"


class CfgValue 
{
    public:

        CfgValue(const std::string &name) : m_name(name), m_displayName(name), m_required(true), m_readOnly(false), m_hidden(false), m_defaultSet(false), m_deprecated(false) { }
        virtual ~CfgValue() { }
        void setType(const std::shared_ptr<CfgType> &pType);
        const std::string &getName() const { return m_name; }
        bool isValueValid(const std::string &newValue) const;
        //bool setValue(const std::string &newValue) { return true; }
        void setDisplayName(const std::string &displayName) { m_displayName = displayName; }
        const std::string &getDisplayName() const { return m_displayName; }
        void setRequired(bool reqd) { m_required = reqd; }
        bool isRequired() const { return m_required; }
		void setDefault(const std::string &dflt) { m_default = dflt; m_defaultSet = true; }
        const std::string &getDefault() const { return m_default; }
		bool hasDefaultValue() const { return m_defaultSet; }
        void setReadOnly(bool readOnly) { m_readOnly = readOnly; }
        bool isReadOnly() const { return m_readOnly; }
        void setHidden(bool hidden) { m_hidden = hidden; }
        bool isHidden() const { return m_hidden; }
		void setDeprecated(bool deprecated) { m_deprecated = deprecated; }
		bool isDeprecated() const { return m_deprecated; }
        void setToolTip(const std::string &toolTip) { m_toolTip = toolTip; }
        const std::string &getToolTip() const { return m_toolTip; }


    protected:

        std::shared_ptr<CfgType> m_pType;
        std::string m_name;
        std::string m_displayName;
        bool m_required;
        bool m_readOnly;
        bool m_hidden;
		bool m_defaultSet;
		bool m_deprecated;
        std::string m_default;
        std::string m_toolTip;

};



#endif // _CONFIG2_VALUE_HPP_