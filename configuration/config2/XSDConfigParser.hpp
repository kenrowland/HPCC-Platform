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


#ifndef _CONFIG2_XSDCONFIGPARSER_HPP_
#define _CONFIG2_XSDCONFIGPARSER_HPP_

#include <string>
#include <memory>
#include <map>
#include <boost/property_tree/ptree.hpp>

#include "ConfigParser.hpp"

namespace pt = boost::property_tree;

class XSDConfigParser : public ConfigParser
{
    public:

        XSDConfigParser(const std::string &basePath) :  ConfigParser(basePath) { };
        virtual ~XSDConfigParser() { };
    


    protected:

        XSDConfigParser() { };
        virtual bool doParse(const std::string &envFilename) override;
        bool parseXSD(const std::string &filename, bool isComponent=false);
        bool parseComponent(const pt::ptree &componentTree);
        bool parseAttributeGroup(const pt::ptree &attributeTree);


    private:
    

};


#endif // _CONFIG2_XSDCONFIGPARSER_HPP_
