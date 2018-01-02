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

#ifndef _CONFIG2_CONFIGITEMCOMPONENT_HPP_
#define _CONFIG2_CONFIGITEMCOMPONENT_HPP_

#include <memory>
#include <vector>
#include "SchemaValue.hpp"
#include "SchemaItem.hpp"


class SchemaItemComponent : public SchemaItem
{
    public:

        SchemaItemComponent(const std::string &name, std::shared_ptr<SchemaItem> pParent) : SchemaItem(name, "component", pParent) { }
        virtual ~SchemaItemComponent() { }
};

#endif // _CONFIG2_CONFIGITEMCOMPONENT_HPP_