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

#ifndef HPCCSYSTEMS_PLATFORM_MEMBERSETVARIABLE_HPP
#define HPCCSYSTEMS_PLATFORM_MEMBERSETVARIABLE_HPP

#include "Variable.hpp"
#include <string>
#include <vector>
#include <map>

class MemberSetVariable : public Variable
{
    public:

        explicit MemberSetVariable(const std::string &name) : Variable(name) {}
        ~MemberSetVariable() override = default;
        void addValue(const std::string &value) override;
        void addMemberName(const std::string &memberName);
        void addMemberValue(const std::string &value, const std::string &memberName);
        std::string getValue(size_t idx, const std::string &member) const override;
        size_t getNumValues() const override;


    protected:

        std::map<std::string, std::vector<std::string>> m_memberValues;

};


#endif //HPCCSYSTEMS_PLATFORM_MEMBERSETVARIABLE_HPP
