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

		EnvironmentMgr() { }
		virtual ~EnvironmentMgr() { }

		void setConfig(const std::shared_ptr<ConfigItem> &pConfig) { m_pConfig = pConfig; }
		virtual bool loadEnvironment(const std::string &file);  // return some error code,or a get last error type of call?
		// virtual bool writeEnvironment(const std::string &file /* same with load */) = 0;
		//virtual bool validate();

	protected:

		virtual bool load(std::istream &in) = 0;


	protected:


		std::shared_ptr<ConfigItem> m_pConfig;
		std::shared_ptr<EnvironmentNode> m_pEnvironment;
};

#endif