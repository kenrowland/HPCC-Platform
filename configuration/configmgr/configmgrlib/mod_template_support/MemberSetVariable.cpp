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

#include "MemberSetVariable.hpp"
#include "TemplateException.hpp"

void MemberSetVariable::addValue(const std::string &value)
{
    throw TemplateException("Variable type MemberSetVariable does not support base addValue method");
}


void MemberSetVariable::addMemberName(const std::string &memberName)
{
    auto retVal = m_memberValues.insert({memberName, std::vector<std::string>()});
    if (!retVal.second)
    {
        std::string msg = "Attempt to add duplicate member name '";
        msg.append(memberName).append("' to MemberSet variable '").append(m_name).append("'");
        throw TemplateException(msg);
    }
}


void MemberSetVariable::addMemberValue(const std::string &value, const std::string &memberName)
{
    auto it = m_memberValues.find(memberName);
    if (it != m_memberValues.end())
    {
        it->second.emplace_back(value);
    }
    else
    {
        std::string msg = "Member name '";
        msg.append(memberName).append("' not found while attempting to add value '").append(value).append("' to MemberSet '").append(m_name).append("'");
        throw TemplateException(msg);
    }
}


std::string MemberSetVariable::getValue(size_t idx, const std::string &member) const
{
    auto it = m_memberValues.find(member);
    if (it != m_memberValues.end())
    {
        if (it->second.size()-1 >= idx)
        {
            return it->second[idx];
        }
        else
        {
            std::string msg = "Index value '";
            msg.append(std::to_string(idx)).append("' exceeds size '").append(std::to_string(it->second.size())).append("' of member '");
            msg.append(member).append("' of MeberSet variable '").append(m_name).append("'");
            throw TemplateException(msg);
        }
    }

    std::string msg = "Member name '";
    msg.append(member).append("' not found while attempting to get value from MemberSet '").append(m_name).append("'");
    throw TemplateException(msg);
}

size_t MemberSetVariable::getNumValues() const
{
    return m_memberValues.begin()->second.size();
}
