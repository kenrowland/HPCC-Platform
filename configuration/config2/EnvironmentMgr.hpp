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

#ifndef _CONFIG2_ENVIRONMENTMGR_HPP_
#define _CONFIG2_ENVIRONMENTMGR_HPP_

#include <string>
#include <fstream>
#include "ConfigItem.hpp"
#include "EnvironmentNode.hpp"

class EnvironmentMgr
{
	public:

		struct valueDef {
			std::string name;
			std::string value;
		};

		EnvironmentMgr() : m_key(0) { }
		virtual ~EnvironmentMgr() { }

		void setConfig(const std::shared_ptr<ConfigItem> &pConfig) { m_pConfig = pConfig; }
		// add a load from stream?
		virtual bool loadEnvironment(const std::string &file);  // return some error code,or a get last error type of call?

		std::shared_ptr<EnvironmentNode> getNodeFromPath(const std::string &path) { return getElement(path); }
		bool setValuesForPath(const std::string &path, const std::vector<valueDef> &values, const std::string &nodeValue, bool force=false);
		
		// save to stream ?
		void saveEnvironment(const std::string &file);

		bool validate();


		// virtual bool writeEnvironment(const std::string &file /* same with load */) = 0;
		//virtual bool validate();

	protected:

		std::string getUniqueKey();
		std::shared_ptr<EnvironmentNode> getElement(const std::string &path);
		void addPath(const std::shared_ptr<EnvironmentNode> pNode);
		virtual bool load(std::istream &in) = 0;
		virtual void save(std::ostream &out) = 0;


	protected:


		std::shared_ptr<ConfigItem> m_pConfig;
		std::shared_ptr<EnvironmentNode> m_pRootNode;
		std::map<std::string, std::shared_ptr<EnvironmentNode>> m_paths;


	private:
		
		int m_key;
};

#endif