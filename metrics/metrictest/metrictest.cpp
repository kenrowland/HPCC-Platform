

#include <cstdio>

#include "Metrics.hpp"
#include "MetricsRegistry.hpp"
#include "MetricSet.hpp"
//#include "FileSink.hpp"
//#include "CountMetric.hpp"
//#include "RateMetric.hpp"
//#include "GaugeMetric.hpp"
#include "PeriodicMetricsReporter.hpp"
#include <thread>
#include <chrono>


using namespace hpccMetrics;

MetricsRegistry mr;

void processThread(int, unsigned);
std::shared_ptr<CountMetric> pCountableMetric;
std::shared_ptr<GaugeMetric<uint32_t>> pQueueSizeMetric;
//std::shared_ptr<QueueLatencyMetric> pQueueLatencyMetric;
std::shared_ptr<RateMetric> pRateMetric;


/*

 Some notes on configuration

 Metric is a collected value
 Metrics must be part of a MetricSet. A metric can only be in one metric set. Name of metric must be unique
 MetricSet is determined by the component
 MetricSets are named and are well known by component
 MetricSink is the component that reports metric values to a store (file, elasticsearch, datadog, prometheus, etc)
 MetricReporter is a container of
   one or more MetricSets
   one or more MetricSinks
   represents a reporting set. All MetricSets are collected and reported to all MetricSinks.


 Global level:

 Things that can be defined for reuse at the component level

 metrics:
   sinks:
   - name: sinkname for reference
     type: elasticsearch
     host: <hostname>
     port: <port>
   - name: sink2
     type: file
     other stuff...

   reporters:
   - name: myreporter    can be referred to as a set of values
     type: periodic
     period: 10
   - name: reporter2
     type: polled


 Component level:

 Defines specifics of metrics for the component. Can reference global values

 metrics:
   sinks:  <see global for format>

   reporters:  <see global for format>

   metric_sets:
   - set
     set_names:
     - <set name>   # this is the
     - <set name>   # metric set name
     sinks:
     - <sink name>
     - <sink name2>
     reporter: reporter name



 Component level
 metrics:
   sinks:
   - name:  if same, then overrides existing, otherwise creates a new one


*/


int main(int argc, char *argv[])
{

    //
    // Create a metric set for request type metrics
    auto pRequestMetricSet = std::make_shared<MetricSet>("set", "myprefix.");
    pCountableMetric     = std::make_shared<CountMetric>("requests");
    pCountableMetric->setType("integer");
    pRateMetric = std::make_shared<RateMetric>("rate");
    pRateMetric->setType("float");
    pRequestMetricSet->addMetric(pCountableMetric);
    pRequestMetricSet->addMetric(pRateMetric);
    mr.add(pCountableMetric);  // demo use of the registry (optional)

    //
    // create a metric set for queues
    auto pQueueMetricSet = std::make_shared<MetricSet>("set2", "myprefix2");
    pQueueSizeMetric = std::make_shared<GaugeMetric<uint32_t>>("queuesize");
    pQueueMetricSet->addMetric(pQueueSizeMetric);

    //
    // Create a file sink for saving metric values

    //
    // Create a collector
    PeriodicMetricsReporter collector;
    collector.addMetricSet(pRequestMetricSet);
    collector.addMetricSet(pQueueMetricSet);
    std::map<std::string, std::string> parms = { {"filename", "/home/ken/metricsreport.txt"}};
    auto pSink = MetricSink::getMetricSinkFromLib("filesink", parms);
    collector.addSink(pSink);

    //
    // start collection
    collector.start(10);

    std::thread first (processThread, 20, 1);
    std::thread second (processThread, 15, 3);

    first.join();
    second.join();

    collector.stop();

    printf("Test complete\n");
}


void processThread(int numLoops, unsigned delay)
{
    for (int i=0; i<numLoops; ++i)
    {
        mr.get<CountMetric>("requests")->inc(1u);
        pQueueSizeMetric->inc(1);
        pRateMetric->inc(1);
        std::this_thread::sleep_for(std::chrono::seconds(delay));
        pQueueSizeMetric->dec(1);
    }
}
