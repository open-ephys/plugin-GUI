/*
  ==============================================================================

   This file is part of the JUCE 8 technical preview.
   Copyright (c) Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#if JUCE_DIRECT2D_METRICS

namespace juce
{

    namespace direct2d
    {
        struct Metrics : public ReferenceCountedObject
        {
            using Ptr = ReferenceCountedObjectPtr<Metrics>;

#define DIRECT2D_PAINT_STAT_LIST \
        DIRECT2D_PAINT_STAT(messageThreadPaintDuration) \
        DIRECT2D_PAINT_STAT(swapChainThreadTime) \
        DIRECT2D_PAINT_STAT(frameInterval) \
        DIRECT2D_PAINT_STAT(endDrawDuration) \
        DIRECT2D_PAINT_STAT(present1Duration) \
        DIRECT2D_PAINT_STAT(createGeometryTime) \
        DIRECT2D_PAINT_STAT(drawGeometryTime) \
        DIRECT2D_PAINT_STAT(fillGeometryTime) \
        DIRECT2D_PAINT_STAT(createFilledGRTime) \
        DIRECT2D_PAINT_STAT(createStrokedGRTime) \
        DIRECT2D_PAINT_STAT(drawGRTime) \
        DIRECT2D_PAINT_STAT(createGradientTime) \
        DIRECT2D_PAINT_STAT(pushAliasedAxisAlignedLayerTime) \
        DIRECT2D_PAINT_STAT(pushGeometryLayerTime) \
        DIRECT2D_PAINT_STAT(fillTranslatedRectTime) \
        DIRECT2D_PAINT_STAT(fillAxisAlignedRectTime) \
        DIRECT2D_PAINT_STAT(fillTransformedRectTime) \
        DIRECT2D_PAINT_STAT(fillRectListTime) \
        DIRECT2D_PAINT_STAT(drawImageTime) \
        DIRECT2D_PAINT_STAT(spriteBatchTime) \
        DIRECT2D_PAINT_STAT(spriteBatchSetupTime) \
        DIRECT2D_PAINT_STAT(createSpriteBatchTime) \
        DIRECT2D_PAINT_STAT(createSpriteSourceTime) \
        DIRECT2D_PAINT_STAT(setSpritesTime) \
        DIRECT2D_PAINT_STAT(addSpritesTime) \
        DIRECT2D_PAINT_STAT(clearSpritesTime) \
        DIRECT2D_PAINT_STAT(drawSpritesTime) \
        DIRECT2D_PAINT_STAT(drawGlyphRunTime) \
        DIRECT2D_PAINT_STAT(createBitmapTime) \
        DIRECT2D_PAINT_STAT(mapBitmapTime) \
        DIRECT2D_LAST_PAINT_STAT(unmapBitmapTime)

#define DIRECT2D_PAINT_STAT(name) name,
#define DIRECT2D_LAST_PAINT_STAT(name) name
            enum
            {
                DIRECT2D_PAINT_STAT_LIST,
                numStats
            };
#undef DIRECT2D_PAINT_STAT
#undef DIRECT2D_LAST_PAINT_STAT

#define DIRECT2D_PAINT_STAT(name) # name,
#define DIRECT2D_LAST_PAINT_STAT(name) # name
            StringArray const accumulatorNames
            {
                DIRECT2D_PAINT_STAT_LIST
            };
#undef DIRECT2D_PAINT_STAT
#undef DIRECT2D_LAST_PAINT_STAT

            CriticalSection& lock;
            String const name;
            void* const windowHandle;
            int64 const  creationTime = Time::getMillisecondCounter();
            double const millisecondsPerTick = 1000.0 / (double)Time::getHighResolutionTicksPerSecond();
            int          paintCount = 0;
            int          presentCount = 0;
            int          present1Count = 0;
            int64        lastPaintStartTicks = 0;
            uint64       lockAcquireMaxTicks = 0;

            Metrics(CriticalSection& lock_, String name_, void* windowHandle_)
                : lock(lock_),
                name(name_),
                windowHandle(windowHandle_)
            {
            }
            ~Metrics() = default;

            void startFrame()
            {
                ScopedLock locker{ lock };
                zerostruct(sums);
            }

            void finishFrame()
            {
            }

            void reset()
            {
                ScopedLock locker{ lock };

                for (auto& accumulator : runningAccumulators)
                {
                    accumulator.reset();
                }

                lastPaintStartTicks = 0;
                paintCount = 0;
                present1Count = 0;
                lockAcquireMaxTicks = 0;
            }

            auto& getAccumulator(size_t index) noexcept
            {
                return runningAccumulators[index];
            }

            auto getSum(size_t index) const noexcept
            {
                return sums[index];
            }

            void addValueTicks(size_t index, int64 ticks)
            {
                addValueMsec(index, Time::highResolutionTicksToSeconds(ticks) * 1000.0);
            }

            void addValueMsec(size_t index, double value)
            {
                ScopedLock locker{ lock };

                auto& accumulator = runningAccumulators[index];

                switch (index)
                {
                case frameInterval:
                    if (accumulator.getCount() > 100)
                    {
                        accumulator.reset();
                    }
                    break;
                }
                accumulator.addValue(value);

                sums[index] += value;
            }

        private:
            std::array<StatisticsAccumulator<double>, numStats> runningAccumulators;
            std::array<double, numStats> sums;
        };

        struct ScopedElapsedTime
        {
            ScopedElapsedTime(Metrics::Ptr& metrics_, size_t accumulatorIndex_)
                : metrics(metrics_.get()),
                accumulatorIndex(accumulatorIndex_)
            {
            }

            ScopedElapsedTime(Metrics* metrics_, size_t accumulatorIndex_)
                : metrics(metrics_),
                accumulatorIndex(accumulatorIndex_)
            {
            }

            ~ScopedElapsedTime()
            {
                auto finishTicks = Time::getHighResolutionTicks();
                metrics->addValueTicks(accumulatorIndex, finishTicks - startTicks);
            }

            int64           startTicks = Time::getHighResolutionTicks();
            Metrics* metrics;
            size_t             accumulatorIndex;
        };

        class MetricsHub : public DeletedAtShutdown
        {
        public:
            MetricsHub()
            {
                imageContextMetrics = new Metrics{ lock, "Image " + getProcessString(), nullptr };
                add(imageContextMetrics);
            }

            ~MetricsHub() override
            {
                clearSingletonInstance();
            }

            void add(Metrics::Ptr metrics)
            {
                metricsArray.insert(0, metrics);
            }

            void remove(Metrics::Ptr metrics)
            {
                metricsArray.removeObject(metrics);
            }

            ReferenceCountedObjectPtr<Metrics> getMetricsForWindowHandle(void* windowHandle) noexcept
            {
                for (auto& metrics : metricsArray)
                {
                    if (metrics->windowHandle == windowHandle)
                    {
                        return metrics;
                    }
                }

                return nullptr;
            }

            enum
            {
                getValuesRequest,
                resetValuesRequest
            };

            struct MetricValues
            {
                size_t count;
                double total;
                double average;
                double minimum;
                double maximum;
                double stdDev;
            };

            struct GetValuesResponse
            {
                int responseType;
                void* windowHandle;
                MetricValues values[Metrics::numStats];
            };

            CriticalSection lock;
            Metrics::Ptr imageContextMetrics;

            static constexpr int magicNumber = 0xd2d1;

            JUCE_DECLARE_SINGLETON(MetricsHub, false)

        private:
            static String getProcessString() noexcept;

            void resetAll();

            struct HubPipeServer : public InterprocessConnection
            {
                HubPipeServer(MetricsHub& owner_) :
                    InterprocessConnection(false, magicNumber),
                    owner(owner_)
                {
                    createPipe("JUCEDirect2DMetricsHub:" + owner.getProcessString(), -1, true);
                }

                ~HubPipeServer() override
                {
                    disconnect();
                }

                void connectionMade() override
                {
                }

                void connectionLost() override
                {
                }

                void messageReceived(const MemoryBlock& message) override;

                MetricsHub& owner;
            };

            HubPipeServer hubPipeServer{ *this };
            ReferenceCountedArray<Metrics> metricsArray;
            Metrics* lastMetrics = nullptr;
        };
    }

}

#define JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, name) juce::direct2d::ScopedElapsedTime scopedElapsedTime_##name{ metrics, juce::direct2d::Metrics::name };

#else

namespace juce
{
    namespace direct2d
    {
        struct Metrics : public ReferenceCountedObject
        {
            using Ptr = ReferenceCountedObjectPtr<Metrics>;
        };
    }
}

#define JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, name)

#endif
