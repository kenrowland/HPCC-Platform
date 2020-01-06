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
#include "OperationModifyNode.hpp"
#include "OperationDeleteNode.hpp"
#include "OperationIncludeTemplate.hpp"
#include "OperationInsertRaw.hpp"
#include "MemberSetVariable.hpp"
#include "Target.hpp"
#include "Exceptions.hpp"
#include <sstream>
#include <fstream>
#include "Utils.hpp"
#include "EnvironmentMgr.hpp"
#include "jfile.hpp"
#include "platform.h"

//
// Good online resource for validation of modification templates is https://www.jsonschemavalidator.net/
// Load the schecma (ModTemplateSchema.json) in the left window and the modification template in the right.


//
// Constructor for a new top level root template.
EnvModTemplate::EnvModTemplate(std::shared_ptr<Environment> &pEnv, const std::string &templateJsonSchemaFile, const std::string &workingDir) :
    m_pTemplate(nullptr),
    m_pSchema(nullptr),
    m_useLocalEnvironmentForTemplate(false),
    m_isRoot(true),
    m_ignoreEmptyTemplate(false),
    m_workingDir(workingDir)
{
    //
    // Create the variables object
    m_pVariables = std::make_shared<Variables>();

    //
    // Setup the environments
    m_pEnvironments = std::make_shared<Environments>();
    m_pDefaultEnv = pEnv;
    m_pEnvironments->add(m_pDefaultEnv, "default");

    //
    // Load and compile the schema document
    std::ifstream jsonFile(templateJsonSchemaFile);
    rapidjson::IStreamWrapper jsonStream(jsonFile);
    rapidjson::Document sd;
    if (sd.ParseStream(jsonStream).HasParseError())
    {
        throw(TemplateException(&sd, m_templateFile));
    }
    m_pSchema = std::make_shared<rapidjson::SchemaDocument>(sd); // Compile a Document to SchemaDocument
}


//
// Constructor for a child template of the input template reference
EnvModTemplate::EnvModTemplate(const EnvModTemplate &modTemplate) :
    m_pTemplate(nullptr),
    m_useLocalEnvironmentForTemplate(false),
    m_isRoot(false),
    m_ignoreEmptyTemplate(false)
{
    //
    // Copy relevant members to avoid duplicate actions. Also, create a new variables
    // object and init it with the global variables
    m_pSchema = modTemplate.m_pSchema;
    m_pVariables =  std::make_shared<Variables>(modTemplate.m_pVariables->getGlobalVariables());
    m_pEnvironments = modTemplate.m_pEnvironments;
    m_pDefaultEnv = modTemplate.m_pDefaultEnv;   // propagate the default environment
    m_workingDir = modTemplate.m_workingDir;
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


void EnvModTemplate::loadTemplateFromFile(const std::string &filename)
{
    bool isAbsolutePath = false;

#ifndef _WIN32
    isAbsolutePath = filename[0] == PATHSEPCHAR;
#else
    isAbsolutePath = (filename.length() > 1 && filename[1] == ':');
    isAbsolutePath |= filename[0] == '\\';
#endif

    m_templateFile = isAbsolutePath ? filename : (m_workingDir + filename);
    std::ifstream jsonFile(m_templateFile);
    rapidjson::IStreamWrapper jsonStream(jsonFile);
    try
    {
        loadTemplate(jsonStream);
    }
    catch (TemplateException &te)
    {
        throw;
    }
    catch (...)
    {
        std::string msg = "There was an problem processing the json in template file " + m_templateFile;
        throw TemplateException(msg, true);
    }
    // todo need to catch here to detect file not found
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
    rapidjson::ParseResult pr = m_pTemplate->ParseStream(stream);
    if (m_pTemplate->HasParseError())
    {
        //
        // Ignore empty (probably no template file found) if indicated
        if (m_ignoreEmptyTemplate && pr.Code() == rapidjson::kParseErrorDocumentEmpty)
        {
            return;
        }
        throw(TemplateException(m_pTemplate, m_templateFile));
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
            const rapidjson::Value &environmentData = (*m_pTemplate)["environment"];
            parseEnvironment(environmentData);
        }

        //
        // Operations (a required section)
        parseOperations((*m_pTemplate)["operations"]);
    }
    catch (TemplateException &te)
    {
        throw;
    }
    catch (...)
    {
        throw TemplateException("An unknown exception has occured while parsing the input template", false);
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
    bool overwriteOk = false;
    bool alreadyAdded = false;

    //
    // input name, make sure not a duplicate
    std::string varName(varValue.FindMember("name")->value.GetString());
    std::string type(varValue.FindMember("type")->value.GetString());

    //
    // Get pointer to variable based on whether we are overwiting and if the variable already exists. If not, create one
    it = varValue.FindMember("overwrite_ok");
    if (it != varValue.MemberEnd())
    {
        overwriteOk = it->value.GetBool();
    }

    if (overwriteOk)
    {
        pVariable = m_pVariables->getVariable(varName, false, false);
        if (pVariable)
        {
            pVariable->clear();
            alreadyAdded = true;
        }
    }

    if (!pVariable)
    {
        pVariable = variableFactory(type, varName);
    }

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
            std::string varVal = val.GetString();
            try
            {
                pVariable->addValue(m_pVariables->doValueSubstitution(varVal));
            }
            catch (TemplateException &te)
            {
                std::string msg = "Error assigning value '" + varVal + "' to variable '" + pVariable->getName() + "' in template '" + m_templateFile + "'";
                throw TemplateException(msg, false);
            }
        }
    }
    else
    {
        it = varValue.FindMember("members");
        if (it != varValue.MemberEnd())
        {
            std::shared_ptr<MemberSetVariable> pMemVar = std::dynamic_pointer_cast<MemberSetVariable>(pVariable);
            if (pMemVar)
            {
                auto memberObj = it->value.GetObject();
                std::vector<std::string> memberNames;

                //
                // Get names
                auto namesArray = memberObj.FindMember("names")->value.GetArray();  // required by schema
                for (auto &memberNameIt: namesArray)
                {
                    memberNames.emplace_back(memberNameIt.GetString());
                    pMemVar->addMemberName(memberNames.back());
                }

                //
                // Values
                it = memberObj.FindMember("values");
                if (it != memberObj.MemberEnd())
                {
                    auto valueSetsArray = it->value.GetArray();
                    for (auto &valueSetIt: valueSetsArray)
                    {
                        unsigned idx = 0;
                        auto values = valueSetIt.GetArray();
                        for (auto &value: values)
                        {
                            pMemVar->addMemberValue(value.GetString(), memberNames[idx++]);
                        }
                    }
                }
            }
            else
            {
                std::string msg = "Variable '" + varName + "' not allowed to use 'members' key in template file " + m_templateFile;
                throw TemplateException(msg, false);
            }
        }
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
    else
    {
        isGlobal = m_isRoot;
    }

    if (!alreadyAdded)
    {
        m_pVariables->add(pVariable, isGlobal);
    }
}


std::shared_ptr<Variable> EnvModTemplate::getVariable(const std::string &name, bool throwIfNotFound) const
{
    return m_pVariables->getVariable(name, throwIfNotFound);
}


std::vector<std::shared_ptr<Variable>> EnvModTemplate::getInputs() const
{
    std::vector<std::shared_ptr<Variable>> returnVars;
    std::shared_ptr<Variables> pGlobalVars = m_pVariables->getGlobalVariables();
    const std::vector<std::shared_ptr<Variable>> &variables = pGlobalVars->getAllVariables();
    for (auto const &pVariable: variables)
    {
        if (pVariable->isUserInput())
        {
            returnVars.push_back(pVariable);
        }
    }
    return returnVars;
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
        throw(TemplateException(&inputJson, filepath));
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
    std::shared_ptr<Operation> pOperation;
    std::string action(operation.FindMember("action")->value.GetString());

    if (action == "include")
    {
        std::shared_ptr<OperationIncludeTemplate> pIncOp = std::make_shared<OperationIncludeTemplate>();
        auto dataIt = operation.FindMember("data");
        if (dataIt != operation.MemberEnd())
        {
            parseIncludeOperation(dataIt->value, pIncOp);  // schema requires data object
        }

        for (auto &path: pIncOp->m_paths)
        {
            std::vector<std::string> files;
            if (pIncOp->m_isPath)
            {
                getFilelist(path, files);
            }
            else
            {
                files.emplace_back(path);
            }

            for (auto const &file: files)
            {
                auto pEnvModTemplate = std::make_shared<EnvModTemplate>(*this);
                pEnvModTemplate->m_ignoreEmptyTemplate = !pIncOp->m_errorIfNotFound;
                pEnvModTemplate->loadTemplateFromFile(file);
                pIncOp->m_envModTemplates.emplace_back(pEnvModTemplate);
            }
        }
        parseCondition(operation, pIncOp);
        m_operations.emplace_back(pIncOp);
        pOperation = pIncOp;
    }
    else if (action == "copy")
    {
        std::shared_ptr<OperationCopy> pCopyOp = std::make_shared<OperationCopy>();
        auto dataIt = operation.FindMember("data");
        if (dataIt != operation.MemberEnd())
        {
            parseCopyOperation(dataIt->value, pCopyOp);
        }
        parseCondition(operation, pCopyOp);
        m_operations.emplace_back(pCopyOp);
        pOperation = pCopyOp;
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
        else if (action == "insert_raw")
        {
            pOp = std::make_shared<OperationInsertRaw>();
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
                }
                else if (action == "insert_raw")
                {
                    std::shared_ptr<OperationInsertRaw> pOpInsert = std::dynamic_pointer_cast<OperationInsertRaw>(pOp);
                    auto it = dataIt->value.FindMember("raw_data");
                    if (it != dataIt->value.MemberEnd())
                    {
                        pOpInsert->m_rawData = it->value.GetString();
                    }

                    pOpInsert->m_nodeType = dataIt->value.FindMember("node_type")->value.GetString();
                }
                else if (action == "noop")
                {
                    return;
                }
            }
            parseCondition(operation, pOp);
            m_operations.emplace_back(pOp);
            pOperation = pOp;
        }
    }

    auto breakIt = operation.FindMember("break");
    if (breakIt != operation.MemberEnd())
    {
        pOperation->m_breakExecution = breakIt->value.GetBool();
    }
}


void EnvModTemplate::parseOperationNodeCommonData(const rapidjson::Value &operationData, const std::shared_ptr<OperationNode>& pOpNode)
{
    rapidjson::Value::ConstMemberIterator it;
    auto dataObj = operationData.GetObject();


    it = dataObj.FindMember("target");
    if (it != dataObj.MemberEnd())
    {
        parseTarget(it->value, pOpNode->m_target);
    }
    else
    {
        std::shared_ptr<OperationCopy> pCopyOp = std::dynamic_pointer_cast<OperationCopy>(pOpNode);
        if (pCopyOp)
        {
            it = dataObj.FindMember("source");
            if (it != dataObj.MemberEnd())
            {
                parseTarget(it->value, pOpNode->m_target);
            }
        }
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
        parseSaveInfo(saveInfoIt->value, pOpNode->m_saveNodeIdName, pOpNode->m_accumulateSaveNodeIdOk, pOpNode->m_saveNodeIdAsGlobalValue, pOpNode->m_saveNodeIdClear);
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
        parseSaveInfo(saveInfoIt->value, pAttribute->saveVariableName, pAttribute->accumulateValuesOk, pAttribute->saveValueGlobal, pAttribute->clear);
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

    attrValueIt = attributeData.FindMember("optional");
    if (attrValueIt != attributeData.MemberEnd())
    {
        pAttribute->optional = attrValueIt->value.GetBool();
    }
}


void EnvModTemplate::parseEnvironment(const rapidjson::Value &environmentValue)
{
    auto targetData = environmentValue.GetObject();

    //
    // name is required at a minimum. Note that is may be a variable. Normally a name by itself appear with
    // set_as_target, that way the template can reference a previously defined environment, even using a variable
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
        // instantiated and saved at runtime using the name from above (which may be based on a variable value)
        m_pLocalEnvironment = std::make_shared<Environment>(masterSchemaFile, paths);

        //
        // Load sets the name of an environment to load on use
        valueIt = targetData.FindMember("load_filename");
        if (valueIt != targetData.MemberEnd())
        {
            m_pLocalEnvironment->setLoadName(valueIt->value.GetString());
        }

        //
        // Initialize creates an empty environment with required nodes
        valueIt = targetData.FindMember("initialize");
        if (valueIt != targetData.MemberEnd())
        {
            m_pLocalEnvironment->setInitializeEmpty(valueIt->value.GetBool());
        }

        //
        // Write sets the name of an environment file to write when execution of the template completes
        valueIt = targetData.FindMember("write_filename");
        if (valueIt != targetData.MemberEnd())
        {
            m_pLocalEnvironment->setOutputName(valueIt->value.GetString());
        }
    }

    //
    // Set as target marks this environment as the default environment for the template
    valueIt = targetData.FindMember("set_as_target");
    if (valueIt != targetData.MemberEnd())
    {
        m_useLocalEnvironmentForTemplate = valueIt->value.GetBool();
    }
}


void EnvModTemplate::parseTarget(const rapidjson::Value &targetValue, Target &target)
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
        target.setTargetPath(trim(valueIt->value.GetString()));
    }

    //
    // Node ID ?
    valueIt = targetData.FindMember("nodeid");
    if (valueIt != targetData.MemberEnd())
    {
        target.setTargetNodeId(valueIt->value.GetString());
    }

    //
    // Is there an environment override?
    valueIt = targetData.FindMember("configuration_name");
    if (valueIt != targetData.MemberEnd())
    {
        target.setEnvironmentName(valueIt->value.GetString());
    }
}


void EnvModTemplate::parseIncludeOperation(const rapidjson::Value &include, const std::shared_ptr<OperationIncludeTemplate> &pOpInc)
{
    auto includeObj = include.GetObject();

    //
    // Get the include template files or directories (one is required)
    auto it = includeObj.FindMember("template_files");
    if (it == includeObj.MemberEnd())
    {
        it = includeObj.FindMember("template_directories");
        pOpInc->m_isPath = true;
    }

    for (auto &pathIt: it->value.GetArray())
    {
        pOpInc->m_paths.emplace_back(pathIt.GetString());
    }

    //
    // Template parameters to build when executing the template
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
            valueIt = parmObject.FindMember("error_if_empty");
            if (valueIt != parmObject.MemberEnd())
            {
                parmValue.m_errorIfEmpty = valueIt->value.GetBool();
            }
            pOpInc->addParameterValue(parmValue);
        }
    }

    it = includeObj.FindMember("count");
    if (it != includeObj.MemberEnd())
    {
        pOpInc->m_count = trim(it->value.GetString());
    }

    //
    // Get the starting index (optional, for windowing into a range of values)
    it = includeObj.FindMember("start_index");
    if (it != includeObj.MemberEnd())
    {
        pOpInc->m_startIndex = trim(it->value.GetString());
    }

    // Is an environment specified?
    it = includeObj.FindMember("configuration");
    if (it != includeObj.MemberEnd())
    {
        pOpInc->m_environmentName = trim(it->value.GetString());
    }

    it = includeObj.FindMember("error_if_not_found");
    if (it != includeObj.MemberEnd())
    {
        pOpInc->m_errorIfNotFound = it->value.GetBool();
    }
}


void EnvModTemplate::parseCopyOperation(const rapidjson::Value &data, const std::shared_ptr<OperationCopy> &pCopyOp)
{
    auto dataObj = data.GetObject();

    //
    // Parse the common stuff
    parseOperationNodeCommonData(data, pCopyOp);

    //
    // Parse copy operation unique stuff
    auto valueIt = dataObj.FindMember("include_children");
    if (valueIt != dataObj.MemberEnd())
    {
        pCopyOp->m_includeChildren = valueIt->value.GetBool();
    }

    //
    // Strict mode?
    valueIt = dataObj.FindMember("strict");
    if (valueIt != dataObj.MemberEnd())
    {
        pCopyOp->m_strictCopyMode = valueIt->value.GetBool();
    }

    //
    // Get the copy type
    valueIt = dataObj.FindMember("copy_type");
    if (valueIt != dataObj.MemberEnd())
    {
        pCopyOp->m_copyType = valueIt->value.GetString();
    }

    //
    // Parse destination object
    auto destObj = dataObj.FindMember("destination")->value.GetObject();

    parseTarget(destObj.FindMember("target")->value, pCopyOp->m_destTarget);

    //
    // Set the throw on destination empty flag
    valueIt = destObj.FindMember("error_if_not_found");
    if (valueIt != destObj.MemberEnd())
    {
        pCopyOp->m_throwIfDestEmpty = valueIt->value.GetBool();
    }

    //
    // Save the destination Node IDs?
    valueIt = destObj.FindMember("save");
    if (valueIt != destObj.MemberEnd())
    {
        parseSaveInfo(valueIt->value, pCopyOp->m_destSaveNodeIdName, pCopyOp->m_destSaveNodeIdAccumulateOk,
                pCopyOp->m_destSaveNodeIdAsGlobalValue, pCopyOp->m_destSaveNodeIdClear);
    }

    //
    // Parse the copy attributes
    valueIt = destObj.FindMember("copy_attributes");
    if (valueIt != destObj.MemberEnd())
    {
        auto copyAttrObj = valueIt->value.GetObject();

        valueIt = copyAttrObj.FindMember("select");
        if (valueIt != copyAttrObj.MemberEnd())
        {
            pCopyOp->m_copyAttributeType = valueIt->value.GetString();
        }

        valueIt = copyAttrObj.FindMember("attributes");
        if (valueIt != copyAttrObj.MemberEnd())
        {
            for (auto &attrName: valueIt->value.GetArray())
            {
                pCopyOp->addCopyAttribute(attrName.GetString());
            }
        }
    }

    //
    // See if any attribute values need to be saved
    valueIt = destObj.FindMember("save_attributes");
    if (valueIt != destObj.MemberEnd())
    {
        std::string attrName, attrValue;
        for (auto &attr: valueIt->value.GetArray())
        {
            attrName = attr.FindMember("name")->value.GetString();
            std::string varName;
            bool accumulateOk = false;
            bool global = false;
            bool clear = false;
            parseSaveInfo(attr.FindMember("save")->value, varName, accumulateOk, global, clear);
            pCopyOp->addSaveAttributeValue(attrName, varName, global, accumulateOk, clear);
        }
    }

    //
    // Destination node type
    valueIt = destObj.FindMember("node_type");
    if (valueIt != destObj.MemberEnd())
    {
        pCopyOp->m_destNodeType = valueIt->value.GetString();
    }
}


void EnvModTemplate::parseSaveInfo(const rapidjson::Value &saveInfo, std::string &varName, bool &accumulateOk, bool &global, bool &clear)
{
    auto saveInfoObj = saveInfo.GetObject();

    varName = saveInfoObj.FindMember("variable_name")->value.GetString();

    // default values
    accumulateOk = false;
    global = false;
    clear = false;

    auto it = saveInfoObj.FindMember("accumulate");
    if (it != saveInfoObj.MemberEnd())
    {
        accumulateOk = it->value.GetBool();
    }

    it = saveInfoObj.FindMember("global");
    if (it != saveInfoObj.MemberEnd())
    {
        global = it->value.GetBool();
    }

    it = saveInfoObj.FindMember("clear_first");
    if (it != saveInfoObj.MemberEnd())
    {
        clear = it->value.GetBool();
    }
}


void EnvModTemplate::parseCondition(const rapidjson::Value &operation, const std::shared_ptr<Operation> &pOp)
{
    auto it = operation.FindMember("condition");
    if (it != operation.MemberEnd())
    {
        std::shared_ptr<Condition> pCondition = std::make_shared<Condition>();
        auto conditionObject = it->value.GetObject();

        //
        // Variable name for the conditional test
        pCondition->m_varName = conditionObject.FindMember("variable")->value.GetString();

        //
        // Type of test
        pCondition->m_type = conditionObject.FindMember("type")->value.GetString();

        //
        // Any values?
        auto valueIt = conditionObject.FindMember("values");
        if (valueIt != conditionObject.MemberEnd())
        {
            for (auto &val: valueIt->value.GetArray())
            {
                pCondition->m_values.emplace_back(val.GetString());
            }
        }

        pOp->m_pCondition = pCondition;
    }
}


void EnvModTemplate::execute(bool isFirst, const std::vector<ParameterValue> &parameters)
{
    if (!m_operations.empty())
    {
        std::shared_ptr<Environment> pTemplateEnv;
        //
        // Set the environment to use. If default is set, then it was passed in and overrides any environment specified in
        // the template. If no default environment was set, use what was defined by the template. If no environment defined
        // by the template, then raise an excpetion
        if (m_pDefaultEnv)
        {
            pTemplateEnv = m_pDefaultEnv;
        }
        else if (m_pLocalEnvironment && m_useLocalEnvironmentForTemplate)
        {
            pTemplateEnv = m_pLocalEnvironment;
        }
        else
        {
            throw TemplateExecutionException("Unable to determine environment for template", "setup", m_templateFile);
        }

        //
        // Initialize the environment
        try
        {
            pTemplateEnv->initialize();  // note that this may already have been done and will be a noop.
        }
        catch (TemplateExecutionException &te)
        {
            te.setContext("Variable preparation", m_templateFile);
            throw;
        }

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
            // If there is an environment defined for the template, and it's not the default, initialize it and make
            // it available by name for use elsewhere
            if (m_pLocalEnvironment && !m_useLocalEnvironmentForTemplate)
            {
                m_pLocalEnvironment->initialize();
                m_pEnvironments->add(m_pLocalEnvironment, m_pVariables->doValueSubstitution(m_environmentName));
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
                pOp->execute(m_pEnvironments, pTemplateEnv, m_pVariables);
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
        // If this is a root teamplate, save environments
        if (m_isRoot)
        {
            m_pEnvironments->save(m_pVariables);
            if (m_pDefaultEnv)
            {
                m_pDefaultEnv->save(m_pVariables);
            }
        }
    }
}


void EnvModTemplate::getFilelist(const std::string &path, std::vector<std::string> &filepaths) const
{
    Owned<IFile> pDir = createIFile(path.c_str());
    if (pDir->exists())
    {
        Owned<IDirectoryIterator> it = pDir->directoryFiles(nullptr, false, false);
        ForEach(*it)
        {
            StringBuffer fname;
            std::string filename = it->getName(fname).str();

            std::string fullyQualifiedFilePath = path;
            if (std::string(1, fullyQualifiedFilePath.back()) != PATHSEPSTR)
                fullyQualifiedFilePath += PATHSEPSTR;
            fullyQualifiedFilePath += filename;
            filepaths.emplace_back(fullyQualifiedFilePath);
        }
    }
}


void EnvModTemplate::validateEnvironments(std::map<std::string, Status> &envStatus) const
{
    m_pEnvironments->validate(envStatus);
}


void EnvModTemplate::saveEnvironments() const
{
    m_pEnvironments->save(m_pVariables);
}
