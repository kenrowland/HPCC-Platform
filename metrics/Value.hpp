//
// Created by rowlke01 on 3/6/20.
//
#pragma once


namespace hpcc_metrics {

    class MetricValueBase {
        public:

            explicit MetricValueBase(std::string name) : m_name{std::move(name)} {}
            virtual ~MetricValueBase() = default;

        protected:

            std::string m_name;
    };


    <template typename T>
    class ValueXX : public MetricValueBase {
        public:
            MetricValue(std::string name, T value) :
                    MetricValueBase(name),
                    m_name{std::move(name)} {}

            std::string toString() const {
                return std::to_string(m_value);
            }

        protected:

            T m_value;
    };

}
