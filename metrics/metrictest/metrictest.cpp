

#include <cstdio>
#include <thread>
#include <chrono>
#include <jptree.hpp>
#include "Metrics.hpp"
//#include "ComponentConfigHelper.hpp"


using namespace hpccMetrics;

void processThread(int, unsigned);
std::shared_ptr<CounterMetric> pEventCountMetric;
std::shared_ptr<GaugeMetric<uint32_t>> pQueueSizeMetric;
//std::shared_ptr<QueueLatencyMetric> pQueueLatencyMetric;
//std::shared_ptr<RateMetric> pRateMetric;

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
          filename: testout.txt
          clear: true
          period: 5
)!!";


const char *localConfigYml = R"!!(config:
  metrics:
    name: config_name
    prefix: myprefix.
    sinks:
      - name: default
        metrics:
          - name: myprefix.requests(count)
          - name: myprefix.requests(resetting_count)
          - name: myprefix.requests(rate)
            description: Number of request arriving per second
          - name: myprefix.queuesize
)!!";

//ComponentConfigHelper testCfgHelper;
MetricsReporter reporter;

int main(int argc, char *argv[])
{
    InitModuleObjects();

    //
    // Simulate retrieving the component and global config
    IPropertyTree *pSettingsGlobal = createPTreeFromYAMLString(globalConfigYml, ipt_none, ptr_ignoreWhiteSpace, nullptr);
    IPropertyTree *pSettingsLocal = createPTreeFromYAMLString(localConfigYml, ipt_none, ptr_ignoreWhiteSpace, nullptr);

    //
    // Retrieve the global and component metrics config
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
    // Init reporter with config
    reporter.init(pGlobalMetricsTree, pLocalMetricsTree);

    //
    // Now create the metrics and add them to the reporter
    pEventCountMetric = std::make_shared<CounterMetric>("requests", "The number of requests");
    reporter.addMetric(pEventCountMetric);
    pQueueSizeMetric = std::make_shared<GaugeMetric<uint32_t>>("queuesize", "request queue size", hpccMetrics::FLOAT);
    reporter.addMetric(pQueueSizeMetric);

    reporter.startCollecting();


    //
    // Starts some threads, each updating metrics
    std::thread first (processThread, 20, 2);
    std::thread second (processThread, 15, 3);

    first.join();
    second.join();

    reporter.stopCollecting();

    printf("Test complete\n");
}


void processThread(int numLoops, unsigned delay)
{
    for (int i=0; i<numLoops; ++i)
    {
        pEventCountMetric->inc(2u);
        pQueueSizeMetric->inc(3);
        std::this_thread::sleep_for(std::chrono::seconds(delay));
        pQueueSizeMetric->dec(1);
    }
}
