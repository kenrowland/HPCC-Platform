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

#ifndef _CONFIG2_ENUMERATION_HPP_
#define _CONFIG2_ENUMERATION_HPP_

#include <vector>

class XSDEnumerationBase 
{
    public:
        XSDEnumerationBase() { };
        virtual ~XSDEnumerationBase() { };
};


template<class T> class XSDEnumeration : public XSDEnumerationBase
{
    public:
        XSDEnumeration() { };
        ~XSDEnumeration() { };

        virtual void addItem(const T &item);

    private:
        std::vector<T> m_list;
};


template <class T> void  XSDEnumeration<T>::addItem(const T &item) 
{ 
    m_list.push_back(item); 
}



#endif // _CONFIG2_ENUMERATION_HPP_