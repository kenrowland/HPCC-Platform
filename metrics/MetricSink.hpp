//
// Created by rowlke01 on 3/5/20.
//

#pragma once

namespace hpcc_metrics {

    class MetricSink {

        public:
            MetricSink() = default;
            virtual ~MetricSink() = default;
            virtual void send(const std::vector<std::shared_ptr<MetricValueBase>> &values);
    };


    class FileMetricSink : public MetricSink
    {
        public:

            FileMetricSink(std::string filename) :
                m_filename{std::move(filename)}
            {}



        protected:

            std::string m_filename;
    };
}
