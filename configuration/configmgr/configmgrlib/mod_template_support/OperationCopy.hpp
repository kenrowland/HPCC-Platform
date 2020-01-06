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


#ifndef HPCCSYSTEMS_PLATFORM_OPERATIONCOPY_HPP
#define HPCCSYSTEMS_PLATFORM_OPERATIONCOPY_HPP

#include "OperationNode.hpp"
#include "EnvironmentNode.hpp"
#include <vector>
#include <map>


struct CopyAttributeInfo_t {
    std::string inputAttributeName;
    std::string outputAttributeName;
    std::string defaultValue;
    friend inline bool operator==(const CopyAttributeInfo_t &lhs, const std::string &str) { return lhs.inputAttributeName == str; }
};


struct SaveAttributeInfo_t {
    std::string attributeName;   // original name from source node
    std::string varName;         // variable name to which the attribute value is saved
    bool global;                 // the variable is to be a global variable
    bool accumulateOk;           // OK to accumulate values from multiple steps (means the variable may already exist)
    bool clear;                  // clear before use
    std::shared_ptr<Variable> pVar;  // variable where value(s) stored
};


class OperationCopy : public OperationNode
{
    public:

        OperationCopy() : m_copyAttributeType("all"), m_includeChildren(false), m_throwIfDestEmpty(true), m_strictCopyMode(false), m_copyType("many_to_one") {}
        bool execute(std::shared_ptr<Environments> pEnvironments, std::shared_ptr<Environment> pEnv, std::shared_ptr<Variables> pVariables) override ;


    protected:

        void addCopyAttribute(std::string attrStr);
        void addSaveAttributeValue(const std::string &attrName, const std::string &varName, bool global, bool accuulateOk, bool clear);
        void saveAttributeValues(const std::shared_ptr<EnvironmentNode> &pEnvNode);
        void getAttributeValues(const std::shared_ptr<EnvironmentNode> &pSourceNode, std::vector<NameValue> &initAttributes, const std::string &copyMpde);
        void doNodeCopy(const std::string &sourceNodId, const std::string &destParentNodeId, bool isChildNode);
        void doNodeCopy(const std::shared_ptr<EnvironmentNode> &pSourceEnvNode, const std::string &destParentNodeId, bool isChildNode);
        void doExecute(std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables, const std::string &nodeId) override {}


    protected:

        Target m_destTarget;
        std::string m_destNodeType;
        bool        m_throwIfDestEmpty;
        bool        m_strictCopyMode;
        std::string m_destSaveNodeIdName;
        bool m_destSaveNodeIdAccumulateOk = false;
        bool m_destSaveNodeIdAsGlobalValue = false;
        bool m_destSaveNodeIdClear = false;
        std::shared_ptr<Variable> m_pDestSaveNodeIdVar;
        std::string m_copyAttributeType;

        bool m_includeChildren;
        std::string m_copyType;
        std::vector<CopyAttributeInfo_t> m_copyAttributes;
        std::shared_ptr<EnvironmentMgr> m_pDestEnvMgr;
        std::map<std::string, SaveAttributeInfo_t> m_saveAttributes;

    friend class EnvModTemplate;
};


#endif //HPCCSYSTEMS_PLATFORM_OPERATIONCOPY_HPP
