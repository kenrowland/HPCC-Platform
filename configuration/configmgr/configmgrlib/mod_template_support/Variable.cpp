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

#include "Variable.hpp"
#include "TemplateException.hpp"
#include "IPAddressRangeVariable.hpp"
#include "IPAddressVariable.hpp"
#include "HostNameVariable.hpp"
#include "MemberSetVariable.hpp"
#include <memory>

// todo add more types that correspond to XSD types so that modification template inputs can be validated earlier

std::shared_ptr<Variable> variableFactory(const std::string &type, const std::string &name)
{
    std::shared_ptr<Variable> pVariable;
    if (type == "string")
    {
        pVariable = std::make_shared<Variable>(name);
    }
    else if (type == "iprange")
    {
        pVariable = std::make_shared<IPAddressRangeVariable>(name);
    }
    else if (type == "ipaddress")
    {
        pVariable = std::make_shared<IPAddressVariable>(name);
    }
    else if (type == "hostname")
    {
        pVariable = std::make_shared<HostNameVariable>(name);
    }
    else if (type == "member_set")
    {
        pVariable = std::make_shared<MemberSetVariable>(name);
    }
    else
    {
        throw TemplateException("Invalid variable type '" + type + "'");
    }
    return pVariable;
}


void Variable::setValue(const std::string &value)
{
    if (m_values.empty())
    {
        addValue(value);
    }
    else if (m_values.size() == 1)
    {
        m_values[0] = value;
    }
    else
    {
        std::string msg = "Attempt to set value of non-scalar variable '" + m_name + "'";
        throw TemplateException(msg, false);
    }
}


std::string Variable::getValue(size_t idx, const std::string &member) const
{
    //
    // If member is set, throw an error since base variable does not support members
    if (!member.empty())
    {
        std::string msg = "Attempt to get value of '" + m_name + "." + member +", member does not exist";
        throw TemplateException(msg, false);
    }

    //
    // If no value assigned yet, throw an exception
    if (m_values.empty())
    {
        std::string msg = "Attempt to get value of uninitialized variable '" + m_name + "'";
        throw TemplateException(msg, false);
    }

    //
    // If there is only one value, then it's a single value input, so just return the first index regardless
    if (m_values.size() == 1)
    {
        idx = 0;
    }

    //
    // Otherwise, make sure the requested index is in range
    else if (idx >= m_values.size())
    {
        std::string msg = "Attempt to get value out of range (" + std::to_string(idx) + "), size = " + std::to_string(m_values.size()) + " for '" + m_name +"'";
        throw TemplateExceptionVarIndexOutOfBounds(msg, false);
    }

    return m_values[idx];
}


void Variable::addValue(const std::string &value)
{
    m_values.emplace_back(value);
}
