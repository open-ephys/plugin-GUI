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

#if JUCE_DIRECT2D_METRICS

namespace juce
{
    namespace direct2d
    {

        String MetricsHub::getProcessString() noexcept
        {
            auto processID = GetCurrentProcessId();
            return String::toHexString((pointer_sized_int)processID);
        }

        void MetricsHub::HubPipeServer::messageReceived(const MemoryBlock& message)
        {
            int requestType = *(int*)message.getData();
            switch (requestType)
            {
            case getValuesRequest:
            {
                ScopedLock locker{ owner.lock };

                auto foregroundWindow = GetForegroundWindow();
                Metrics::Ptr metrics = nullptr;
                for (int i = 0; i < owner.metricsArray.size(); ++i)
                {
                    auto arrayEntry = owner.metricsArray[i];
                    if (arrayEntry->windowHandle && arrayEntry->windowHandle == foregroundWindow)
                    {
                        metrics = arrayEntry;
                        break;
                    }
                }

                if (!metrics)
                {
                    if (owner.lastMetrics && owner.metricsArray.contains(owner.lastMetrics))
                    {
                        metrics = owner.lastMetrics;
                    }
                }

                if (metrics)
                {
                    MemoryBlock block{ sizeof(GetValuesResponse), true };

                    auto* response = (GetValuesResponse*)block.getData();
                    response->responseType = getValuesRequest;
                    response->windowHandle = metrics->windowHandle;

                    for (size_t i = 0; i <= Metrics::drawGlyphRunTime; ++i)
                    {
                        auto& accumulator = metrics->getAccumulator(i);
                        response->values[i].count = accumulator.getCount();
                        response->values[i].total = metrics->getSum(i);
                        response->values[i].average = accumulator.getAverage();
                        response->values[i].minimum = accumulator.getMinValue();
                        response->values[i].maximum = accumulator.getMaxValue();
                        response->values[i].stdDev = accumulator.getStandardDeviation();
                    }

                    //
                    // Track bitmap operations common to all device contexts
                    //
                    for (size_t i = Metrics::createBitmapTime; i <= Metrics::unmapBitmapTime; ++i)
                    {
                        auto& accumulator = owner.imageContextMetrics->getAccumulator(i);
                        response->values[i].count = accumulator.getCount();
                        response->values[i].total = metrics->getSum(i);
                        response->values[i].average = accumulator.getAverage();
                        response->values[i].minimum = accumulator.getMinValue();
                        response->values[i].maximum = accumulator.getMaxValue();
                        response->values[i].stdDev = accumulator.getStandardDeviation();
                    }

                    sendMessage(block);

                    owner.lastMetrics = metrics.get();
                }
                break;
            }

            case resetValuesRequest:
            {
                owner.resetAll();
                break;
            }
            }
        }

        void MetricsHub::resetAll()
        {
            ScopedLock locker{ lock };

            imageContextMetrics->reset();
            for (auto metrics : metricsArray)
            {
                metrics->reset();
            }
        }

    }

} // namespace juce

#endif
