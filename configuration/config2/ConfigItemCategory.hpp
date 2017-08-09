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


#ifndef _CONFIG2_CONFIGITEMCATEGORY_HPP_
#define _CONFIG2_CONFIGITEMCATEGORY_HPP_

#include "CfgItem.hpp"


class ConfigItemCategory
{
    public:

        ConfigItemCategory(const std::string &name, std::shared<ConfigItem> &pParent) : ConfigItem(name, pParent, "category")  { };
        virtual ~ConfigItemCategory() { };


    protected:

        // some kind of category map parent/child Software->ESP->[components]
        ConfigItemCategory() { };


    private:

        
        

};


#endif // _CONFIG2_CONFIGITEMCATEGORY_HPP_
