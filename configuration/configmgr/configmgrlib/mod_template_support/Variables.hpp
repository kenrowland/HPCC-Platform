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


#ifndef HPCCSYSTEMS_PLATFORM_INPUTS_HPP
#define HPCCSYSTEMS_PLATFORM_INPUTS_HPP

#include <string>
#include <vector>
#include <memory>
#include <stack>

class Variable;

class Variables : public std::enable_shared_from_this<Variables>
{
    public:

        explicit Variables(std::shared_ptr<Variables> pGlobalVars = std::shared_ptr<Variables>()); // { m_pGlobalVariables = std::move(pGlobalVars); }
        ~Variables() = default;
        void add(std::shared_ptr<Variable> pVariable, bool global = false);
        std::shared_ptr<Variable> getVariable(const std::string &name, bool localOnly = true, bool throwIfNotFound = true) const;
        std::shared_ptr<Variable> getGlobalVariable(const std::string &name, bool throwIfNotFound = true) const;
        std::shared_ptr<Variables> getGlobalVariables() { return (m_pGlobalVariables ? m_pGlobalVariables : shared_from_this()); }
        void setIterationInfo(size_t idx, size_t iter);
        void setIterationLimits(size_t start, size_t count);
        size_t getCurIndex() const { return m_curIndex; }
        size_t getCurIteration() const { return m_iter; }
        std::string doValueSubstitution(const std::string &value) const;
        const std::vector<std::shared_ptr<Variable>> &getAllVariables() const { return m_variables; }
        void initialize();


    protected:

        std::string evaluate(const std::string &expr) const;
        std::size_t findClosingDelimiter(const std::string &input, std::size_t startPos, const std::string &openDelim, const std::string &closeDelim) const;
        void
        getVaribaleNameComponents(const std::string &varRef, std::string &varName, bool &isSize, bool &isList, std::string &index, std::string &member, std::string &defaultValue) const;


    private:

        std::vector<std::shared_ptr<Variable>> m_variables;
        std::shared_ptr<Variables> m_pGlobalVariables;
        size_t m_curIndex = 0;
        size_t m_iter = 0;
};


#endif //HPCCSYSTEMS_PLATFORM_INPUTS_HPP
