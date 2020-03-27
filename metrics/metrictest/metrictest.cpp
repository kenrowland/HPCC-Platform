

#include <cstdio>

#include "MetricsManager.hpp"
#include "FileSink.hpp"
#include "CountableMetric.hpp"
#include <thread>
#include <chrono>

#include "MetricDistribution.hpp"

using namespace hpcc_metrics;

MetricsManager mm;

void processThread(int, unsigned);
std::shared_ptr<CountableMetric> pMetric;

int main(int argc, char *argv[])
{
    std::string distName = "distname";
    MetricDistribution<unsigned> md(distName, std::vector<unsigned>{10, 20, 30});


//    std::map<unsigned, std::string> latency;
//    latency[0] = "a";
//    latency[10] = "b"; // <-- 0 - 10
//    latency[20] = "c"; // <-- 11-19
//    latency[30] = "d"; // <-- 20-29
//    latency[UINT32_MAX] = "e"; // <-- 30 and up
//
//    auto itLow = latency.lower_bound(9);
//    itLow = latency.lower_bound(10);  // b
//    itLow = latency.lower_bound(11);  // c
//    itLow = latency.lower_bound(15);  // c
//    itLow = latency.lower_bound(19);  // c
//    itLow = latency.lower_bound(20);  // c
//    itLow = latency.lower_bound(22);  // d
//    itLow = latency.lower_bound(31);  // e
//    //auto itHi = latency.lower_bound(9);


    std::string name = "mymetricset";
    auto pFileSink = std::make_shared<FileMetricSink>("/home/ken/metric_output.txt");
    auto pMetricSet = mm.createMetricSet(name, pFileSink);
    pMetric = mm.createMetric<CountableMetric>("numrequests");
    pMetricSet->addMetric(pMetric);
    pMetricSet->setCollectionPeriod(5);
    pMetricSet->startCollection();

    //pMetric->inc(2);

    std::thread first (processThread, 20, 2);
    std::thread second (processThread, 15, 3);

    first.join();
    second.join();

    pMetricSet->stopCollection();

//    unsigned curVal = pMetric->getAndClear();
//    unsigned clearedVal = pMetric->getAndClear();

    printf("Hello World\n");
}


void processThread(int numLoops, unsigned delay)
{
    for (int i=0; i<numLoops; ++i)
    {
        pMetric->inc(1);
        std::this_thread::sleep_for(std::chrono::seconds(delay));
    }
}
