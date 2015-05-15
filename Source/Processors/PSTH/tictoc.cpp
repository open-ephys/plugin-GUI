#include "tictoc.h"
#include <algorithm>
std::vector<int> TicToc::sort_indexes(std::vector<double> v)
{
    // initialize original index locations
    std::vector<int> idx(v.size());

    for (int i = 0; i != idx.size(); ++i)
    {
        idx[i] = i;
    }

    // sort indexes based on comparing values in v
    sort(
        idx.begin(),
        idx.end(),
        [&v](size_t i1, size_t i2)
    {
        return v[i1] > v[i2];
    }
    );

    return idx;
}


void TicToc::print()
{
    // sort by total time spent.
    std::vector<int> sortIndByTotalTime = sort_indexes(totalTime);
    std::vector<int> sortIndByAverageTime = sort_indexes(averageTime);
    printf("\n\n\nSorted by total time:\n");
    for (int k=0; k<N; k++)
    {
        int ind = sortIndByTotalTime[k];
        if (numSamples[ind] > 0)
        {
            printf("Timer %d, total time: %.3f sec, or %.3f ms, num calls: %d\n",ind,totalTime[ind],totalTime[ind]*1000,numSamples[ind]);
        }
    }
    printf("/***************************************************/\nSorted by average time:\n");
    for (int k=0; k<N; k++)
    {
        int ind = sortIndByAverageTime[k];
        if (numSamples[ind] > 0)
        {
            printf("Timer %d, average time: %.3f sec, or %.3f ms, num calls: %d\n",ind,averageTime[ind],averageTime[ind]*1000,numSamples[ind]);
        }
    }
}

TicToc::TicToc()
{
    N = 100;
    tics.resize(N);
    tocs.resize(N);
    averageTime.resize(N);
    numSamples.resize(N);
    totalTime.resize(N);
    for (int k=0; k<N; k++)
    {
        totalTime[k] = numSamples[k] = averageTime[k] = tics[k] =tocs[k] =  0;
    }
}


void TicToc::clear()
{
    for (int k=0; k<N; k++)
    {
        totalTime[k] = numSamples[k] = averageTime[k] = tics[k] =tocs[k] =  0;
    }
}

void TicToc::Tic(int x)
{
    Time t;
    tics[x] = double(t.getHighResolutionTicks()) / double(t.getHighResolutionTicksPerSecond());
}

void TicToc::Toc(int x)
{
    Time t;
    tocs[x] = double(t.getHighResolutionTicks()) / double(t.getHighResolutionTicksPerSecond());
    double TimeDiff = tocs[x]-tics[x];
    totalTime[x] +=TimeDiff;
    numSamples[x]++;
    averageTime[x] = ((numSamples[x] - 1) * averageTime[x] + TimeDiff) / numSamples[x];
}

