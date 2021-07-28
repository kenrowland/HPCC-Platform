/*##############################################################################
    HPCC SYSTEMS software Copyright (C) 2021 HPCC Systems®.
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


#include "elasticsearchSink.hpp"

#include <cstdio>
#include "platform.h"

using namespace hpccMetrics;

extern "C" MetricSink* getSinkInstance(const char *name, const IPropertyTree *pSettingsTree)
{
    MetricSink *pSink = new ElasticsearchSink(name, pSettingsTree);
    return pSink;
}

ElasticsearchSink::ElasticsearchSink(const char *name, const IPropertyTree *pSettingsTree) :
        PeriodicMetricSink(name, "elasticsearch", pSettingsTree)
{
    //pSettingsTree->getProp("@filename", fileName);
    //clearFileOnStartCollecting = pSettingsTree->getPropBool("@clear", false);
}


void ElasticsearchSink::doCollection()
{

}


std::string ElasticsearchSink::getIndexName(const std::string &nameTemplate, const std::string &suffix)
{
    time_t rawtime;
    struct tm *timeinfo;
    time (&rawtime);
    timeinfo = gmtime (&rawtime);
    std::string indexName;
    size_t startPos = 0;
    size_t pos = nameTemplate.find_first_of('%', startPos);
    while (pos != std::string::npos)
    {
        indexName.append(nameTemplate.substr(startPos, pos - startPos));
        if (pos < nameTemplate.length())
        {
            char elem = nameTemplate[pos+1];
            switch (elem)
            {
                // date YYYY-MM-DD
                case 'd':
                    char buff[64];
                    sprintf(buff, "%4d", timeinfo->tm_year + 1900);
                    indexName.append(buff).append("-");
                    sprintf(buff, "%2d", timeinfo->tm_mon + 1);
                    indexName.append(buff).append("-");
                    sprintf(buff, "%2d", timeinfo->tm_mday);
                    indexName.append(buff).append("-");
                    startPos = pos + 2;
                    break;

                case 'n':
                    indexName.append(suffix);
                    startPos = pos + 2;
                    break;

                default:
                    indexName.append(nameTemplate.substr(pos, 2));
                    startPos = pos + 2;
                    break;
            }
        }
        else
        {
            startPos = pos + 1;
        }
        pos = nameTemplate.find_first_of('%', startPos);
    }
    indexName.append(nameTemplate.substr(startPos));
    transform(indexName.begin(), indexName.end(), indexName.begin(), ::tolower);
    return indexName;
}
