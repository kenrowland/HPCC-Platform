/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2017 HPCC Systems®.

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


#ifndef _CONFIG2_ENVIRONMENTMGR_HPP_
#define _CONFIG2_ENVIRONMENTMGR_HPP_

#include <string>
#include <fstream>
#include <vector>
#include <atomic>
#include "SchemaItem.hpp"
#include "SchemaParser.hpp"
#include "EnvironmentNode.hpp"
#include "Status.hpp"
#include "NameValue.hpp"
#include "platform.h"
#include "EnvSupportLib.hpp"
#include "Cfgmgrlib.hpp"
#include "EnvironmentTypes.hpp"


//CFGMGRLIB_API EnvironmentMgr *getEnvironmentMgrInstance(const EnvironmentType envType);
CFGMGRLIB_API std::shared_ptr<EnvironmentMgr> getEnvironmentMgrInstance(const EnvironmentType envType);

class CFGMGRLIB_API EnvironmentMgr
{
    public:

        EnvironmentMgr();
        virtual ~EnvironmentMgr() {}
        //bool loadSchema(const std::string &configPath, const std::string &masterConfigFile, const std::map<std::string, std::string> &cfgParms = std::map<std::string, std::string>());
        bool loadSchema(const std::string &masterConfigFile, const std::vector<std::string> &configPaths, const std::map<std::string, std::string> &cfgParms = std::map<std::string, std::string>());
        std::string getLastSchemaMessage() const;
        std::string getLastEnvironmentMessage() const { return m_message;  }
        bool loadEnvironment(const std::string &qualifiedFilename);
        bool initializeEmptyEnvironment();
        std::shared_ptr<EnvironmentNode> findEnvironmentNodeById(const std::string &nodeId) const;
        std::shared_ptr<EnvironmentNode> getNewEnvironmentNode(const std::string &parentNodeId, const std::string &inputItem, Status &status) const;
        std::shared_ptr<EnvironmentNode> addNewEnvironmentNode(const std::string &parentNodeId, const std::string &configType,
                std::vector<NameValue> &initAttributes, Status &status, bool allowInvalid=false, bool forceCreate=false,
                bool createRequiredChildren = true);
        std::shared_ptr<EnvironmentNode> addNewEnvironmentNode(const std::shared_ptr<EnvironmentNode> &pParentNode, const std::string &nodeData,
                Status &status, const std::string &itemType);
        std::shared_ptr<EnvironmentNode> addNewEnvironmentNode(const std::shared_ptr<EnvironmentNode> &pParentNode, const std::shared_ptr<SchemaItem> &pSchemaItem,
                std::vector<NameValue> &initAttributes, Status &status, bool allowInvalid=false, bool forceCreate=false,
                bool createRequiredChildren = true);
        void insertRawEnvironmentData(const std::shared_ptr<EnvironmentNode> &pParentEnvNode, const std::string &itemType, const std::string &rawData);
        bool removeEnvironmentNode(const std::string &nodeId);
        virtual bool serialize(std::ostream &out, const std::shared_ptr<EnvironmentNode> &pStartNode);
        bool saveEnvironment(const std::string &qualifiedFilename);
        void discardEnvironment() { } //if (m_pRootNode) m_pRootNode = nullptr; if (!m_nodeIds.empty()) m_nodeIds.clear();}
        void validate(Status &status, bool includeHiddenNodes=false) const;
        std::string getRootNodeId() const;
        static std::string getUniqueKey();
        void fetchNodes(std::string path, std::vector<std::shared_ptr<EnvironmentNode>> &nodes, const std::shared_ptr<EnvironmentNode> &pStartNode = nullptr) const;
        bool isSchemaLoaded() const { return static_cast<bool>(m_pSchema); }
        bool isEnvironmentLoaded() const { return static_cast<bool>(m_pRootNode); }
        void enableValidationOnChange(bool enable) { m_enableValidationOnChange = enable; }


    protected:

        void addPath(std::shared_ptr<EnvironmentNode> pNode);
        virtual bool createParser() { return false; }
        virtual std::vector<std::shared_ptr<EnvironmentNode>> doLoadEnvironment(std::istream &in, const std::shared_ptr<SchemaItem> &pSchemaItem, const std::string &itemType) { return std::vector<std::shared_ptr<EnvironmentNode>>(); }
        virtual bool save(std::ostream &out) { return false; }
        void assignNodeIds(const std::shared_ptr<EnvironmentNode> &pNode);
        void insertExtraEnvironmentData(const std::shared_ptr<EnvironmentNode> &pParentNode);
        std::shared_ptr<SchemaItem> findInsertableItem(const std::shared_ptr<EnvironmentNode> &pNode, const std::string &itemType) const;
        void getPredefinedAttributeValues(const std::string &inputItemType, std::string &itemType, std::vector<NameValue> &initAttributes) const;
        void createInitNodes(const std::shared_ptr<EnvironmentNode> &pParentNode, const std::shared_ptr<SchemaItem> &pSchemaitem);


    protected:

        std::shared_ptr<SchemaItem> m_pSchema;
        std::shared_ptr<SchemaParser> m_pSchemaParser;
        std::shared_ptr<EnvironmentNode> m_pRootNode;
        std::map<std::string, std::shared_ptr<EnvironmentNode>> m_nodeIds;
        std::string m_message;
        std::vector<std::shared_ptr<EnvSupportLib>> m_supportLibs;
        bool m_enableValidationOnChange;


    private:

        static std::atomic_int m_key;
};

#endif
