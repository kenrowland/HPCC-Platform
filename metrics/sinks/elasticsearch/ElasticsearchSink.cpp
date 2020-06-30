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
#include "Measurement.hpp"
#include "MetricSet.hpp"
#include <map>
#include <algorithm>
#include "jstring.hpp"


using namespace hpcc_metrics;

extern "C" MetricSink* getMetricSinkInstance(const std::map<std::string, std::string> &parms)
{
    MetricSink *pSink = new ElasticsearchSink(parms);
    return pSink;
}



ElasticsearchSink::ElasticsearchSink(const std::map<std::string, std::string> &parms) :
    MetricSink{parms}
{
    //
    // Extract and store index name templates mapped by setName to template
    // Any metricset name w/o a template will simply use the metric set name as the index name

    //
    //  metricsetname -> index template
    //    create a metricsetinfo for each listed



    // Ask how data is to be passed to the component

    protocol.append("http");
    port.append("9200");
    domain.append("localhost");

}


void ElasticsearchSink::send(const std::vector<std::shared_ptr<MeasurementBase>> &values, const std::string &setName)
{
    //
    // Lookup setName in the index map.
    // If present
    //    Generate index name
    //    If different than last index name used (means we need to store the index name used)
    //       Create new index
    //

    auto it = setInfo.find(setName);
    if (it != setInfo.end())
    {
        initializeIndex(&it->second);

        //
        // Build the payload
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
        url.append("/").append(it->second.lastIndexName.c_str());
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

void ElasticsearchSink::init(const std::vector<std::shared_ptr<MetricSet>> &metricSets)
{

    //
    // Loop through the metric sets and for any that don't have information saved, create a new
    // metric info entry and set the index template to the set name as a default
    for (auto const &pMetricSet : metricSets)
    {
        MetricSetInfo *pMi;
        auto findIt = setInfo.find(pMetricSet->getName());  // probably don't need to do this since it only protects against duplicate metric set names
        if (findIt == setInfo.end())
        {
            auto it = setInfo.insert({pMetricSet->getName(), MetricSetInfo()});
            pMi = &(it.first->second);
            pMi->indexTemplate = pMetricSet->getName();
            pMi->setName = pMetricSet->getName();

            //
            // Now add any type info
            auto metrics = pMetricSet->getMetrics();
            for (auto const &pMetric : metrics)
            {
                std::string type = pMetric->getType();
                if (!type.empty())
                {
                    pMi->metricValueTypes[pMetric->getName()] = pMetric->getType();
                }
            }
        }

    }


    //initializeIndex("mytestindex");


//    Owned<IHttpClientContext> httpctx = getHttpClientContext();
//    Owned<IHttpClient> httpClient = httpctx->createHttpClient(nullptr, "http://localhost:9200");
//
//    StringBuffer resp, status, req;
//    req.append("/_search");
//    int ret = httpClient->sendRequest("GET", "application/json", req, resp, status);




//    struct addrinfo* result;
//    struct addrinfo *rp;
//    int sfd;
//    int error = getaddrinfo("localhost", "9200", nullptr, &result);
//
//    for (rp = result; rp != nullptr; rp = rp->ai_next) {
//        sfd = socket(rp->ai_family, rp->ai_socktype,
//                     rp->ai_protocol);
//        if (sfd == -1)
//            continue;
//
//        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
//            break;                  /* Success */
//
//        close(sfd);
//    }
//
//    char buf[2056];
//    int byte_count, len;
//    char header[256] = "GET /_search HTTP/1.1\nHost: localhost:9200\nAccept: */*\n";
//    len = strlen(header) + 1;
//    write(sfd,header, len);
//    printf("GET Sent...\n");
//    //all right ! now that we're connected, we can receive some data!
//    byte_count = recv(sfd,buf,sizeof buf,0);
//    printf("recv()'d %d bytes of data in buf\n",byte_count);
//    printf("%s",buf);

}

//
// Creates the index if needed
bool ElasticsearchSink::initializeIndex(MetricSetInfo *pSetInfo)
{
    bool rc = false;
    std::string indexName = buildIndexName(pSetInfo->indexTemplate, pSetInfo->setName);

    //
    // If the index name matches the last used index name, then all is good.
    if (indexName == pSetInfo->lastIndexName)
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
            if (createNewIndex(pSetInfo, indexName))
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
        pSetInfo->lastIndexName = indexName;
    }
    return rc;
}


bool ElasticsearchSink::createNewIndex(MetricSetInfo *pSetInfo, const std::string &indexName)
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
    // Build any mappings
    if (!pSetInfo->metricValueTypes.empty())
    {
        content.append(R"({ "mappings": { "properties" : { )");
        auto fieldIt = pSetInfo->metricValueTypes.begin();
        bool done = false;
        while (!done)
        {
            content.append("\"").append(fieldIt->first.c_str()).append("\" : ");
            content.append(R"({ "type" : ")").append(fieldIt->second.c_str()).append("\"}");
            done = ++fieldIt == pSetInfo->metricValueTypes.end();
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
