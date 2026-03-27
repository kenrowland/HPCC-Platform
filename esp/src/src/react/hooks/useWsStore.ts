import { useEffect, useState } from "react";
import { getRecentFilters } from "../../KeyValStore";

export const useGet = (key: string, filter?: object) => {
    const [loading, setLoading] = useState(true);
    const [data, setData] = useState(null);
    const [error, setError] = useState<unknown>(undefined);
    useEffect(() => {
        let cancelled = false;
        getRecentFilters(key).then(response => {
            if (!cancelled) {
                setData(response);
                setError(undefined);
            }
        }).catch((e) => {
            if (!cancelled) {
                setData(null);
                setError(e);
            }
        }).finally(() => {
            if (!cancelled) {
                setLoading(false);
            }
        });

        return () => {
            cancelled = true;
        };
    }, [key, filter]);
    return { data, loading, error };
};
