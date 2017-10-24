/*##############################################################################

HPCC SYSTEMS software Copyright (C) 2017 HPCC Systems�.

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

#include "EnvironmentMgr.hpp"
#include "ConfigExceptions.hpp"
#include "XMLEnvironmentMgr.hpp"



EnvironmentMgr *getEnvironmentMgrInstance(const std::string &envType, const std::string &configPath)
{
    EnvironmentMgr *pEnvMgr = NULL;
    if (envType == "XML")
    {
        //std::shared_ptr<ConfigParser> pCfgParser = std::make_shared<XSDConfigParser>(configPath, m_pConfig);
        pEnvMgr = new XMLEnvironmentMgr(configPath);
    }
    return pEnvMgr;
}


EnvironmentMgr::EnvironmentMgr(const std::string &configPath) :
    m_key(0), 
    m_configPath(configPath)
{
    m_pConfig = std::make_shared<ConfigItem>("root");  // make the root
}


bool EnvironmentMgr::loadConfig(const std::vector<std::string> &cfgParms)
{
    createParser(cfgParms);
    m_pConfigParser->parseEnvironmentConfig(cfgParms);
    m_pConfig->postProcessConfig();
    return true;
}


bool EnvironmentMgr::loadEnvironment(const std::string &filename)
{
	std::ifstream in;
    std::string fpath = m_configPath + filename;
	
	in.open(fpath);
	if (in.is_open())
	{
		load(in);
	}
	return true;
}


Status EnvironmentMgr::saveEnvironment(const std::string &filename)
{
    Status status;
	std::ofstream out;

	out.open(filename);
	if (out.is_open())
	{
		save(out);
	}
    return status;
}


void EnvironmentMgr::addPath(const std::shared_ptr<EnvironmentNode> pNode)
{
	auto retVal = m_nodeIds.insert({pNode->getId(), pNode });
	if (!retVal.second)
	{
		throw (new ParseException("Attempted to insert duplicate path name " + pNode->getId() + " for node "));
	}
}


std::shared_ptr<EnvironmentNode> EnvironmentMgr::getEnvironmentNode(const std::string &nodeId)
{
	std::shared_ptr<EnvironmentNode> pNode;
	auto pathIt = m_nodeIds.find(nodeId);
	if (pathIt != m_nodeIds.end())
        pNode = pathIt->second;
	return pNode;
}


// todo: make a standard return that has a status string and array of messages
Status EnvironmentMgr::setAttributeValues(const std::string &nodeId, const std::vector<valueDef> &values, const std::string &nodeValue, bool force)
{
    Status status;
	std::shared_ptr<EnvironmentNode> pEnvNode = getEnvironmentNode(nodeId);
	if (pEnvNode)
	{
		for (auto it = values.begin(); it != values.end(); ++it)
			pEnvNode->setAttributeValue((*it).name, (*it).value, status, force);
	}
    else
    {
        status.addStatusMsg(error, nodeId, "", "", "Indicated node ID does not exist");
    }
	return status;
}


std::string EnvironmentMgr::getUniqueKey()
{
	return std::to_string(m_key++);
}


Status EnvironmentMgr::validate() const
{
    Status status;
	if (m_pRootNode)
	{
        m_pRootNode->validate(status);
	}
	return status;
}