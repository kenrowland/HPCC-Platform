

#include <cstdio>

#include "MetricsRegistry.hpp"
#include "MetricSet.hpp"
//#include "FileSink.hpp"
#include "CountMetric.hpp"
#include "QueueLatencyMetric.hpp"
#include "RateMetric.hpp"
#include "GaugeMetric.hpp"
#include "PeriodicMetricsReporter.hpp"
#include <thread>
#include <chrono>

#include "DistributionMetric.hpp"

using namespace hpcc_metrics;

MetricsRegistry mr;

void processThread(int, unsigned);
std::shared_ptr<CountMetric<uint32_t>> pCountableMetric;
std::shared_ptr<GaugeMetric<uint32_t>> pQueueSizeMetric;
std::shared_ptr<GaugeMetric<uint32_t>> pGaugeMetric;
//std::shared_ptr<QueueLatencyMetric> pQueueLatencyMetric;
std::shared_ptr<RateMetric> pRateMetric;


/*

 What needs to be specified
 - Sinks and metric sets (would be nice if a single sink, it gets all metric sets)

 Global level:

 metrics:
   sinks:
   - name: elasticsearchsink
     type: elasticsearch
     host: <hostname>
     port: <port>

   reporters
   - name: myreporter
     type: periodic
     period: 10


 Component level
 metrics:
   sinks:
   - name:  if same, then overrides existing, otherwise creates a new one


*/


int main(int argc, char *argv[])
{

    //
    // Create a metric set for request type metrics
    auto pRequestMetricSet = std::make_shared<MetricSet>("set");
    pCountableMetric     = std::make_shared<CountMetric<uint32_t>>("requests");
    pCountableMetric->setType("integer");
    pRateMetric = std::make_shared<RateMetric>("rate");
    pRateMetric->setType("float");
    pRequestMetricSet->addMetric(pCountableMetric);
    pRequestMetricSet->addMetric(pRateMetric);
    mr.add(pCountableMetric);  // demo use of the registry (optional)

    //
    // create a metric set for queues
    auto pQueueMetricSet = std::make_shared<MetricSet>("set2");
    pQueueSizeMetric = std::make_shared<GaugeMetric<uint32_t>>("queuesize");
    pGaugeMetric = std::make_shared<GaugeMetric<uint32_t>>("gauge");
//    pQueueLatencyMetric = std::make_shared<QueueLatencyMetric>("queue_latency");
    pQueueMetricSet->addMetric(pQueueSizeMetric);
//    pQueueMetricSet->addMetric(pQueueLatencyMetric);

    //
    // Create a file sink for saving metric values
    //auto pFileSink = std::make_shared<FileMetricSink>("/home/ken/metric_output.txt");

    //
    // Create a collector
    PeriodicMetricsReporter collector;
    collector.addMetricSet(pRequestMetricSet);
    collector.addMetricSet(pQueueMetricSet);
    std::map<std::string, std::string> parms = { {"filename", "metrics.txt"}};
    collector.addSink("elasticsearchsink", parms);
//    collector.addSink("filesink", parms);
    //collector.addSink(pFileSink);

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
        mr.get<CountMetric<uint32_t>>("requests")->inc(1u);
        pQueueSizeMetric->inc(1);
//        pQueueLatencyMetric->add();
        pRateMetric->inc(1);
        std::this_thread::sleep_for(std::chrono::seconds(delay));
        pQueueSizeMetric->dec(1);
//        pQueueLatencyMetric->remove();
    }
}
