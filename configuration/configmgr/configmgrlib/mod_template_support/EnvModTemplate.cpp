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

#include "EnvModTemplate.hpp"
#include "rapidjson/error/en.h"
#include "TemplateException.hpp"
#include "TemplateExecutionException.hpp"
#include "OperationNode.hpp"
#include "OperationCreateNode.hpp"
#include "OperationFindNode.hpp"
#include "OperationModifyNode.hpp"
#include "OperationDeleteNode.hpp"
#include "OperationIncludeTemplate.hpp"
#include "Exceptions.hpp"
#include <sstream>
#include <fstream>
#include "Utils.hpp"
#include "EnvironmentMgr.hpp"

//
// Good online resource for validation of modification templates is https://www.jsonschemavalidator.net/
// Load the schecma (ModTemplateSchema.json) in the left window and the modification template in the right.


//
// Constructor for a new top level root template.
EnvModTemplate::EnvModTemplate(std::shared_ptr<Environment> pEnv, const std::string &schemaFile) :
    m_pTemplate(nullptr),
    m_pSchema(nullptr),
    m_useLocalEnvironmentForTemplate(false),
    m_isRoot(true)
{
    m_pVariables = std::make_shared<Variables>();
    m_pEnvironments = std::make_shared<Environments>();

    m_pEnvironments->add(pEnv, "");  // add the default environment manager

    //
    // Load and compile the schema document
    std::ifstream jsonFile(schemaFile);
    rapidjson::IStreamWrapper jsonStream(jsonFile);
    rapidjson::Document sd;
    if (sd.ParseStream(jsonStream).HasParseError())
    {
        throw(TemplateException(&sd));
    }
    m_pSchema = std::make_shared<rapidjson::SchemaDocument>(sd); // Compile a Document to SchemaDocument
}


//
// Constructor for a child template of the input template reference
EnvModTemplate::EnvModTemplate(const EnvModTemplate &modTemplate) :
    m_pTemplate(nullptr),
    m_useLocalEnvironmentForTemplate(false),
    m_isRoot(false)
{
    //
    // Copy relevant members to avoid duplicate actions. Also, create a new variables
    // object and init it with the global variables
    m_pSchema = modTemplate.m_pSchema;
    m_pVariables =  std::make_shared<Variables>(modTemplate.m_pVariables->getGlobalVariables());
    m_pEnvironments = modTemplate.m_pEnvironments;
}


EnvModTemplate::~EnvModTemplate()
{
    releaseTemplate();
}


void EnvModTemplate::releaseTemplate()
{
    if (m_pTemplate != nullptr)
    {
        delete m_pTemplate;
        m_pTemplate = nullptr;
    }
}


void EnvModTemplate::loadTemplateFromFile(const std::string &fqTemplateFile)
{
    m_templateFile = fqTemplateFile;
    std::ifstream jsonFile(fqTemplateFile);
    rapidjson::IStreamWrapper jsonStream(jsonFile);
    loadTemplate(jsonStream);
}


void EnvModTemplate::loadTemplateFromJson(const std::string &templateJson)
{
    m_templateFile = "JSON";
    std::stringstream json;
    json << templateJson;
    rapidjson::IStreamWrapper jsonStream(json);
    loadTemplate(jsonStream);
}


void EnvModTemplate::loadTemplate(rapidjson::IStreamWrapper &stream)
{
    //
    // Cleanup anything that may be laying around.
    releaseTemplate();
    m_pVariables->initialize();

    m_pTemplate = new rapidjson::Document();

    //
    // Parse and make sure it's valid JSON
    m_pTemplate->ParseStream(stream);
    if (m_pTemplate->HasParseError())
    {
        throw(TemplateException(m_pTemplate));
    }

    if (m_pSchema != nullptr)
    {
        rapidjson::SchemaValidator validator(*m_pSchema);
        if (!m_pTemplate->Accept(validator))
        {
            // Input JSON is invalid according to the schema
            // Output diagnostic information
            std::string msg = "Template, " + m_templateFile + ", failed validation, ";
            rapidjson::StringBuffer sb;
            validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
            msg += " Invalid schema: " + std::string(sb.GetString()) + ", ";
            msg += " Invalid keyword: " + std::string(validator.GetInvalidSchemaKeyword()) + ", ";
            sb.Clear();
            validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
            msg += "Invalid document: " + std::string(sb.GetString());
            throw TemplateException(msg, true);
        }
    }

    //
    // Now, parse the template, all as part of loading it
    parseTemplate();
}


void EnvModTemplate::parseTemplate()
{
    try
    {
        //
        // Parse common stuff from the template
        parseCommon();


        //
        // Inputs
        if (m_pTemplate->HasMember("variables"))
        {
            const rapidjson::Value &variables = (*m_pTemplate)["variables"];
            parseVariables(variables);
        }

        //
        // Environment for the template
        if (m_pTemplate->HasMember("environment"))
        {
            const rapidjson::Value &environmentData = (*m_pTemplate)["variables"];
            parseEnvironment(environmentData);
        }



        //
        // Operations (a required section)
        parseOperations((*m_pTemplate)["operations"]);
    }
    catch (TemplateException &te)
    {
        throw;  // just rethrow
    }
    catch (...)
    {
        throw TemplateException("An exception has occured while parsing the input template", false);
    }
}


void EnvModTemplate::parseCommon()
{
    //
    // Overrides the default environment for the template. Environment is retrieved at runtime
    auto it = m_pTemplate->FindMember("environment");
    if (it != m_pTemplate->MemberEnd())
    {
        m_environmentName = trim(it->value.GetString());
    }

}


void EnvModTemplate::parseVariables(const rapidjson::Value &variables)
{
    for (auto &varDef: variables.GetArray())
    {
        parseVariable(varDef);
    }
}


void EnvModTemplate::parseVariable(const rapidjson::Value &varValue)
{
    rapidjson::Value::ConstMemberIterator it;
    std::shared_ptr<Variable> pVariable;
    bool isGlobal = false;

    //
    // input name, make sure not a duplicate
    std::string varName(varValue.FindMember("name")->value.GetString());
    std::string type(varValue.FindMember("type")->value.GetString());
    pVariable = variableFactory(type, varName);

    //
    // Get and set the rest of the input values
    it = varValue.FindMember("prompt");
    if (it != varValue.MemberEnd())
        pVariable->m_userPrompt = it->value.GetString();
    else
        pVariable->m_userPrompt = "Input value for " + varName;

    it = varValue.FindMember("description");
    if (it != varValue.MemberEnd())
    {
        pVariable->m_description = it->value.GetString();
    }

    it = varValue.FindMember("values");
    if (it != varValue.MemberEnd())
    {
        for (auto &val: it->value.GetArray())
        {
            pVariable->addValue(trim(val.GetString()));
        }
    }

    it = varValue.FindMember("prepared_value");
    if (it != varValue.MemberEnd())
    {
        pVariable->m_preparedValue = trim(it->value.GetString());
    }

    it = varValue.FindMember("user_input");
    if (it != varValue.MemberEnd())
    {
        pVariable->m_userInput = it->value.GetBool();
    }

    //
    // Local variable?
    it = varValue.FindMember("global");
    if (it != varValue.MemberEnd())
    {
        isGlobal = it->value.GetBool();
    }

    m_pVariables->add(pVariable, isGlobal);
}


std::shared_ptr<Variable> EnvModTemplate::getVariable(const std::string &name, bool throwIfNotFound) const
{
    return m_pVariables->getVariable(name, throwIfNotFound);
}


std::vector<std::shared_ptr<Variable>> EnvModTemplate::getVariables(bool userInputOnly) const
{
    std::vector<std::shared_ptr<Variable>> variables;
//    if (!userInputOnly)
//    {
//        variables.insert( variables.end(), m_pVariables->all().begin(), m_pVariables->all().end() );
//    }
//    else
//    {
//        auto allVars = m_pVariables->all();
//        for (auto &varIt: allVars)
//        {
//            if (varIt->m_userInput)
//                variables.emplace_back(varIt);
//        }
//    }

    return variables;
}


void EnvModTemplate::assignVariablesFromFile(const std::string &filepath)
{
    std::ifstream jsonFile(filepath);
    rapidjson::IStreamWrapper jsonStream(jsonFile);

    //
    // Format:
    // {
    //   "variable-name" : [ "value1", "value2" , ..., "valuen"],
    //     ...
    // }

    rapidjson::Document inputJson;

    //
    // Parse and make sure it's valid JSON
    inputJson.ParseStream(jsonStream);
    if (inputJson.HasParseError())
    {
        throw(TemplateException(&inputJson));
    }

    //
    // go through all the inputs and assign values
    for (auto itr = inputJson.MemberBegin();
         itr != inputJson.MemberEnd(); ++itr)
    {
        std::string inputName = itr->name.GetString();
        auto pVariable = m_pVariables->getVariable(inputName);
        auto valueArray = itr->value.GetArray();
        for (auto &val: valueArray)
        {
            pVariable->addValue(val.GetString());
        }
    }
}


void EnvModTemplate::parseOperations(const rapidjson::Value &operations)
{
    for (auto &opDef: operations.GetArray())
    {
        parseOperation(opDef);
    }
}


void EnvModTemplate::parseOperation(const rapidjson::Value &operation)
{

    std::string action(operation.FindMember("action")->value.GetString());

    if (action == "include")
    {
        std::shared_ptr<OperationIncludeTemplate> pIncOp = std::make_shared<OperationIncludeTemplate>();
        auto dataIt = operation.FindMember("data");
        if (dataIt != operation.MemberEnd())
        {
            parseIncludeOperation(dataIt->value, pIncOp);
        }

        pIncOp->m_pEnvModTemplate = std::make_shared<EnvModTemplate>(*this);
        pIncOp->m_pEnvModTemplate->loadTemplateFromFile(pIncOp->m_path);
        m_operations.emplace_back(pIncOp);
    }
    else
    {
        std::shared_ptr<OperationNode> pOp;

        if (action == "create")
        {
            pOp = std::make_shared<OperationCreateNode>();
        }
        else if (action == "modify")
        {
            pOp = std::make_shared<OperationModifyNode>();
        }
        else if (action == "find")
        {
            pOp = std::make_shared<OperationFindNode>();
        }
        else if (action == "delete")
        {
            pOp = std::make_shared<OperationDeleteNode>();
        }

        if (pOp)
        {
            //
            // Parse the data section
            auto dataIt = operation.FindMember("data");
            if (dataIt != operation.MemberEnd())
            {
                parseOperationNodeCommonData(dataIt->value, pOp);

                //
                // Parse specific operation type values not handled by the common data object parser above
                if (action == "create")
                {
                    std::shared_ptr<OperationCreateNode> pCreateOp = std::dynamic_pointer_cast<OperationCreateNode>(pOp);
                    pCreateOp->m_nodeType = dataIt->value.FindMember("node_type")->value.GetString();

                    auto it = dataIt->value.FindMember("populate_children");
                    if (it != dataIt->value.MemberEnd())
                    {
                        pCreateOp->m_populateChildren = it->value.GetBool();
                    }
                }
                else if (action == "find")
                {
                    std::shared_ptr<OperationFindNode> pFindOp = std::dynamic_pointer_cast<OperationFindNode>(pOp);
                    auto it = dataIt->value.FindMember("create_if_not_found");
                    if (it != dataIt->value.MemberEnd())
                    {
                        pFindOp->m_createIfNotFound = it->value.GetBool();
                    }

                    it = dataIt->value.FindMember("node_type");
                    if (it != dataIt->value.MemberEnd())
                    {
                        pFindOp->m_nodeType = dataIt->value.FindMember("node_type")->value.GetString();
                    }
                }
            }
            m_operations.emplace_back(pOp);
        }
    }
}


void EnvModTemplate::parseOperationNodeCommonData(const rapidjson::Value &operationData,
                                                  std::shared_ptr<OperationNode> pOpNode)
{
    rapidjson::Value::ConstMemberIterator it;
    auto dataObj = operationData.GetObject();


    it = dataObj.FindMember("target");
    if (it != dataObj.MemberEnd())
    {
        parseTarget(it->value, pOpNode);
    }

    //
    // Get the count (optional, default is 1)
    it = dataObj.FindMember("count");
    if (it != dataObj.MemberEnd())
    {
        pOpNode->m_count = trim(it->value.GetString());
    }

    //
    // Get the starting index (optional, for windowing into a range of values)
    it = dataObj.FindMember("start_index");
    if (it != dataObj.MemberEnd())
    {
        pOpNode->m_startIndex = trim(it->value.GetString());
    }

    //
    // Save node id
    rapidjson::Value::ConstMemberIterator saveInfoIt = dataObj.FindMember("save");
    if (saveInfoIt != dataObj.MemberEnd())
    {
        pOpNode->m_saveNodeIdName = saveInfoIt->value.GetObject().FindMember("name")->value.GetString();
        it = saveInfoIt->value.GetObject().FindMember("accumulate");
        if (it != saveInfoIt->value.MemberEnd())
        {
            pOpNode->m_accumulateSaveNodeIdOk = it->value.GetBool();
        }

        it = saveInfoIt->value.GetObject().FindMember("global");
        if (it != saveInfoIt->value.MemberEnd())
        {
            pOpNode->m_saveNodeIdAsGlobalValue = it->value.GetBool();
        }
    }

    //
    // Node value?
    rapidjson::Value::ConstMemberIterator nodeValueIt = dataObj.FindMember("node_value");
    if (nodeValueIt != dataObj.MemberEnd())
    {
        parseAttribute(nodeValueIt->value, &pOpNode->m_nodeValue);
        pOpNode->m_nodeValueValid = true;
    }

    //
    // Get the attributes. Note that the allowed key/value pairs for each entry in this array vary by operation type.
    // The schema validates that the member objects are correct based on the operation type.
    it = dataObj.FindMember("attributes");
    if (it != dataObj.MemberEnd())
    {
        rapidjson::Value::ConstMemberIterator attrValueIt;
        std::string attrName, attrValue;
        for (auto &attr: it->value.GetArray())
        {
            modAttribute newAttribute;
            parseAttribute(attr, &newAttribute);
            pOpNode->addAttribute(newAttribute);
        }
    }

    //
    // Throw on empty is a flag that will force an exception to be thrown during execution if no
    // nodes are found to modify.
    it = dataObj.FindMember("error_if_not_found");
    if (it != dataObj.MemberEnd())
    {
        pOpNode->m_throwOnEmpty = it->value.GetBool();
    }
}


void EnvModTemplate::parseAttribute(const rapidjson::Value &attributeValue, modAttribute *pAttribute)
{
    auto attributeData = attributeValue.GetObject();
    //
    // Get the attribute name or set of names. One shall be present based on the schema and operation type
    auto valueIt = attributeData.FindMember("name");
    if (valueIt != attributeData.MemberEnd())
    {
        pAttribute->addName(trim(valueIt->value.GetString()));
    }
    else
    {
        valueIt = attributeData.FindMember("first_of");
        if (valueIt != attributeData.MemberEnd())
        {
            for (auto &nameIt: valueIt->value.GetArray())
            {
                pAttribute->addName(trim(nameIt.GetString()));
            }
        }
    }

    //
    // Remaining attribute values and settings
    valueIt = attributeData.FindMember("value");
    if (valueIt != attributeData.MemberEnd())
    {
        pAttribute->value = trim(valueIt->value.GetString());
    }

    auto attrValueIt = attributeData.FindMember("start_index");
    if (attrValueIt != attributeData.MemberEnd())
    {
        pAttribute->startIndex = trim(attrValueIt->value.GetString());
    }

    attrValueIt = attributeData.FindMember("do_not_set");
    if (attrValueIt != attributeData.MemberEnd())
    {
        pAttribute->doNotSet = attrValueIt->value.GetBool();
    }

    auto saveInfoIt = attributeData.FindMember("save");
    if (saveInfoIt != attributeData.MemberEnd())
    {
        pAttribute->saveVariableName = saveInfoIt->value.GetObject().FindMember("name")->value.GetString();
        attrValueIt = saveInfoIt->value.GetObject().FindMember("accumulate");
        if (attrValueIt != saveInfoIt->value.MemberEnd())
        {
            pAttribute->accumulateValuesOk = attrValueIt->value.GetBool();
        }
        attrValueIt = saveInfoIt->value.GetObject().FindMember("global");
        if (attrValueIt != saveInfoIt->value.MemberEnd())
        {
            pAttribute->saveValueGlobal = attrValueIt->value.GetBool();
        }
    }

    attrValueIt = attributeData.FindMember("error_if_not_found");
    if (attrValueIt != attributeData.MemberEnd())
    {
        pAttribute->errorIfNotFound = attrValueIt->value.GetBool();
    }

    attrValueIt = attributeData.FindMember("error_if_empty");
    if (attrValueIt != attributeData.MemberEnd())
    {
        pAttribute->errorIfEmpty = attrValueIt->value.GetBool();
    }
}


void EnvModTemplate::parseEnvironment(const rapidjson::Value &environmentValue)
{
    auto targetData = environmentValue.GetObject();

    //
    // name is required at a minimum. Note that is may be a variable.
    auto valueIt = targetData.FindMember("name");
    m_environmentName = valueIt->value.GetString();

    //
    // If a master schema file is specified, then schema paths are also required. In this case
    // an environment is being defined for use as opposed to just referencing one that may exist by name
    valueIt = targetData.FindMember("master_schema_filename");
    if (valueIt != targetData.MemberEnd())
    {
        std::string masterSchemaFile = valueIt->value.GetString();

        valueIt = targetData.FindMember("schema_paths");
        std::vector<std::string> paths;
        for (auto &val: valueIt->value.GetArray())
        {
            paths.emplace_back(val.GetString());
        }

        //
        // Make a new Environment object and save it for this template. Note, the environment is
        // instantiated at runtime when the template is executed. Also, the name may be a variable value.
        m_pEnv = std::make_shared<Environment>(masterSchemaFile, paths);

        //
        // These remaining values are only valid when an environment is being defined
        valueIt = targetData.FindMember("load_environment");
        if (valueIt != targetData.MemberEnd())
        {
            m_pEnv->setLoadName(valueIt->value.GetString());
        }

        valueIt = targetData.FindMember("write_environment");
        if (valueIt != targetData.MemberEnd())
        {
            m_pEnv->setOutputName(valueIt->value.GetString());
        }

        valueIt = targetData.FindMember("set_as_target");
        if (valueIt != targetData.MemberEnd())
        {
            m_useLocalEnvironmentForTemplate = valueIt->value.GetBool();
        }
    }

    //
    // Add additional


}


void EnvModTemplate::parseTarget(const rapidjson::Value &targetValue, std::shared_ptr<OperationNode> pOp)
{
    auto targetData = targetValue.GetObject();

    //
    // Note that the target can only be of one type. The schema prevents specifying conflicting values, so the
    // parsing here is common.

    //
    // Path ?
    auto valueIt = targetData.FindMember("path");
    if (valueIt != targetData.MemberEnd())
    {
        pOp->m_path = trim(valueIt->value.GetString());
    }

    //
    // Node ID ?
    valueIt = targetData.FindMember("nodeid");
    if (valueIt != targetData.MemberEnd())
    {
        pOp->m_parentNodeId = trim(valueIt->value.GetString());
    }

    //
    // filepath ?
    valueIt = targetData.FindMember("filepath");
    if (valueIt != targetData.MemberEnd())
    {
        // reuse the path member, the specific operation class will interpret accordingly
        pOp->m_path = trim(valueIt->value.GetString());
    }

}


void EnvModTemplate::parseIncludeOperation(const rapidjson::Value &include, std::shared_ptr<OperationIncludeTemplate> pOpInc)
{
    auto includeObj = include.GetObject();
    auto it = includeObj.FindMember("target");
    if (it != includeObj.MemberEnd())
    {
        auto targetObj = it->value.GetObject();

        auto targetIt = targetObj.FindMember("filepath");
        pOpInc->m_path = targetIt->value.GetString();
    }

    it = includeObj.FindMember("template_parameters");
    if (it != includeObj.MemberEnd())
    {
        for (auto &parmIt: it->value.GetArray())
        {
            ParameterValue parmValue;

            auto parmObject = parmIt.GetObject();

            parmValue.name = trim(parmObject.FindMember("name")->value.GetString());
            auto valueIt = parmObject.FindMember("values");
            for (auto &val: valueIt->value.GetArray())
            {
                parmValue.values.emplace_back(trim(val.GetString()));
            }
            pOpInc->addParameterValue(parmValue);
        }
    }

    it = includeObj.FindMember("count");
    if (it != includeObj.MemberEnd())
    {
        pOpInc->m_count = trim(it->value.GetString());
    }

    // Is an envriomment specified?
    it = includeObj.FindMember("environment");
    if (it != includeObj.MemberEnd())
    {
        pOpInc->m_environmentName = trim(it->value.GetString());
    }

}


void EnvModTemplate::execute(bool isFirst, const std::vector<ParameterValue> &parameters)
{
    //
    // Add any parameter values to the local variables for this template
    for (auto &parm: parameters)
    {
        std::shared_ptr<Variable> pParmVar;

        //
        // If the first time through, create the variable and add it. If one exists, an exception is thrown. Otherwise
        // subsequent iterations will find the variable exists, so retrieve it and clear it for this iteration's values
        if (isFirst)
        {
            pParmVar = variableFactory("string", parm.name);
            m_pVariables->add(pParmVar, false);
        }
        else
        {
            // This should pass
            pParmVar = m_pVariables->getVariable(parm.name, true, true);
            pParmVar->clear();  // clear it since we are going to add this execution's values
        }

        //
        // Now set the values.
        for (auto &parmValue: parm.values)
        {
            pParmVar->addValue(parmValue);
        }
    }

    try
    {
        //
        // Do final prep on inputs. This may set values for inputs that are dependent on previously set inputs
        m_pVariables->prepare();
    }
    catch (TemplateExecutionException &te)
    {
        te.setContext("Variable preparation", m_templateFile);
        throw;
    }


    // todo get environment for the template to make sure there is one

    std::shared_ptr<EnvironmentMgr> pTemplateEnvMgr;

    try
    {
        // substitute the environment name (this may simply be "", the default)
        std::string envName = m_pVariables->doValueSubstitution(m_environmentName);

        //
        // If there is an environment defined for the template, create it
        if (m_pEnv)
        {
            m_pEnv->initialize();
            m_pEnvironments->add(m_pEnv, envName);
            if (m_useLocalEnvironmentForTemplate)
            {
                pTemplateEnvMgr = m_pEnvironments->get(envName)->m_pEnvMgr;
            }
        }
        else
        {
            //
            // envName holds the name, regardless, of the default environment for the template
            pTemplateEnvMgr = m_pEnvironments->get(envName)->m_pEnvMgr;
        }
    }
    catch (TemplateExecutionException &te)
    {
        std::string msg = "Unable to create, initialize, or find environment ";
        msg.append(m_environmentName).append(" error = ").append(te.what());
        throw TemplateExecutionException(msg, "setup", m_templateFile);
    }



    //
    // Execute each operation...
    unsigned opNum = 1;
    for (auto &pOp: m_operations)
    {
        try
        {
            pOp->execute(m_pEnvironments, pTemplateEnvMgr, m_pVariables);
        }
        catch (TemplateExecutionException &te)
        {
            //
            // Set the operation step number and rethrow so user can tell what step caused the problem
            te.setContext(std::to_string(opNum), m_templateFile);
            throw;
        }
        catch (const ParseException &pe)
        {
            std::string msg = pe.what();
            throw TemplateExecutionException(msg, std::to_string(opNum), m_templateFile);
        }
        catch (const TemplateException &te)
        {
            std::string msg = te.what();
            throw TemplateExecutionException(msg, std::to_string(opNum), m_templateFile);
        }
        ++opNum;
    }

    //
    // If this is a root teamplate, save envrionments
    if (m_isRoot)
    {
        m_pEnvironments->save();
    }
}
