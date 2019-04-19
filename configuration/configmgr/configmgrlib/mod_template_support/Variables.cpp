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


void Variables::prepare()
{
    //
    // Prepare all local values
    for (auto &pVar: m_variables)
    {
        std::string preparedValue = pVar->getPreparedValue();
        if (!preparedValue.empty())
        {
            pVar->addValue(doValueSubstitution(preparedValue));
        }
    }
}


//std::string Variables::doValueSubstitution(const std::string &value) const
//{
//    //
//    // A value has the form {{name}}[{{index}}] where name and index can be simple strings and the index is optional
//    // Or {{name}}.size which will return the size of the variable name (number of entries)
//    std::string varName, result = value;
//    std::size_t index;
//    int subIndex = -1;
//
//    std::size_t bracesStartPos = result.find("{{");
//    bool done = bracesStartPos == std::string::npos;
//
//    while (!done)
//    {
//        index = m_curIndex;
//
//        std::size_t bracesEndPos = findClosingDelimiter(result, bracesStartPos,"{{", "}}");
//        varName = result.substr(bracesStartPos + 2, bracesEndPos - bracesStartPos - 2);
//
//        //
//        // If there is an index defined, evaluate it and update the index to be used for the final value
//        std::size_t bracketStartPos = result.find('[');
//        std::size_t sizePos = result.find(".size");
//        std::size_t bracketEndPos = std::string::npos;
//
//        if (bracketStartPos != std::string::npos && sizePos != std::string::npos)
//        {
//            throw TemplateException("Both [] and .size may not appear in a variable");
//        }
//
//        if (bracketStartPos == bracesEndPos + 2)
//        {
//            bracketEndPos = findClosingDelimiter(result, bracketStartPos, "[", "]");  //  result.find(']');
//            //
//            // Index can take form with a . for a subIndex
//            std::string subIndexStr = "-1";
//            std::string indexStr = result.substr(bracketStartPos+1, bracketEndPos - bracketStartPos - 1);
//            std::size_t dotPos = indexStr.find('.');
//            if (dotPos != std::string::npos)
//            {
//                subIndexStr = indexStr.substr(dotPos+1);
//                indexStr = indexStr.substr(0, dotPos);
//            }
//            //varName = result.substr(bracesStartPos + 2, bracketStartPos - bracesStartPos - 2);
//            try
//            {
//                index = std::stoul(evaluate(doValueSubstitution(indexStr)));
//                subIndex = std::stoul(evaluate(doValueSubstitution(subIndexStr)));
//            }
//            catch (...)
//            {
//                throw TemplateException("Non-numeric count found for index value", false);
//            }
//        }
//
//        if (sizePos == bracesEndPos + 2) // != std::string::npos)
//        {
//            std::string substitueValue = std::to_string(getVariable(varName, false, true)->getNumValues());
//            std::string newResult = result.substr(0, bracesStartPos);
//            newResult += substitueValue;
//            newResult += result.substr(sizePos + 5);
//            result = newResult;  //std::to_string(getVariable(varName, false)->getNumValues());
//        }
//        else
//        {
//            std::string substitueValue = doValueSubstitution(getVariable(varName, false)->getValue(index, subIndex));
//            std::string newResult = result.substr(0, bracesStartPos);
//            newResult += substitueValue;
//            newResult += result.substr((bracketEndPos != std::string::npos) ? (bracketEndPos + 1) : (bracesEndPos + 2) );
//            result = newResult;
//        }
//
//        bracesStartPos = result.find("{{");
//        done = bracesStartPos == std::string::npos;
//    }
//
//    //
//    // This should NOT have a [] in it
//
//    return evaluate(result);
//}


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

    std::string varRef, result = value;
    std::size_t index;

    std::size_t bracesStartPos = result.find("{{");
    bool done = bracesStartPos == std::string::npos;

    while (!done)
    {
        index = m_curIndex;  // always start with this value
        bool isSize = false;
        std::string varNameStr, varName;
        std::string indexStr;
        std::string memberStr, memberName;

        //
        // Isolate the variable reference
        std::size_t bracesEndPos = findClosingDelimiter(result, bracesStartPos,"{{", "}}");
        varRef = result.substr(bracesStartPos + 2, bracesEndPos - bracesStartPos - 2);
        getVaribaleNameComponents(varRef, varNameStr, isSize, indexStr, memberStr);

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
        else
        {
            substitueValue = doValueSubstitution(getVariable(varName, false, true)->getValue(index, memberName));
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
}


void Variables::getVaribaleNameComponents(const std::string &varRef, std::string &varName, bool &isSize, std::string &index, std::string &member) const
{
    //
    // The following forms are supported (note the {{ and }} have already been removed)
    //   {{varName}} - value of the variable based on the current execution index
    //   {{varName.member}} - value of "member" of variable
    //   {{varName.size}} - the number of values stored in the variable varName
    //   {{varName[index]}} - the value at the specified index of the variable varName
    //   {{varName[index].member}} - the value of "member" at the specified index

    varName = varRef;   // in case nothing to really do
    member = "";        // assume no member

    std::size_t bracketPos, sizePos, memberPos;

    sizePos = varRef.find(".size");
    bracketPos = varRef.find('[');

    //
    // Double check it's a valid reference
    if (sizePos != std::string::npos && bracketPos != std::string::npos)
    {
        throw TemplateException("Both [] and .size may not appear in a variable");
    }

    //
    // If asking for size....
    if (sizePos != std::string::npos)
    {
        varName = varRef.substr(0, sizePos);
        isSize = true;
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
