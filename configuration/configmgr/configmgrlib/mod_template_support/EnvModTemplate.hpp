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

#ifndef HPCCSYSTEMS_PLATFORM_ENVMODTEMPLATE_HPP
#define HPCCSYSTEMS_PLATFORM_ENVMODTEMPLATE_HPP

#include "rapidjson/document.h"
#include "rapidjson/schema.h"
#include "rapidjson/istreamwrapper.h"
#include "TemplateException.hpp"
#include <string>
#include "Variable.hpp"
#include "Variables.hpp"
#include "Environments.hpp"
#include "ParameterValue.hpp"
#include "OperationNode.hpp"
#include "OperationFindNode.hpp"
#include "OperationIncludeTemplate.hpp"
#include <map>
#include <vector>
#include <memory>
#include <map>
#include "platform.h"
#include "Cfgmgrlib.hpp"

class EnvironmentMgr;

class CFGMGRLIB_API EnvModTemplate
{
    public:

        EnvModTemplate(std::shared_ptr<EnvironmentMgr> pEnvMgr, const std::string &schemaFile);
        EnvModTemplate(const EnvModTemplate &modTemplate);
        ~EnvModTemplate();

        void loadTemplateFromJson(const std::string &templateJson);
        void loadTemplateFromFile(const std::string &fqTemplateFile);
        std::string getTemplateFilename() const { return m_templateFile; }
        std::shared_ptr<Variable> getVariable(const std::string &name, bool throwIfNotFound = true) const;
        std::vector<std::shared_ptr<Variable>> getVariables(bool userInputOnly = false) const;
        void assignVariablesFromFile(const std::string &filepath);
        void execute(bool isFirst, const std::vector<ParameterValue> &parameters = std::vector<ParameterValue>());
        void setTargetEnvironment(const std::string &name) { if (m_environmentName.empty()) m_environmentName = name; }


    protected:

        void releaseTemplate();
        void loadTemplate(rapidjson::IStreamWrapper &stream);
        void parseTemplate();
        void parseCommon();
        void parseVariables(const rapidjson::Value &variables);
        void parseVariable(const rapidjson::Value &varValue);
        void parseOperations(const rapidjson::Value &operations);
        void parseOperation(const rapidjson::Value &operation);
        void parseOperationNodeCommonData(const rapidjson::Value &operationData, std::shared_ptr<OperationNode> pOpNode);
        void parseAttribute(const rapidjson::Value &attributeData, modAttribute *pAttribute);
        void parseTarget(const rapidjson::Value &targetData, std::shared_ptr<OperationNode> pOp);
        void parseIncludeOperation(const rapidjson::Value &include, std::shared_ptr<OperationIncludeTemplate> pOpInc);


    protected:

        std::shared_ptr<rapidjson::SchemaDocument> m_pSchema;
        rapidjson::Document *m_pTemplate;   // same as GenericDocument<UTF8<> >
        std::shared_ptr<Variables> m_pVariables;
        std::shared_ptr<Environments> m_pEnvironments;
        std::vector<std::shared_ptr<Operation>> m_operations;
        std::string m_templateFile;
        std::string m_environmentName;
};


#endif //HPCCSYSTEMS_PLATFORM_ENVMODTEMPLATE_HPP
