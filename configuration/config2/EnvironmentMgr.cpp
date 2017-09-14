/*##############################################################################

HPCC SYSTEMS software Copyright (C) 2015 HPCC Systems®.

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

bool EnvironmentMgr::loadEnvironment(const std::string &filename)
{
	std::ifstream in;
	
	in.open(filename);
	if (in.is_open())
	{
		load(in);
	}
	return true;
}


void EnvironmentMgr::addPath(const std::string &pathName, const std::shared_ptr<EnvironmentNode> pNode)
{
	auto retVal = m_paths.insert({pathName, pNode });
	if (!retVal.second)
	{
		throw (new ParseException("Attempted to insert duplicate path name " + pathName + " for node "));
	}
}


std::vector<std::shared_ptr<EnvironmentNode>> EnvironmentMgr::getElements(const std::string &path)
{
	auto pathIt = m_paths.find(path);
	//if (pathIt == m_paths.end())
	//	return;

	std::shared_ptr<EnvironmentNode> pNode = pathIt->second;
	return pNode->getElements();
}


std::map<std::string, std::shared_ptr<EnvValue>> EnvironmentMgr::getValues(const std::string &path)
{
	auto pathIt = m_paths.find(path);
	//if (pathIt == m_paths.end())
	//	return;

	std::shared_ptr<EnvironmentNode> pNode = pathIt->second;
	return pNode->getValues();
}


std::string EnvironmentMgr::getUniqueKey()
{
	return std::to_string(m_key++);
}