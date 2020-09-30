//
// Created by ken on 9/9/20.
//

#ifndef HPCCSYSTEMS_PLATFORM_HOLDONTO_HPP
#define HPCCSYSTEMS_PLATFORM_HOLDONTO_HPP

// This distribution metric is still being worked out
//template <typename T>
//class DistributionMetric : public Metric
//{
//    public:
//        DistributionMetric(std::string &name, const std::vector<T> &bucketLevels) :
//                Metric{std::move(name)}
//        {
//            if (bucketLevels[0] != std::numeric_limits<T>::min())
//                buckets[0] = new std::atomic<uint32_t>();
//
//            for (auto const &distValue : bucketLevels)
//            {
//                buckets.insert(std::pair<T, std::atomic<unsigned> *>(distValue, new std::atomic<unsigned>()));
//            }
//
//            if (bucketLevels[bucketLevels.size() - 1] != std::numeric_limits<T>::max())
//                buckets.insert(std::pair<T, std::atomic<uint32_t> *>(std::numeric_limits<T>::max(),
//                                                                     new std::atomic<uint32_t>()));
//        }
//
//        void inc(T level, uint32_t val)
//        {
//            auto it = buckets.lower_bound(level);
//            if (it != buckets.end())
//            {
//                it.second += val;
//            }
//        }
//
//        // TBD
//        bool collect(MeasurementVector &values) override
//        {
//            return false;
//        }
//
//    protected:
//        std::map<T, std::atomic<uint32_t> *> buckets;
//};

#endif //HPCCSYSTEMS_PLATFORM_HOLDONTO_HPP
