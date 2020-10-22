

#include <cstdio>
#include <thread>
#include <chrono>
#include <jptree.hpp>
#include "Metrics.hpp"
#include "ComponentConfigHelper.hpp"


using namespace hpccMetrics;

void processThread(int, unsigned);
std::shared_ptr<EventCountMetric> pEventCountMetric;
std::shared_ptr<GaugeMetric<uint32_t>> pQueueSizeMetric;
//std::shared_ptr<QueueLatencyMetric> pQueueLatencyMetric;
std::shared_ptr<RateMetric> pRateMetric;

MetricsReporter *pReporter;


const char *testYml = R"!!(config:
  metrics:
    name: config_name
    sinks:
      - type: sinktype
        name: sinkname
        config:
          key1: data1
          key2: data2
          Key3:
            key3.1: data3.1
            key3.2: data3.2
    prefixes:
      - key1: val1
      - key2: val2
)!!";


const char *globalConfigYml = R"!!(config:
  metrics:
    name: cluster config
    sinks:
      - type: filesink
        name: default
        settings:
          filename: /testout.txt
          clear: true
    report_trigger:
      type: periodic
      settings:
        period: 15
)!!";


const char *localConfigYml = R"!!(config:
  metrics:
    name: config_name
    prefix: myprefix.
    metric_set_prefixis:
      - set_name: set1
        prefix: set1prefix.
      - set_name: set2
        prefix: prefixset2.
)!!";

ComponentConfigHelper testCfgHelper;

int main(int argc, char *argv[])
{
    InitModuleObjects();

    //
    // Simulate retrieving the component and global config
    IPropertyTree *pSettingsGlobal = createPTreeFromYAMLString(globalConfigYml, ipt_none, ptr_ignoreWhiteSpace, nullptr);
    IPropertyTree *pSettingsLocal = createPTreeFromYAMLString(localConfigYml, ipt_none, ptr_ignoreWhiteSpace, nullptr);

    //
    // Retrieve the glocal and component metrics config
    IPropertyTree *pGlobalMetricsTree = pSettingsGlobal->getPropTree("config/metrics");
    IPropertyTree *pLocalMetricsTree = pSettingsLocal->getPropTree("config/metrics");


    //
    // Allow override of output file for the file sink
    if (argc > 1)
    {
        std::string sinkReportFilename;
        sinkReportFilename = std::string(argv[1]);
        auto pSinkTree = pSettingsGlobal->getPropTree("config/metrics/sinks[1]/settings");
        pSinkTree->removeProp("@filename");
        pSinkTree->addProp("@filename", sinkReportFilename.c_str());
    }

    //
    // Initialize the config helper object
    testCfgHelper.init(pGlobalMetricsTree, pLocalMetricsTree);

    //
    // Create and add metrics to their named sets using the config helper object
    pEventCountMetric = std::make_shared<EventCountMetric>("requests", "The number of requests that have come in");
    testCfgHelper.addMetricToSet(pEventCountMetric, "set1");

    pRateMetric = std::make_shared<RateMetric>("rate", "");
    testCfgHelper.addMetricToSet(pRateMetric, "set1");

    pQueueSizeMetric = std::make_shared<GaugeMetric<uint32_t>>("queuesize", "", ValueType::INTEGER);
    testCfgHelper.addMetricToSet(pQueueSizeMetric, "set2");

    //
    // Start collecting
    testCfgHelper.start();

    //
    // Starts some threads, each updating metrics
    std::thread first (processThread, 20, 2);
    std::thread second (processThread, 15, 3);

    first.join();
    second.join();

    testCfgHelper.stop();

    printf("Test complete\n");
}


void processThread(int numLoops, unsigned delay)
{
    for (int i=0; i<numLoops; ++i)
    {
        pEventCountMetric->inc(1u);
        pQueueSizeMetric->inc(1);
        pRateMetric->inc(1);
        std::this_thread::sleep_for(std::chrono::seconds(delay));
        pQueueSizeMetric->dec(1);
    }
}
