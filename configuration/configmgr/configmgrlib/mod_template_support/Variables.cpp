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
#include "Variables.hpp"
#include "TemplateException.hpp"


Variables::Variables(std::shared_ptr<Variables> pGlobalVars)
{
    m_pGlobalVariables = std::move(pGlobalVars);
    initialize();
}


void Variables::add(const std::shared_ptr<Variable> pVariable, bool global)
{
    if (global && m_pGlobalVariables)
    {
        m_pGlobalVariables->add(pVariable, false);
    }
    else
    {
        for (auto &pVar: m_variables)
        {
            if (pVar->getName() == pVariable->getName())
            {
                throw TemplateException("Variable '" + pVariable->getName() + "' is a duplicate.", true);
            }
        }
        m_variables.emplace_back(pVariable);
    }
}


std::shared_ptr<Variable> Variables::getVariable(const std::string &name, bool localOnly, bool throwIfNotFound) const
{
    std::shared_ptr<Variable> pRetVar;
    std::string varName;

    //
    // Accept both a regular string or a {{name}} string for the input name
    std::size_t bracesStartPos = name.find("{{");
    if (bracesStartPos != std::string::npos)
    {
        std::size_t bracesEndPos = findClosingDelimiter(name, bracesStartPos,"{{", "}}");
        varName = name.substr(bracesStartPos + 2, bracesEndPos - bracesStartPos - 2);
    }
    else
    {
        varName = name;
    }

    //
    // First search for a local variable (variable context matches the current context)
    for (auto &pVar: m_variables)
    {
        if (pVar->getName() == varName)
        {
            pRetVar = pVar;
            break;
        }
    }


    //
    // If no match was found, see if there is a global version
    if (!pRetVar && !localOnly && m_pGlobalVariables)
    {
        pRetVar = m_pGlobalVariables->getVariable(varName, false, false);
    }

    if (!pRetVar && throwIfNotFound)
    {
        throw TemplateException("Unable to find variable, name = '" + name + "'.");
    }
    return pRetVar;
}


std::shared_ptr<Variable> Variables::getGlobalVariable(const std::string &name, bool throwIfNotFound) const
{
    if (m_pGlobalVariables)
    {
        return m_pGlobalVariables->getVariable(name, false, throwIfNotFound);
    }
    else
    {
        return getVariable(name, false, throwIfNotFound);
    }
}


std::string Variables::doValueSubstitution(const std::string &value) const
{
    //
    // A value is either a immediate value, or a variable reference.
    // A variable reference has the form (where varName itself can be a variable reference)
    //   {{varName}} - value of the variable based on the current execution index
    //   {{varName.member}} - value of "member" of variable
    //   {{varName.size}} - the number of values stored in the variable varName
    //   {{varName[index]}} - the value at the specified index of the variable varName
    //   {{varName[index].member}} - the value of "member" at the specified index
    // Additionally, a default value can be specified by adding a ':defaultValue' to the
    // end of the variable reference. A default value is not allowed in the .size case

    std::string varRef, result = value;
    std::size_t index;

    std::size_t bracesStartPos = result.find("{{");
    bool done = bracesStartPos == std::string::npos;

    while (!done)
    {
        index = m_curIndex;  // always start with this value
        bool isSize = false;
        bool isList = false;
        std::string varNameStr, varName;
        std::string indexStr;
        std::string memberStr, memberName;
        std::string defaultValueStr;

        //
        // Isolate the variable reference
        std::size_t bracesEndPos = findClosingDelimiter(result, bracesStartPos,"{{", "}}");
        varRef = result.substr(bracesStartPos + 2, bracesEndPos - bracesStartPos - 2);
        getVaribaleNameComponents(varRef, varNameStr, isSize, isList, indexStr, memberStr, defaultValueStr);

        //
        // Do value substitutions on each component of the variable reference
        varName = doValueSubstitution(varNameStr);  // in case the variable name is actually a variable
        if (!indexStr.empty())
        {
            try
            {
                index = std::stoul(evaluate(doValueSubstitution(indexStr)));
            }
            catch (...)
            {
                std::string msg = "Non-numeric count found for index (";
                msg.append(indexStr).append(") value in variable ").append(varRef);
                throw TemplateException(msg, false);
            }
        }
        memberName = doValueSubstitution(memberStr);

        //
        // Do the substitution
        std::string substitueValue;
        if (isSize)
        {
            substitueValue = std::to_string(getVariable(varName, false, true)->getNumValues());
        }

        //
        // If list, then generate, potentially empty, list
        else if (isList)
        {
            result = "";
            auto pVar = getVariable(varName, false, false);
            if (pVar)
            {
                auto numValues = pVar->getNumValues();
                for (size_t i=0; i<numValues; ++i)
                {
                    result.append(pVar->getValue(i));
                    if (i != numValues-1)
                    {
                        result.append(",");
                    }
                }
            }
        }
        else
        {
            auto pVar = getVariable(varName, false, defaultValueStr.empty());
            if (pVar)
            {
                substitueValue = doValueSubstitution(pVar->getValue(index, memberName));
            }
            else
            {
                substitueValue = doValueSubstitution(defaultValueStr);
            }
        }

        std::string newResult = result.substr(0, bracesStartPos);
        newResult += substitueValue;
        newResult += result.substr(bracesEndPos + 2);
        result = newResult;  //std::to_string(getVariable(varName, false)->getNumValues());

        bracesStartPos = result.find("{{");
        done = bracesStartPos == std::string::npos;
    }

    //
    // This should NOT have a [] in it
    return evaluate(result);
}


std::size_t Variables::findClosingDelimiter(const std::string &input, std::size_t startPos, const std::string &openDelim, const std::string &closeDelim) const
{
    std::size_t curPos = startPos + openDelim.length();
    std::size_t openPos, closePos;
    unsigned depth = 1;

    do
    {
        closePos = input.find(closeDelim, curPos);
        openPos = input.find(openDelim, curPos);

        if (closePos == std::string::npos)
        {
            throw TemplateException("Missing closing delimiter '" + closeDelim + "' string = '" + input + "'", false);
        }

        if (openPos != std::string::npos && closePos > openPos)
        {
            ++depth;
            curPos = openPos + openDelim.length();
        }
        else
        {
           if (--depth > 0)
           {
               if (openPos != std::string::npos)
               {
                   ++depth;
                   curPos = openPos + openDelim.length();
               }
               else
               {
                   curPos = closePos + closeDelim.length();
               }
           }
        }
    }
    while (depth > 0);

    return closePos;
}


std::string Variables::evaluate(const std::string &expr) const
{
    std::size_t opPos, dotPos;
    std::string result = expr;

    opPos = expr.find_first_of("+-");
    dotPos = expr.find_first_of('.');  // eliminates any attempt to evaluate an IP address range

    if (opPos != std::string::npos && dotPos == std::string::npos)
    {
        long op1, op2, value;
        try
        {
            op1 = std::stol(expr.substr(0, opPos));
        }
        catch (...)
        {
            return result;
            //throw TemplateException("Non-numeric operand 1 found in expression '" + expr + "'", false);
        }

        try
        {
            op2 = std::stol(expr.substr(opPos+1));
        }
        catch (...)
        {
            return result;
            //throw TemplateException("Non-numeric operand 2 found in expression '" + expr + "'", false);
        }

        if (expr[opPos] == '-')
            value = op1 - op2;
        else
            value = op1 + op2;

        if (value < 0)
        {
            throw TemplateException("Result of expression '" + expr + "' is negative, only >0 results allowed", false);
        }

        result = std::to_string(value);
    }
    return result;
}


void Variables::setIterationInfo(size_t idx, size_t iter)
{
    m_curIndex = idx;
    m_iter = iter;
    auto pIndex = getVariable("loop_index");
    pIndex->setValue(std::to_string(m_curIndex));

    auto pIteration = getVariable("loop_iteration");
    pIteration->setValue(std::to_string(iter));
}


void Variables::setIterationLimits(size_t start, size_t count)
{
    auto pStart = getVariable("loop_start");
    pStart->setValue(std::to_string(start));

    auto pCount = getVariable("loop_count");
    pCount->setValue(std::to_string(count));
}


void Variables::initialize()
{
    //
    // Clear everything out
    m_variables.clear();

    //
    // Create reserved variables
    std::shared_ptr<Variable> pIndex = variableFactory("string", "loop_index");
    add(pIndex, false);
    std::shared_ptr<Variable> pIteration = variableFactory("string", "loop_iteration");
    add(pIteration, false);
    std::shared_ptr<Variable> pStart = variableFactory("string", "loop_start");
    add(pStart, false);
    std::shared_ptr<Variable> pCount = variableFactory("string", "loop_count");
    add(pCount, false);
    std::shared_ptr<Variable> pEmptyStrVar = variableFactory("string", "emptyStr");
    pEmptyStrVar->setValue("");
    add(pEmptyStrVar, false);
}


void Variables::getVaribaleNameComponents(const std::string &varRef, std::string &varName, bool &isSize, bool &isList, std::string &index, std::string &member, std::string &defaultValue) const
{
    //
    // The following forms are supported (note the {{ and }} have already been removed)
    //   {{varName}} - value of the variable based on the current execution index
    //   {{varName.member}} - value of "member" of variable
    //   {{varName.size}} - the number of values stored in the variable varName
    //   {{varName[index]}} - the value at the specified index of the variable varName
    //   {{varName[index].member}} - the value of "member" at the specified index
    // An optional default value can be specified by adding '|defaultValue' to the end
    // of the reference. A default value is not allowed in the .size case
    //
    // Another general note, [index] could be implicity implied for an operation executing over
    // a number of values. The retrieval of the variable value will use the current index. Explicitly
    // using a [index] in the reference overrides any implicit index usage in favor of the
    // supplied index.

    varName = varRef;   // in case nothing to really do
    member = "";        // assume no member
    defaultValue = "";

    //
    // See if there is a default value
    std::size_t barPos = varRef.find('|');
    if (barPos != std::string::npos)
    {
        defaultValue = varName.substr(barPos+1);
        varName.erase(barPos);
    }

    std::size_t bracketPos, sizePos, memberPos, listPos;

    sizePos = varRef.find(".size");
    listPos = varRef.find(".list");
    bracketPos = varRef.find('[');

    //
    // If sizePos or listPos if valid, make sure that it's just that and not a longer name of which a portion matches
    if (sizePos != std::string::npos && varRef.substr(sizePos).length() != 5)
    {
        sizePos = std::string::npos;
    }
    if (listPos != std::string::npos && varRef.substr(listPos).length() != 5)
    {
        listPos = std::string::npos;
    }


    if ((sizePos    != std::string::npos && (bracketPos != std::string::npos || listPos != std::string::npos)) ||
        (bracketPos != std::string::npos && (sizePos != std::string::npos    || listPos != std::string::npos)) ||
        (listPos    != std::string::npos && (bracketPos != std::string::npos || sizePos != std::string::npos)))
    {
        throw TemplateException("Only one of [], .size, or .list may appear in a variable reference");
    }
    else if (barPos != std::string::npos && (bracketPos != std::string::npos || listPos != std::string::npos || sizePos    != std::string::npos))
    {
        throw TemplateException("A default value is not allowed when using .size, [], or .list");
    }

    //
    // If asking for size....
    if (sizePos != std::string::npos)
    {
        varName = varRef.substr(0, sizePos);
        isSize = true;
    }
    else if (listPos != std::string::npos)
    {
        varName = varRef.substr(0, sizePos);
        isList = true;
    }
    else
    {
        //
        // An indexed value ?
        if (bracketPos != std::string::npos)
        {
            std::size_t endBracketPos = findClosingDelimiter(varRef, bracketPos, "[", "]");
            varName = varRef.substr(0, bracketPos);
            index = varRef.substr(bracketPos + 1, endBracketPos - bracketPos - 1);
            memberPos = varRef.find('.', endBracketPos);
        }
        else
        {
            memberPos = varRef.find('.');
        }

        if (memberPos != std::string::npos)
        {
            member = varRef.substr(memberPos + 1);
            if (bracketPos == std::string::npos)
            {
                varName = varRef.substr(0, memberPos);
            }
        }
    }
}
