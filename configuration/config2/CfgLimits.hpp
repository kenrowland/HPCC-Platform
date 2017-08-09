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

#ifndef _CONFIG2_CFGLIMITS_HPP_
#define _CONFIG2_CFGLIMITS_HPP_

#include <memory>
#include <vector>
#include <string>
//#include "CfgCollection.hpp"

// class CfgLimitsBase 
// {
//     public:

//         CfgLimitsBase() { };
//         virtual ~CfgLimitsBase() { };
//         virtual void addPattern(const std::string &pattern) { m_patterns.push_back(pattern); };
//         virtual bool checkPatternMatch(const std::string &value) const { return true; };


//     protected:

//         std::vector<std::string> m_patterns;
// };



class CfgLimits 
{
    public:

        
        CfgLimits() :
            m_minInclusive(0),
            m_maxInclusive(0),
            m_minExclusive(0),
            m_maxExclusive(0),
            m_length(0)
        {

        };
        virtual ~CfgLimits() { };
        //virtual void addItemToCollection(const T &item);
        virtual void setMinInclusive(int v)  { m_minInclusive = v; }; 
        virtual void setMinExclusive(int v)  { m_minExclusive = v; }; 
        virtual void setMaxInclusive(int v)  { m_maxInclusive = v; }; 
        virtual void setMaxExclusive(int v)  { m_maxExclusive = v; }; 
        virtual void setLength(int v)        { m_length       = v; };
        virtual void setMinLength(int v)     { m_minLength    = v; }; 
        virtual void setMaxLength(int v)     { m_maxLength    = v; };
        virtual void addPattern(const std::string &pattern) { m_patterns.push_back(pattern); }
        bool isValueValid(const std::string &testValue) { return true; };


    protected:

        int m_minInclusive;
        int m_maxInclusive;
        int m_minExclusive;
        int m_maxExclusive;
        int m_length;
        int m_minLength;
        int m_maxLength;
        std::vector<std::string> m_patterns;


    private:
    
        
        
        
};



#endif // _CONFIG2_CFGLIMITS_HPP_
