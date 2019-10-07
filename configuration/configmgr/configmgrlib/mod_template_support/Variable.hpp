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


#ifndef HPCCSYSTEMS_PLATFORM_ENVMODTEMPLATEINPUT_HPP
#define HPCCSYSTEMS_PLATFORM_ENVMODTEMPLATEINPUT_HPP


#include <string>
#include <memory>
#include <vector>
#include "platform.h"
#include "Cfgmgrlib.hpp"


class CFGMGRLIB_API Variable
{
    public:

        explicit Variable(const std::string &name) : m_name(name), m_userInput(false) {}
        virtual ~Variable() = default;
        std::string getName() const { return m_name; }
        void setName(std::string name) { m_name = name; }
        std::string getUserPrompt() const { return m_userPrompt; }
        std::string getDescription() const { return m_description; }

        virtual size_t getNumValues() const { return m_values.size(); }
        virtual void addValue(const std::string &value);
        virtual void setValue(const std::string &value);
        void clear() { m_values.clear(); }
        virtual std::string getValue(size_t idx) const { return getValue(idx, ""); }
        virtual std::string getValue(size_t idx, const std::string &member) const;
        bool isUserInput() const { return m_userInput; }
        virtual std::string getType() const { return "string"; }


    protected:

        std::string m_name;
        std::string m_userPrompt;
        std::string m_description;
        bool m_userInput = true;
        std::vector<std::string> m_values;


    friend class EnvModTemplate;
};

std::shared_ptr<Variable> variableFactory(const std::string &type, const std::string &name);

#endif //HPCCSYSTEMS_PLATFORM_ENVMODTEMPLATEINPUT_HPP
