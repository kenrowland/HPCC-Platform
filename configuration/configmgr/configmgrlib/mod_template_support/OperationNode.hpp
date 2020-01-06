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

#ifndef HPCCSYSTEMS_PLATFORM_OPERATIONNODE_HPP
#define HPCCSYSTEMS_PLATFORM_OPERATIONNODE_HPP

#include "Variables.hpp"
#include "Operation.hpp"
#include "Target.hpp"
#include <string>
#include <vector>


struct modAttribute {
    modAttribute() : accumulateValuesOk(false), doNotSet(false), optional(false),
            errorIfNotFound(false), errorIfEmpty(false), saveValueGlobal(false), clear(false), notEqual(false) {}
    ~modAttribute() = default;
    void addName(const std::string &_name) { names.emplace_back(_name); }
    const std::string &getName(std::size_t idx=0) { return names[idx]; }
    std::size_t getNumNames() { return names.size(); }
    std::vector<std::string> names;  // attribute name. Vector is for first_of use when finding the first match.
    std::string value;
    std::string startIndex;
    std::string cookedValue;
    std::string saveVariableName;
    bool saveValueGlobal;
    bool doNotSet;
    bool accumulateValuesOk;
    bool clear;
    bool errorIfNotFound;
    bool errorIfEmpty;
    bool optional;
    bool notEqual;
};


class EnvironmentMgr;
class EnvironmentNode;

class OperationNode : public Operation
{

    public:

        OperationNode() = default;
        bool execute(std::shared_ptr<Environments> pEnvironments, std::shared_ptr<Environment> pEnv, std::shared_ptr<Variables> pVariables) override;
        void addAttribute(modAttribute &newAttribute);
        void assignAttributeCookedValues(const std::shared_ptr<Variables> &pVariables);


    protected:

        virtual bool preExecute(const std::shared_ptr<Variables> &pVariables);
        virtual void doExecute(std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables, const std::string &nodeId) = 0;

        std::shared_ptr<Variable> createVariable(const std::string& varName, const std::string &varType, const std::shared_ptr<Variables>& pVariables,
                bool accumulateOk, bool global, bool clear);
        bool createAttributeSaveInputs(const std::shared_ptr<Variables> &pVariables);
        void saveAttributeValues(const std::shared_ptr<Variables> &pVariables, const std::shared_ptr<EnvironmentNode> &pEnvNode);
        void processNodeValue(const std::shared_ptr<Variables> &pVariables, const std::shared_ptr<EnvironmentNode> &pEnvNode);
        void initializeForExecution(const std::shared_ptr<Environments> &pEnvironments, const std::shared_ptr<Environment>& pEnv,
                const std::shared_ptr<Variables> &pVariables);


    protected:

        Target m_target;
        //std::string m_environmentName;
        std::vector<std::string> m_parentNodeIds;
        std::vector<modAttribute> m_attributes;
        modAttribute m_nodeValue;
        bool m_nodeValueValid = false;
        bool m_throwOnEmpty = true;
        std::string m_saveNodeIdName;
        std::shared_ptr<Variable> m_pSaveNodeIdVar;
        bool m_accumulateSaveNodeIdOk = false;
        bool m_saveNodeIdAsGlobalValue = false;
        bool m_saveNodeIdClear = false;
        bool m_saveAttributeValues = false;
        std::shared_ptr<EnvironmentMgr> m_pOpEnvMgr;   // the environment mgr to use for operation execution


    friend class EnvModTemplate;
};


#endif //HPCCSYSTEMS_PLATFORM_OPERATIONNODE_HPP
