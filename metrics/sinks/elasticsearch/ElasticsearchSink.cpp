/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2020 HPCC Systems®.

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

#include "httpclient.hpp"
#include "ElasticsearchSink.hpp"
#include "IMeasurement.hpp"
#include "IMetricSet.hpp"
#include <map>
#include <algorithm>
#include "jstring.hpp"


using namespace hpccMetrics;

extern "C" MetricSink* getMetricSinkInstance(const std::map<std::string, std::string> &parms)
{
    MetricSink *pSink = new ElasticsearchSink(parms);
    return pSink;
}



ElasticsearchSink::ElasticsearchSink(const std::map<std::string, std::string> &parms) :
    MetricSink{parms}
{
    protocol.append("http");
    port.append("9200");
    domain.append("localhost");   // this will be a config parameter
}


void ElasticsearchSink::send(const std::vector<std::shared_ptr<IMeasurement>> &values, const std::string &setName)
{

    auto it = metricSets.find(setName);
    if (it != metricSets.end())
    {
        //
        // Do this every time in case the index name changes because of something like a day change.
        MetricSetInfo &curMetricSetInfo = metricSetInfo[setName];
        initializeIndex(it->second);

        //
        // Build the JSON payload directly
        std::string payload;
        payload.append("{");
        for (auto const &pMeas : values)
        {
            payload.append("\"").append(pMeas->getName()).append("\" : ");
            payload.append("\"").append(pMeas->toString()).append("\", ");
        }
        payload.erase(payload.find_last_of(','));  // take off the last ',';
        payload.append("}");

        StringBuffer content(payload.c_str());
        Owned<IHttpClientContext> httpctx = getHttpClientContext();
        StringBuffer url;
        url.append(protocol.c_str()).append("://").append(domain.c_str()).append(":").append(port.c_str());
        url.append("/").append(curMetricSetInfo.lastIndexName.c_str());
        url.append("/_doc/");
        Owned<IHttpClient> httpClient = httpctx->createHttpClient(nullptr, url);

        StringBuffer resp, status;
        int ret = httpClient->sendRequest("POST", "application/json", content, resp, status);
        if (ret == 0)
        {
            int statusCode = atoi(status.str());
        }
    }
    // else a throw here, or is it trusted
}


// Index pattern from filebeat:  index: "filebeat-%{[agent.version]}-%{+yyyy.MM.dd}"

void ElasticsearchSink::init()
{
    // nothing to do
}


//
// Creates the index if needed
bool ElasticsearchSink::initializeIndex(const std::shared_ptr<IMetricSet>& pMetricSet)
{
    bool rc = false;
    MetricSetInfo *pMetricSetInfo = &(metricSetInfo[pMetricSet->getName()]);
    std::string indexName = buildIndexName(pMetricSetInfo->indexTemplate, pMetricSet->getName());

    //
    // If the index name matches the last used index name, then all is good.
    if (indexName == pMetricSetInfo->lastIndexName)
    {
        return true;
    }

    //
    // If the requested index already exists, just set the name in the set info and the
    // sink is ready to start reporting. If it doesn't, create it
    Owned<IHttpClientContext> httpctx = getHttpClientContext();
    StringBuffer urlBase, url;
    urlBase.append(protocol.c_str()).append("://").append(domain.c_str()).append(":").append(port.c_str());
    url = urlBase;
    url.append("/").append(indexName.c_str());
    Owned<IHttpClient> httpClient = httpctx->createHttpClient(nullptr, url);

    StringBuffer resp, status, content;
    int ret = httpClient->sendRequest("GET", "application/json", content, resp, status);
    if (ret == 0)
    {
        int statusCode = atoi(status.str());

        //
        // If the index does not exist, create it
        if (statusCode == 404)
        {
            if (createNewIndex(pMetricSetInfo, pMetricSet, indexName))
            {
                rc = true;
            }
        }
        else
        {
            rc = true;
        }
    }

    if (rc)
    {
        pMetricSetInfo->lastIndexName = indexName;   // se we don't have to regenerate each time
    }
    return rc;
}


bool ElasticsearchSink::createNewIndex(MetricSetInfo *pSetInfo, const std::shared_ptr<IMetricSet>& pMetricSet, const std::string &indexName)
{
    bool rc = false;
    Owned<IHttpClientContext> httpctx = getHttpClientContext();
    StringBuffer urlBase, url;
    StringBuffer resp, status, content;
    urlBase.append(protocol.c_str()).append("://").append(domain.c_str()).append(":").append(port.c_str());
    url = urlBase;
    url.append("/").append(indexName.c_str());
    Owned<IHttpClient> httpClient = httpctx->createHttpClient(nullptr, url);

    //
    // Build mappings for typed metrics
    auto typedMetrics = pMetricSet->getTypedMetrics();
    if (!typedMetrics.empty())
    {
        content.append(R"({ "mappings": { "properties" : { )");
        auto typedMetric = typedMetrics.begin();
        bool done = false;
        while (!done)
        {
            content.append("\"").append(typedMetric->first.c_str()).append("\" : ");
            content.append(R"({ "type" : ")").append(typedMetric->second.c_str()).append("\"}");
            done = ++typedMetric == typedMetrics.end();
            if (!done)
            {
                content.append(",");
            }
        }
        content.append("} } }");
    }

    int ret = httpClient->sendRequest("PUT", "application/json", content, resp, status);
    if (ret == 0)
    {
        int statusCode = atoi(status.str());
        rc = statusCode == 200;
    }
    return rc;
}


std::string ElasticsearchSink::buildIndexName(const std::string &nameTemplate, const std::string &setName)
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
                    indexName.append(setName);
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


void ElasticsearchSink::addMetricSet(const std::shared_ptr<IMetricSet> &pSet)
{
    // add it to the base map
    MetricSink::addMetricSet(pSet);

    // Create a place holder in case more information is added
    metricSetInfo.insert({pSet->getName(), MetricSetInfo()});
}
