import * as React from "react";
import { Select, Spinner, Tag, TagDismissData, TagDismissEvent, TagGroup, Toolbar, ToolbarButton, ToolbarDivider, makeStyles, tokens } from "@fluentui/react-components";
import { ArrowClockwise20Regular } from "@fluentui/react-icons";
import { useConst } from "@fluentui/react-hooks";
import { WorkunitsService, WsWorkunits } from "@hpcc-js/comms";
import { Area, Column, Pie, Bar } from "@hpcc-js/chart";
import { GroupRow, filter, group, map, pipe, sort } from "@hpcc-js/dataflow";
import nlsHPCC from "src/nlsHPCC";
import { wuidToDate } from "src/Utility";
import { CreateWUQueryStore } from "../comms/workunit";
import { AutosizeHpccJSComponent } from "../layouts/HpccJSAdapter";
import { DockPanel, DockPanelItem } from "../layouts/DockPanel";
import { HolyGrail } from "../layouts/HolyGrail";
import { pushParamExact } from "../util/history";
import { Workunits } from "./Workunits";

const DEFAULT_LASTNDAYS = 7;

const useStyles = makeStyles({
    loadingOverlay: {
        position: "absolute",
        inset: 0,
        display: "flex",
        alignItems: "center",
        justifyContent: "center",
        backgroundColor: tokens.colorNeutralBackgroundAlpha,
        zIndex: 1,
    },
    lastNDaysSelect: {
        minWidth: "120px",
        "> select": {
            paddingLeft: tokens.spacingHorizontalXS,
        },
    },
});

const service = new WorkunitsService({ baseUrl: "" });

interface WorkunitEx extends WsWorkunits.ECLWorkunit {
    Day: string;
}

export interface WorkunitsDashboardFilter {
    lastNDays?: number;
    cluster?: string;
    owner?: string;
    state?: string;
    protected?: boolean;
    day?: string;
}

export interface WorkunitsDashboardProps {
    filterProps?: WorkunitsDashboardFilter;
}

export const WorkunitsDashboard: React.FunctionComponent<WorkunitsDashboardProps> = ({
    filterProps
}) => {
    filterProps = {
        lastNDays: DEFAULT_LASTNDAYS,
        ...filterProps
    };

    const styles = useStyles();

    const [loading, setLoading] = React.useState(false);
    const [workunits, setWorkunits] = React.useState<WorkunitEx[]>([]);
    const [refreshToken, setRefreshToken] = React.useState(0);

    React.useEffect(() => {
        setLoading(true);
        setWorkunits([]);
        const end = new Date();
        const start = new Date();
        start.setDate(start.getDate() - filterProps.lastNDays);
        service.WUQuery({
            StartDate: start.toISOString(),
            EndDate: end.toISOString(),
            PageSize: 999999
        }).then(response => {
            setWorkunits([...map(response.Workunits.ECLWorkunit, (row: WsWorkunits.ECLWorkunit) => ({ ...row, Day: wuidToDate(row.Wuid) }))]);
            setLoading(false);
        });
    }, [filterProps.lastNDays, refreshToken]);

    //  Cluster Chart ---
    const clusterChart = useConst(() =>
        new Bar()
            .columns(["Cluster", "Count"])
            .on("click", (row, col, sel) => pushParamExact("cluster", sel ? row.Cluster : undefined))
    );

    React.useEffect(() => {
        const pipeline = pipe(
            filter<WorkunitEx>(row => filterProps.state === undefined || row.State === filterProps.state),
            filter(row => filterProps.owner === undefined || row.Owner === filterProps.owner),
            filter(row => filterProps.day === undefined || row.Day === filterProps.day),
            filter(row => filterProps.protected === undefined || row.Protected === filterProps.protected),
            group<WorkunitEx>(row => row.Cluster),
            map<GroupRow<WorkunitEx>, [string, number]>(row => [row.key, row.value.length]),
            sort((l, r) => l[0].localeCompare(r[0])),
        );
        clusterChart.data([...pipeline(workunits)]).lazyRender();
    }, [clusterChart, workunits, filterProps.state, filterProps.owner, filterProps.day, filterProps.protected]);

    //  Owner Chart ---
    const ownerChart = useConst(() =>
        new Column()
            .columns(["Owner", "Count"])
            .on("click", (row, col, sel) => pushParamExact("owner", sel ? row.Owner : undefined))
    );

    React.useEffect(() => {
        const pipeline = pipe(
            filter<WorkunitEx>(row => filterProps.cluster === undefined || row.Cluster === filterProps.cluster),
            filter(row => filterProps.state === undefined || row.State === filterProps.state),
            filter(row => filterProps.day === undefined || row.Day === filterProps.day),
            filter(row => filterProps.protected === undefined || row.Protected === filterProps.protected),
            group<WorkunitEx>(row => row.Owner),
            map<GroupRow<WorkunitEx>, [string, number]>(row => [row.key, row.value.length]),
            sort((l, r) => l[0].localeCompare(r[0])),
        );
        ownerChart.data([...pipeline(workunits)]).lazyRender();
    }, [ownerChart, workunits, filterProps.cluster, filterProps.state, filterProps.day, filterProps.protected]);

    //  State Chart ---
    const stateChart = useConst(() =>
        new Pie()
            .columns(["State", "Count"])
            .on("click", (row, col, sel) => pushParamExact("state", sel ? row.State : undefined))
    );

    React.useEffect(() => {
        const pipeline = pipe(
            filter<WorkunitEx>(row => filterProps.cluster === undefined || row.Cluster === filterProps.cluster),
            filter(row => filterProps.owner === undefined || row.Owner === filterProps.owner),
            filter(row => filterProps.day === undefined || row.Day === filterProps.day),
            filter(row => filterProps.protected === undefined || row.Protected === filterProps.protected),
            group<WorkunitEx>(row => row.State),
            map<GroupRow<WorkunitEx>, [string, number]>(row => [row.key, row.value.length]),
        );
        stateChart.data([...pipeline(workunits)]).lazyRender();
    }, [stateChart, workunits, filterProps.cluster, filterProps.owner, filterProps.day, filterProps.protected]);

    //  Protected Chart ---
    const protectedChart = useConst(() =>
        new Pie()
            .columns(["Protected", "Count"])
            .on("click", (row, col, sel) => pushParamExact("protected", sel ? row.Protected === "true" : undefined))
    );

    React.useEffect(() => {
        const pipeline = pipe(
            filter<WorkunitEx>(row => filterProps.cluster === undefined || row.Cluster === filterProps.cluster),
            filter(row => filterProps.owner === undefined || row.Owner === filterProps.owner),
            filter(row => filterProps.day === undefined || row.Day === filterProps.day),
            group<WorkunitEx>(row => "" + row.Protected),
            map<GroupRow<WorkunitEx>, [string, number]>(row => [row.key, row.value.length]),
        );
        protectedChart.data([...pipeline(workunits)]).lazyRender();
    }, [protectedChart, workunits, filterProps.cluster, filterProps.owner, filterProps.day]);

    //  Day Chart ---
    const dayChart = useConst(() =>
        new Area()
            .columns(["Day", "Count"])
            .xAxisType("time")
            .interpolate("cardinal")
            // .xAxisTypeTimePattern("")
            .on("click", (row, col, sel) => pushParamExact("day", sel ? row.Day : undefined))
    );

    React.useEffect(() => {
        const pipeline = pipe(
            filter<WorkunitEx>(row => filterProps.cluster === undefined || row.Cluster === filterProps.cluster),
            filter(row => filterProps.owner === undefined || row.Owner === filterProps.owner),
            filter(row => filterProps.state === undefined || row.State === filterProps.state),
            filter(row => filterProps.protected === undefined || row.Protected === filterProps.protected),
            group<WorkunitEx>(row => row.Day),
            map<GroupRow<WorkunitEx>, [string, number]>(row => [row.key, row.value.length]),
            sort((l, r) => l[0].localeCompare(r[0])),
        );
        dayChart.data([...pipeline(workunits)]).lazyRender();
    }, [dayChart, workunits, filterProps.cluster, filterProps.owner, filterProps.state, filterProps.protected]);

    //  Table ---
    const workunitsStore = useConst(() => CreateWUQueryStore());

    const workunitsFilter = React.useMemo(() => {
        let start = new Date();
        let end = new Date();

        if (filterProps.day) {
            start = new Date(filterProps.day);
            end = new Date(filterProps.day);
            end.setDate(end.getDate() + 1);
        } else {
            const lastNDays = filterProps.lastNDays ?? DEFAULT_LASTNDAYS;
            start.setDate(start.getDate() - lastNDays);
        }

        return {
            StartDate: start.toISOString(),
            EndDate: end.toISOString(),
            ...(filterProps.cluster && { Cluster: filterProps.cluster }),
            ...(filterProps.owner && { Owner: filterProps.owner }),
            ...(filterProps.state && { State: filterProps.state }),
            ...(filterProps.protected !== undefined && { Protected: filterProps.protected }),
        };
    }, [filterProps.cluster, filterProps.day, filterProps.lastNDays, filterProps.owner, filterProps.protected, filterProps.state]);

    function removeItem(e: TagDismissEvent, data: TagDismissData<string>): void {
        switch (data.value) {
            case "state":
                pushParamExact("state", undefined);
                break;
            case "day":
                pushParamExact("day", undefined);
                break;
            case "protected":
                pushParamExact("protected", undefined);
                break;
            case "owner":
                pushParamExact("owner", undefined);
                break;
            case "cluster":
                pushParamExact("cluster", undefined);
                break;
        }
    }

    return <HolyGrail
        header={
            <Toolbar>
                <ToolbarButton appearance="subtle" icon={<ArrowClockwise20Regular />} aria-label={nlsHPCC.Refresh} onClick={() => setRefreshToken(t => t + 1)}>
                    {nlsHPCC.Refresh}
                </ToolbarButton>
                <ToolbarDivider />
                <Select className={styles.lastNDaysSelect} appearance="underline" value={filterProps.lastNDays} onChange={(evt, data) => { pushParamExact("lastNDays", Number(data.value)); }}>
                    <option value={1}>1 Day</option>
                    <option value={2}>2 Days</option>
                    <option value={3}>3 Days</option>
                    <option value={7}>1 Week</option>
                    <option value={14}>2 Weeks</option>
                    <option value={21}>3 Weeks</option>
                    <option value={31}>1 Month</option>
                </Select>
                <ToolbarDivider />
                <TagGroup onDismiss={removeItem}>
                    {filterProps.state !== undefined && <Tag key="state" dismissible value="state" size="small">{filterProps.state}</Tag>}
                    {filterProps.day !== undefined && <Tag key="day" dismissible value="day" size="small">{filterProps.day}</Tag>}
                    {filterProps.protected !== undefined && <Tag key="protected" dismissible value="protected" size="small">{"" + filterProps.protected}</Tag>}
                    {filterProps.owner !== undefined && <Tag key="owner" dismissible value="owner" size="small">{filterProps.owner}</Tag>}
                    {filterProps.cluster !== undefined && <Tag key="cluster" dismissible value="cluster" size="small">{filterProps.cluster}</Tag>}
                </TagGroup>
            </Toolbar>
        }
        main={
            <div style={{ position: "relative", width: "100%", height: "100%" }}>
                <DockPanel hideSingleTabs={false}>
                    <DockPanelItem key="stateChart" title={nlsHPCC.State}>
                        <AutosizeHpccJSComponent widget={stateChart} />
                    </DockPanelItem>
                    <DockPanelItem key="ownerChart" title={nlsHPCC.Owner} location="split-bottom" relativeTo="stateChart">
                        <AutosizeHpccJSComponent widget={ownerChart} />
                    </DockPanelItem>
                    <DockPanelItem key="workunitsTable" title={nlsHPCC.Workunits} location="split-bottom" relativeTo="ownerChart">
                        <Workunits filter={workunitsFilter} store={workunitsStore} />
                    </DockPanelItem>
                    <DockPanelItem key="dayChart" title={nlsHPCC.Day} location="split-right" relativeTo="stateChart">
                        <AutosizeHpccJSComponent widget={dayChart} />
                    </DockPanelItem>
                    <DockPanelItem key="protectedChart" title={nlsHPCC.Protected} location="split-right" relativeTo="dayChart">
                        <AutosizeHpccJSComponent widget={protectedChart} />
                    </DockPanelItem>
                    <DockPanelItem key="clusterChart" title={nlsHPCC.Cluster} location="split-right" relativeTo="ownerChart">
                        <AutosizeHpccJSComponent widget={clusterChart} />
                    </DockPanelItem>
                </DockPanel>
                {loading && <div className={styles.loadingOverlay}>
                    <Spinner label={nlsHPCC.Loading} size="large" />
                </div>}
            </div>
        }
    />;
};