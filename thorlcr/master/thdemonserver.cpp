/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems®.

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

#include "thdemonserver.hpp"

#include "jmutex.hpp"
#include "jmisc.hpp"
#include "daaudit.hpp"
#include "daclient.hpp"

#include "thormisc.hpp"
#include "thorport.hpp"
#include "thcompressutil.hpp"
#include "thgraphmaster.ipp"
#include "thgraphmanager.hpp"
#include "thwatchdog.hpp"


class DeMonServer : public CSimpleInterface, implements IDeMonServer
{
private:
    Mutex mutex;
    unsigned lastReport;
    unsigned reportRate;
    CIArrayOf<CGraphBase> activeGraphs;
    UnsignedArray graphStarts;
    double thorManagerRate = 0;
    double thorWorkerRate = 0;
    cost_type costLimit = 0;
    cost_type workunitCost = 0;
    StatisticsAggregator statsAggregator;

    void doReportGraph(IStatisticGatherer & stats, CGraphBase *graph)
    {
        ((CMasterGraph *)graph)->getStats(stats);
    }
    void reportGraphContents(IStatisticGatherer & stats, CGraphBase *graph)
    {
        if (graph->hasProgress()) // if there have ever been any progress, ensure they are republished
            doReportGraph(stats, graph);
        Owned<IThorGraphIterator> graphIter = graph->getChildGraphIterator();
        ForEach (*graphIter)
            reportGraph(stats, &graphIter->query());
    }
    void reportGraph(IStatisticGatherer & stats, CGraphBase *graph)
    {
        try
        {
            if (graph->queryParentActivityId() && !graph->containsActivities())
            {
                StatsActivityScope activity(stats, graph->queryParentActivityId());
                StatsChildGraphScope subgraph(stats, graph->queryGraphId());
                reportGraphContents(stats, graph);
            }
            else
            {
                StatsSubgraphScope subgraph(stats, graph->queryGraphId());
                reportGraphContents(stats, graph);
            }
        }
        catch (IException *e)
        {
            StringBuffer s;
            IWARNLOG("Failed to update progress information: %s", e->errorMessage(s).str());
            e->Release();
        }
    }
    void reportStatus(IWorkUnit *wu, CGraphBase &graph, unsigned startTime, bool finished, bool success=true)
    {
        const char *graphname = graph.queryJob().queryGraphName();
        unsigned wfid = graph.queryJob().getWfid();
        StringBuffer timer, graphScope;
        formatGraphTimerLabel(timer, graphname, 0, graph.queryGraphId());
        formatGraphTimerScope(graphScope, wfid, graphname, 0, graph.queryGraphId());
        unsigned duration = msTick()-startTime;
        updateWorkunitStat(wu, SSTsubgraph, graphScope, StTimeElapsed, timer, milliToNano(duration));
        if (costLimit || finished)
        {
            const cost_type sgCost = money2cost_type(calcCost(thorManagerRate, duration) + calcCost(thorWorkerRate, duration) * queryNodeClusterWidth());
            if (finished)
            {
                if (sgCost)
                    wu->setStatistic(queryStatisticsComponentType(), queryStatisticsComponentName(), SSTsubgraph, graphScope, StCostExecute, NULL, sgCost, 1, 0, StatsMergeReplace);
            }

            const cost_type totalCost = workunitCost + sgCost + graph.getDiskAccessCost();

            if (costLimit>0 && totalCost > costLimit)
            {
                WARNLOG("ABORT job cost exceeds limit");
                graph.fireException(MakeThorException(TE_CostExceeded, "Job cost exceeds limit"));
            }
        }
        if (finished)
        {
            if (memcmp(graphname,"graph",5)==0)
                graphname+=5;
            LOG(MCauditInfo,",Timing,ThorGraph,%s,%s,%s,%u,1,%d,%s,%s,%s",
                queryServerStatus().queryProperties()->queryProp("@thorname"),
                wu->queryWuid(),
                graphname,
                (unsigned)graph.queryGraphId(),
                duration,
                success?"SUCCESS":"FAILED",
                queryServerStatus().queryProperties()->queryProp("@nodeGroup"),
                queryServerStatus().queryProperties()->queryProp("@queue"));
            queryServerStatus().queryProperties()->removeProp("@graph");
            queryServerStatus().queryProperties()->removeProp("@subgraph");
            queryServerStatus().queryProperties()->removeProp("@sg_duration");
        }
        else
        {
            queryServerStatus().queryProperties()->setProp("@graph", graph.queryJob().queryGraphName());
            queryServerStatus().queryProperties()->setPropInt("@subgraph", (int)graph.queryGraphId());
            queryServerStatus().queryProperties()->setPropInt("@sg_duration", (duration+59999)/60000); // round it up
        }
    }
    void updateGraphStats(IConstWorkUnit &currentWU, const char *graphName, unsigned wfid, CGraphBase & graph)
    {
        Owned<IWUGraphStats> stats = currentWU.updateStats(graphName, SCTthor, queryStatisticsComponentName(), wfid, graph.queryGraphId(), false);
        IStatisticGatherer & statsBuilder = stats->queryStatsBuilder();
        reportGraph(statsBuilder, &graph);
        // Merge only the stats at the specified scope level
        Owned<IStatisticCollection> statsCollection = statsBuilder.getResult();
        statsAggregator.recordStats(statsCollection, wfid, graphName, graph.queryGraphId());
    }
    void reportActiveGraphs(bool finished, bool success=true)
    {
        if (activeGraphs.ordinality())
        {
            try
            {
                CJobBase & activeJob = activeGraphs.item(0).queryJob();
                IConstWorkUnit &currentWU = activeJob.queryWorkUnit();
                const char *graphName = ((CJobMaster &)activeJob).queryGraphName();
                unsigned wfid = activeJob.getWfid();
                ForEachItemIn (g, activeGraphs)
                {
                    CGraphBase &graph = activeGraphs.item(g);
                    updateGraphStats(currentWU, graphName, wfid, graph);
                }
                Owned<IWorkUnit> wu = &currentWU.lock();
                ForEachItemIn (g2, activeGraphs)
                {
                    CGraphBase &graph = activeGraphs.item(g2);
                    unsigned startTime = graphStarts.item(g2);
                    reportStatus(wu, graph, startTime, finished, success);
                }
                updateAggregates(wu);
                queryServerStatus().commitProperties();
            }
            catch (IException *E)
            {
                IWARNLOG(E, "Failed to update progress information");
                E->Release();
            }
        }
    }
    void reportGraph(CGraphBase *graph, bool finished, bool success, unsigned startTime, unsigned __int64 startTimeStamp)
    {
        try
        {
            IConstWorkUnit &currentWU = graph->queryJob().queryWorkUnit();
            const char *graphName = ((CJobMaster &)activeGraphs.item(0).queryJob()).queryGraphName();
            unsigned wfid = graph->queryJob().getWfid();
            updateGraphStats(currentWU, graphName, wfid, *graph);

            Owned<IWorkUnit> wu = &currentWU.lock();
            if (startTimeStamp)
            {
                StringBuffer graphScope;
                const char *graphname = graph->queryJob().queryGraphName();
                formatGraphTimerScope(graphScope, wfid, graphname, 0, graph->queryGraphId());
                wu->setStatistic(queryStatisticsComponentType(), queryStatisticsComponentName(), SSTsubgraph, graphScope, StWhenStarted, NULL, getTimeStampNowValue(), 1, 0, StatsMergeAppend);
            }
            reportStatus(wu, *graph, startTime, finished, success);
            queryServerStatus().commitProperties();
        }
        catch (IException *e)
        {
            StringBuffer s;
            IWARNLOG(e, "Failed to update progress information");
            e->Release();
        }
    }
public:
    IMPLEMENT_IINTERFACE_USING(CSimpleInterface);

    DeMonServer() : statsAggregator(stdAggregateKindStatistics)
    {
        lastReport = msTick();
        reportRate = globals->getPropInt("@watchdogProgressInterval", 30);
        thorManagerRate = getThorManagerRate();
        thorWorkerRate = getThorWorkerRate();
    }

    virtual void takeHeartBeat(MemoryBuffer &progressMb)
    {
        synchronized block(mutex);
        if (0 == activeGraphs.ordinality())
        {
            StringBuffer urlStr;
            IWARNLOG("heartbeat packet received with no active graphs");
            return;
        }
        size32_t compressedProgressSz = progressMb.remaining();
        if (compressedProgressSz)
        {
            MemoryBuffer uncompressedMb;
            ThorExpand(progressMb.readDirect(compressedProgressSz), compressedProgressSz, uncompressedMb);
            do
            {
                graph_id graphId;
                unsigned slave;
                uncompressedMb.read(slave);
                uncompressedMb.read(graphId);
                CMasterGraph *graph = NULL;
                ForEachItemIn(g, activeGraphs) if (activeGraphs.item(g).queryGraphId() == graphId) graph = (CMasterGraph *)&activeGraphs.item(g);
                if (!graph)
                {
                    IWARNLOG("heartbeat received from unknown graph %" GIDPF "d", graphId);
                    break;
                }
                if (!graph->deserializeStats(slave, uncompressedMb))
                {
                    IWARNLOG("heartbeat error in graph %" GIDPF "d", graphId);
                    break;
                }
            }
            while (uncompressedMb.remaining());
        }
        unsigned now=msTick();
        if (now-lastReport > 1000*reportRate)
        {
            reportActiveGraphs(false);
            lastReport = msTick();
        }
    }
    void startGraph(CGraphBase *graph)
    {
        synchronized block(mutex);

        IConstWorkUnit & wu =  graph->queryJob().queryWorkUnit();
        workunitCost = aggregateCost(&wu);

        costLimit = getGuillotineCost(&wu);
        activeGraphs.append(*LINK(graph));
        unsigned startTime = msTick();
        graphStarts.append(startTime);
        reportGraph(graph, false, true, startTime, getTimeStampNowValue());
    }
    void endGraph(CGraphBase *graph, bool success)
    {
        synchronized block(mutex);
        unsigned g = activeGraphs.find(*graph);
        if (NotFound != g)
        {
            unsigned startTime = graphStarts.item(g);
            reportGraph(graph, true, success, startTime, 0);
            activeGraphs.remove(g);
            graphStarts.remove(g);
        }
    }
    void endGraphs()
    {
        synchronized block(mutex);
        reportActiveGraphs(true, false);
        activeGraphs.kill();
    }
    virtual void updateAggregates(IWorkUnit * lockedWu) override
    {
        statsAggregator.updateAggregates(lockedWu);
    }
    virtual void loadExistingAggregates(IConstWorkUnit &workunit) override
    {
        statsAggregator.loadExistingAggregates(workunit);
    }
};


IDeMonServer *createDeMonServer()
{
    return new DeMonServer();
}
