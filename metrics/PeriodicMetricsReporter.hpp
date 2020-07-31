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

#pragma once

#include <memory>
#include <vector>
#include <thread>
#include "MetricsReporter.hpp"

namespace hpccMetrics
{

class PeriodicMetricsReporter : public MetricsReporter
{
    public:

        explicit PeriodicMetricsReporter() :
                periodSeconds{0},
                stopCollection{false}
            { }

        ~PeriodicMetricsReporter() override = default;

        void start(unsigned seconds)
        {
            init();
            periodSeconds = std::chrono::seconds(seconds);
            collectThread = std::thread(collectionThread, this);
        }

        void stop()
        {
            stopCollection = true;
            collectThread.join();
        }

        bool isStopCollection() const
        {
            return stopCollection;
        }

        static void collectionThread(PeriodicMetricsReporter *pMetricCollector)
        {
            while (!pMetricCollector->isStopCollection())
            {
                std::this_thread::sleep_for(pMetricCollector->periodSeconds);
                pMetricCollector->report();
            }
        }


    private:

        bool stopCollection;
        std::thread collectThread;
        std::chrono::seconds periodSeconds;
};

}
