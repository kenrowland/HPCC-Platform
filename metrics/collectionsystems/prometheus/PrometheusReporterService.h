/*
 * PrometheusReporterService.h
 *
 *  Created on: Aug 17, 2020
 *      Author: ubuntu
 */

#ifndef SUBPROJECTS__HPCCSYSTEMS_PLATFORM_METRICS_PROMETHEUS_REPORTER_PROMETHEUSREPORTERSERVICE_H_
#define SUBPROJECTS__HPCCSYSTEMS_PLATFORM_METRICS_PROMETHEUS_REPORTER_PROMETHEUSREPORTERSERVICE_H_

#include "jliball.hpp"
#include "securesocket.hpp"
#include "http/platform/httptransport.ipp"

#include "jiface.hpp"
#include "IMetricsReportTrigger.hpp"
#include "MetricsReportTrigger.hpp"
#include "IMetric.hpp"
#include "IMetricSet.hpp"
#include "IMetricSink.hpp"
#include "MetricsReportContext.hpp"
#include "MetricsReportConfig.hpp"
#include "MetricSink.hpp"
#include "IMeasurement.hpp"

#include <thread>

#define COLLECTOR_NAME "prometheus"

using hpccMetrics::IMetricSet;
using hpccMetrics::MeasurementVector;
using hpccMetrics::MetricsReportConfig;
using hpccMetrics::MetricsReportTrigger;
using hpccMetrics::MetricSink;
using hpccMetrics::MetricsReportContext;
using hpccMetrics::MetricType;

class PrometheusMetricsReportTrigger;

class PrometheusMetricsReportContext : public MetricsReportContext,  public CInterface
{
public:
    IMPLEMENT_IINTERFACE

    PrometheusMetricsReportContext() {}

    virtual ~PrometheusMetricsReportContext() = default;

    const char * getMetrics() const
    {
        return metricsText.str();
    }

    void setMetrics(const char * metrics)
    {
        metricsText.set(metrics);
    }

private:
    StringBuffer metricsText;
};

class PrometheusReporterService :  public CInterface
{
private:
    static constexpr const char * HTTP_PAGE_TITLE = "HPCC Systems - Prometheus Metrics Service";
    static constexpr const char * DEFAULT_PROMETHEUS_METRICS_SERVICE_RESP_TYPE = "text/html; charset=UTF-8";
    static constexpr int          DEFAULT_PROMETHEUS_METRICS_SERVICE_PORT = 8767;
    static constexpr bool         DEFAULT_PROMETHEUS_METRICS_SERVICE_SSL = false;
    static constexpr const char * DEFAULT_PROMETHEUS_METRICS_SERVICE_NAME = "metrics";

    PrometheusMetricsReportTrigger * m_metricsTrigger;

    bool                            m_processing;
    int                             m_port;
    bool                            m_use_ssl;
    Owned<ISecureSocketContext>     m_ssctx;
    StringBuffer                    m_metricsServiceName;
    IPropertyTree *                 m_sslconfig;

    static bool onMetricsUnavailable(CHttpRequest * request, CHttpResponse * response);
    static bool onException(CHttpResponse * response, IException * e);
    static bool onException(CHttpResponse * response, const char * message);
    bool onMetrics(CHttpRequest * request, CHttpResponse * response);
    bool onPost(CHttpRequest * request,  CHttpResponse * response);
    bool onGet(CHttpRequest * request, CHttpResponse * response);
    void handleRequest(ISocket* client);

    bool setMetricsServiceName(const char * servicename)
    {
        if (servicename && *servicename)
        {
            m_metricsServiceName.set(servicename);
            return true;
        }
        else
            return false;
    }

public:
    IMPLEMENT_IINTERFACE

    static bool onGetNotFound(CHttpResponse * response, const char * path);
    static bool onPostNotFound(CHttpResponse * response, const char * path);
    static bool onMethodNotFound(CHttpResponse * response, const char * httpMethod, const char * path);

    PrometheusReporterService(PrometheusMetricsReportTrigger * parent, const std::map<std::string, std::string> & parms);
    virtual ~PrometheusReporterService() {};
    int start();
    int stop();
    bool isProcessing();
};

class PremetheusMetricSink : public hpccMetrics::MetricSink
{
    static const char * mapMockMetricTypeToPrometheusStr(MetricType type)
    {
        switch (type)
        {
            case hpccMetrics::COUNTER:
                return "counter";
            case hpccMetrics::GAUGE:
//            case hpccMetrics::RATE: //I'm not sure about this one
                return "gauge";
            default:
                return nullptr;
        }
    }
public:
    explicit PremetheusMetricSink(const std::string & name, const std::map<std::string, std::string> & parms) :

    MetricSink(std::move(name), COLLECTOR_NAME) {}

    void send(const MeasurementVector &values, const std::shared_ptr<IMetricSet> &pMetricSet, MetricsReportContext *pContext) override
    {
        StringBuffer payload;
        //std::string metricSetName = pMetricSet->getName(); -- should this be part of the fully qualified metric name???

        PrometheusMetricsReportContext * promContext = static_cast<PrometheusMetricsReportContext *>(pContext);
        assertex(promContext != nullptr);

        for (auto const &pMeas : values)
        {
            std::string name = pMeas->getName();
            //const char * fullName = metric.getFullName();

            if (!pMeas->getDescription().empty())
                payload.append("# HELP ").append(name.c_str()).append(" ").append(pMeas->getDescription().c_str()).append("\n");
            MetricType type = pMeas->getMetricType();
            if (type != MetricType::UNKNOWN && mapMockMetricTypeToPrometheusStr(type))
                payload.append("# TYPE ").append(name.c_str()).append(" ").append(mapMockMetricTypeToPrometheusStr(type)).append("\n");

            payload.append(name.c_str()).append(" ").append(pMeas->valueToString().c_str()).append("\n");
        }

        promContext->setMetrics(payload.str());
    }
};

class PrometheusMetricsReportTrigger : public MetricsReportTrigger
{
private:

    Owned<PrometheusReporterService> m_server;

public:
    void getContextContent(StringBuffer & content)
    {
        Owned<PrometheusMetricsReportContext> reportContext = new PrometheusMetricsReportContext();
        doReport(std::string(COLLECTOR_NAME), reportContext.get());
        content.set(reportContext->getMetrics());
    }

                                   //this should be an iproptree!!!!!!
    PrometheusMetricsReportTrigger(const std::map<std::string, std::string> & parms)
    {
        InitModuleObjects(); //logging this might be a requirement on the framework

        try
        {
            m_server.set(new PrometheusReporterService(this, parms));
        }
        catch(IException *e)
        {
            StringBuffer emsg;
            LOG(MCuserInfo, "Error setting up PrometheusReporterService");
            e->Release();
        }
    }

    void start() override
    {
        stopCollection = false;
        collectThread = std::thread(collectionThread, this);
    }

    void stop() override
    {
        stopCollection = true;
        m_server->stop();
        collectThread.join();
    }

    bool isStopCollection() const
    {
        return stopCollection;
    }

    void startServer()
    {
        m_server->start();
    }

protected:
    static void collectionThread(PrometheusMetricsReportTrigger * pReportTrigger)
    {
        pReportTrigger->startServer();
    }

private:
    bool stopCollection = false;
    std::thread collectThread;
};


#endif /* SUBPROJECTS__HPCCSYSTEMS_PLATFORM_METRICS_PROMETHEUS_REPORTER_PROMETHEUSREPORTERSERVICE_H_ */