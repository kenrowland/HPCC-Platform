

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
          filename: /home/rowlke01/testout.txt
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

//    IPropertyTree *pTestSettings = createPTreeFromYAMLString(testYml, ipt_none, ptr_ignoreWhiteSpace, nullptr);
//    IPropertyTreeIterator *pPrefixIter = pTestSettings->getElements("config/metrics/prefixes");
//    pPrefixIter->first();
//    IPropertyTree &sinkTree = pPrefixIter->query();
//    StringBuffer setName;
//    sinkTree.getName(setName);
//    StringBuffer prefix;
//    sinkTree.getProp("", prefix);

    IPropertyTree *pGlobal = createPTreeFromYAMLString(globalConfigYml, ipt_none, ptr_ignoreWhiteSpace, nullptr);
    IPropertyTree *pGlobalTree = pGlobal->getPropTree("config/metrics");
    StringBuffer cfgName;
    pGlobalTree->getProp("@name", cfgName);
    unsigned num = pGlobal->numChildren();
    IPropertyTree *pTriggerTree = pGlobalTree->getPropTree("./report_trigger");
    StringBuffer cfgTriggerType;
    pTriggerTree->getProp("@type", cfgTriggerType);  // this one is required
    std::string type(cfgTriggerType.str());

    IPropertyTree *pSettings = pTriggerTree->getPropTree("./settings");
    StringBuffer seconds;
    bool hasProp = pSettings->hasProp("@period");
    pSettings->getProp("@period", seconds);





    //Owned<IPropertyTree> configTree;
    //configTree.setown(createPTreeFromYAMLFile(filename, 0, ptr_ignoreWhiteSpace, nullptr));
    IPropertyTree *pSettingsGlobal = createPTreeFromYAMLString(globalConfigYml, ipt_none, ptr_ignoreWhiteSpace, nullptr);
    IPropertyTree *pSettingsLocal = createPTreeFromYAMLString(localConfigYml, ipt_none, ptr_ignoreWhiteSpace, nullptr);

    IPropertyTree *pGlobalMetricsTree = pSettingsGlobal->getPropTree("config/metrics");
    IPropertyTree *pLocalMetricsTree = pSettingsLocal->getPropTree("config/metrics");

    testCfgHelper.init(pGlobalMetricsTree, pLocalMetricsTree);

    pEventCountMetric = std::make_shared<EventCountMetric>("requests", "The number of requests that have come in");
    testCfgHelper.addMetricToSet(pEventCountMetric, "set1");

    pRateMetric = std::make_shared<RateMetric>("rate", "");
    testCfgHelper.addMetricToSet(pRateMetric, "set1");

    pQueueSizeMetric = std::make_shared<GaugeMetric<uint32_t>>("queuesize", "", ValueType::INTEGER);
    testCfgHelper.addMetricToSet(pQueueSizeMetric, "set2");

    testCfgHelper.start();


//    StringBuffer _name;
//    pMetricsTree->getProp("@name", _name);
//    std::string name(_name.str());



//    MetricsReportConfig reportConfig;
//
//    //
//    // Create a metric set for request type metrics
//    std::vector<std::shared_ptr<IMetric>> metrics;
//    pEventCountMetric     = std::make_shared<EventCountMetric>("requests", "The number of requests that have come in");
//    metrics.emplace_back(pEventCountMetric);
//
//    pRateMetric = std::make_shared<RateMetric>("rate", "");
//    metrics.emplace_back(pRateMetric);
//
//    auto pRequestMetricSet = std::make_shared<MetricSet>("set", "myprefix.", metrics);
//
//    //
//    // create a metric set for queues
//    metrics.clear();
//    pQueueSizeMetric = std::make_shared<GaugeMetric<uint32_t>>("queuesize", "", ValueType::INTEGER);
//    metrics.emplace_back(pQueueSizeMetric);
//    auto pQueueMetricSet = std::make_shared<MetricSet>("set2", "myprefix2", metrics);

//    //
//    // Get the name of thee report file
//    auto pSinkSettings = createPTree("SinkSettings");
//    std::string sinkReportFilename;
//    if (argc > 1)
//    {
//        StringBuffer fname;
//        sinkReportFilename = std::string(argv[1]);
//        pSinkSettings->addProp("filename", sinkReportFilename.c_str());
//        pSinkSettings->getProp("filename", fname);
//    }
//    else
//    {
//        printf("You must provide the full path to the report file\n\n");
//        exit(0);
//    }
//
//    auto pSink = MetricSink::getSinkFromLib("filesink", nullptr, "es", pSinkSettings);
//    reportConfig.addReportConfig(pSink, pRequestMetricSet);
//    reportConfig.addReportConfig(pSink, pQueueMetricSet);
//
//    IPropertyTree *pTriggerSettings = createPTree("TriggerSettings");;
//    pTriggerSettings->addPropInt("period", 10);
//    IMetricsReportTrigger *pTrigger = MetricsReportTrigger::getTriggerFromLib("periodic", nullptr, pTriggerSettings);
//
//    pReporter = new MetricsReporter(reportConfig, pTrigger);
//
//    //
//    // start collection
//    pReporter->start();

    std::thread first (processThread, 20, 1);
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
