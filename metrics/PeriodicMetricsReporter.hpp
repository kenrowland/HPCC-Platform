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
#include "MetricsReporter.hpp"

namespace hpcc_metrics
{
    class PeriodicMetricsReporter : public MetricsReporter
    {
        public:

            explicit PeriodicMetricsReporter() :
                m_periodSeconds{0},
                m_stopCollection{false}
                { }

            ~PeriodicMetricsReporter() override = default;

            void start(unsigned periodSeconds)
            {
                init();
                m_periodSeconds = std::chrono::seconds(periodSeconds);
                m_collectThread = std::thread(collectionThread, this);
                m_collectThread.detach();
            }

            void stop()
            {
                m_stopCollection = true;
            }

            bool isStopCollection() const { return m_stopCollection; }
            static void collectionThread(PeriodicMetricsReporter *pMetricCollector);


        private:

            bool m_stopCollection;
            std::thread m_collectThread;
            std::chrono::seconds m_periodSeconds;
    };


    void PeriodicMetricsReporter::collectionThread(PeriodicMetricsReporter *pMetricCollector)
    {
        while (!pMetricCollector->isStopCollection())
        {
            std::this_thread::sleep_for(pMetricCollector->m_periodSeconds);
            pMetricCollector->report();
        }
    }
}
