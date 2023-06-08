#ifndef SGWU_METRICS_H
#define SGWU_METRICS_H

#include "ogs-metrics.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* GLOBAL */
    typedef enum sgw_metric_type_global_s
    {
        SGWU_METR_GLOB_GAUGE_GTP_OUTDATAOCTS1USGW,
        SGWU_METR_GLOB_GAUGE_GTP_INDATAOCTS1USGW,
        SGWU_METR_GLOB_GAUGE_SGWU_SESSIONNBR,

        _SGWU_METR_GLOB_MAX,
    } sgw_metric_type_global_t;
    extern ogs_metrics_inst_t *sgwu_metrics_inst_global[_SGWU_METR_GLOB_MAX];

    int sgwu_metrics_init_inst_global(void);
    int sgwu_metrics_free_inst_global(void);

    static inline void sgwu_metrics_inst_global_set(sgw_metric_type_global_t t, int val)
    {
        ogs_metrics_inst_set(sgwu_metrics_inst_global[t], val);
    }

    static inline void sgwu_metrics_inst_global_add(sgw_metric_type_global_t t, int val)
    {
        ogs_metrics_inst_add(sgwu_metrics_inst_global[t], val);
    }

    static inline void sgwu_metrics_inst_global_inc(sgw_metric_type_global_t t)
    {
        ogs_metrics_inst_inc(sgwu_metrics_inst_global[t]);
    }
    
    static inline void sgwu_metrics_inst_global_dec(sgw_metric_type_global_t t)
    {
        ogs_metrics_inst_dec(sgwu_metrics_inst_global[t]);
    }

    int sgwu_metrics_open(void);
    int sgwu_metrics_close(void);

#ifdef __cplusplus
}
#endif

#endif /* SGWU_METRICS_H */
