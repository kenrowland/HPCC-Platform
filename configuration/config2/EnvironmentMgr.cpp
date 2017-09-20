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


void EnvironmentMgr::saveEnvironment(const std::string &filename)
{
	std::ofstream out;

	out.open(filename);
	if (out.is_open())
	{
		save(out);
	}
}


void EnvironmentMgr::addPath(const std::shared_ptr<EnvironmentNode> pNode)
{
	auto retVal = m_paths.insert({pNode->getPath(), pNode });
	if (!retVal.second)
	{
		throw (new ParseException("Attempted to insert duplicate path name " + pNode->getPath() + " for node "));
	}
}


std::shared_ptr<EnvironmentNode> EnvironmentMgr::getElement(const std::string &path)
{
	auto pathIt = m_paths.find(path);
	//if (pathIt == m_paths.end())
	//	return;

	return pathIt->second;
}


//std::map<std::string, std::shared_ptr<EnvValue>> EnvironmentMgr::getAttributes(const std::string &path)
//{
//	auto pathIt = m_paths.find(path);
//	//if (pathIt == m_paths.end())
//	//	return;
//
//	std::shared_ptr<EnvironmentNode> pNode = pathIt->second;
//	return pNode->getAttributes();
//}


std::string EnvironmentMgr::getUniqueKey()
{
	return std::to_string(m_key++);
}