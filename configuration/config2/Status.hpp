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

#ifndef _CONFIG2_NODESTATUS_HPP_
#define _CONFIG2_NODESTATUS_HPP_

#include <map>
#include <vector>
#include <string>

enum msgLevel
{
    ok = 0,     // informational messages mainly
    warning,
    error,
    fatal
};

struct statusMsg {
    statusMsg(msgLevel _msgLevel, const std::string &_nodeId, const std::string &_name, const std::string &_referNodeId, const std::string &_msg) :
        msgLevel(_msgLevel), nodeId(_nodeId), attribute(_name), referNodeId(_referNodeId), msg(_msg) { }
    msgLevel msgLevel;           // Message level
    std::string nodeId;          // if not '', the node ID to which this status applies
    std::string attribute;       // possible name of attribute in nodeId
    std::string referNodeId;     // refernece node to which this status may also apply
    std::string msg;             // message for user
};

class Status
{
	public:
		
		Status() : m_highestMsgLevel(ok) { }
		~Status() {}
		void addStatusMsg(msgLevel status, const std::string &nodeId, const std::string &name, const std::string &referNodeId, const std::string &msg);
        msgLevel getHighestMsgLevel() const { return m_highestMsgLevel; }
        bool isOk() const { return m_highestMsgLevel == ok; }
		std::string getStatusTypeString(msgLevel status) const;
        std::vector<statusMsg> getMessages() const;


	private:

		msgLevel m_highestMsgLevel;
		std::multimap<msgLevel, statusMsg> m_messages;
};

#endif