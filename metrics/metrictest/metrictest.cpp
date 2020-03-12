

#include "stdio.h"

#include "MetricsManager.hpp"
#include <thread>
#include <chrono>

using namespace hpcc_metrics;

MetricsManager mm;

void processThread(int, unsigned);
std::shared_ptr<CountableMetric> pMetric;

int main(int argc, char *argv[])
{

    std::string name = "mymetricset";
    auto pMetricSet = mm.createMetricSet(name);
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
