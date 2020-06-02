

#include <cstdio>

#include "MetricsRegistry.hpp"
#include "MetricSet.hpp"
#include "FileSink.hpp"
#include "CountableMetric.hpp"
#include "QueueSizeMetric.hpp"
#include "QueueLatencyMetric.hpp"
#include "RateMetric.hpp"
#include "PeriodicMetricsReporter.hpp"
#include <thread>
#include <chrono>

#include "MetricDistribution.hpp"

using namespace hpcc_metrics;

MetricsRegistry mr;

void processThread(int, unsigned);
std::shared_ptr<CountableMetric> pCountableMetric;
std::shared_ptr<QueueSizeMetric> pQueueSizeMetric;
std::shared_ptr<QueueLatencyMetric> pQueueLatencyMetric;
std::shared_ptr<RateMetric> pRateMetric;

int main(int argc, char *argv[])
{

    //
    // Create a metric set for request type metrics
    auto pRequestMetricSet = std::make_shared<MetricSet>();
    pCountableMetric     = std::make_shared<CountableMetric>("requests");
    pRateMetric = std::make_shared<RateMetric>("rate");
    pRequestMetricSet->addMetric(pCountableMetric);
    pRequestMetricSet->addMetric(pRateMetric);
    mr.add(pCountableMetric);  // demo use of the registry (optional)

    //
    // create a metric set for queues
    auto pQueueMetricSet = std::make_shared<MetricSet>();
    pQueueSizeMetric = std::make_shared<QueueSizeMetric>("queuesize");
    pQueueLatencyMetric = std::make_shared<QueueLatencyMetric>("queue_latency");
    pQueueMetricSet->addMetric(pQueueSizeMetric);
    pQueueMetricSet->addMetric(pQueueLatencyMetric);

    //
    // Create a file sink for saving metric values
    //auto pFileSink = std::make_shared<FileMetricSink>("/home/ken/metric_output.txt");

    //
    // Create a collector
    PeriodicMetricsReporter collector;
    collector.addMetricSet(pRequestMetricSet);
    collector.addMetricSet(pQueueMetricSet);
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
        mr.get<CountableMetric>("requests")->update(1u);
        pQueueSizeMetric->addElement();
        pQueueLatencyMetric->add();
        pRateMetric->update();
        std::this_thread::sleep_for(std::chrono::seconds(delay));
        pQueueSizeMetric->removeElement();
        pQueueLatencyMetric->remove();
    }
}
