#ifndef SGWC_METRICS_H
#define SGWC_METRICS_H

#include "ogs-metrics.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* GLOBAL */
    typedef enum sgw_metric_type_global_s
    {
        SGWC_METR_GLOB_CTR_SM_CREATIONDEFAULTBEARERS11ATT = 0,
        SGWC_METR_GLOB_CTR_SM_CREATIONDEFAULTBEARERS11SUCC,
        SGWC_METR_GLOB_CTR_SM_CREATIONDEDICATEDBEARERS11ATT,
        SGWC_METR_GLOB_CTR_SM_CREATIONDEDICATEDBEARERS11SUCC,

        SGWC_METR_GLOB_CTR_SM_ESTABLISHPFCPSESSIONATT,
        SGWC_METR_GLOB_CTR_SM_ESTABLISHPFCPSESSIONSUCC,
        SGWC_METR_GLOB_CTR_SM_ESTABLISHPFCPSESSIONFAIL,

        SGWC_METR_GLOB_CTR_SM_MODIFYPFCPSESSIONATT,
        SGWC_METR_GLOB_CTR_SM_MODIFYPFCPSESSIONSUCC,
        SGWC_METR_GLOB_CTR_SM_MODIFYPFCPSESSIONFAIL,

        SGWC_METR_GLOB_CTR_SM_DELETIONPFCPSESSIONATT,
        SGWC_METR_GLOB_CTR_SM_DELETIONPFCPSESSIONSUCC,
        SGWC_METR_GLOB_CTR_SM_DELETIONPFCPSESSIONFAIL,

        SGWC_METR_GLOB_GAUGE_SGWC_SESSIONNBR,

        _SGWC_METR_GLOB_MAX,
    } sgw_metric_type_global_t;
    extern ogs_metrics_inst_t *sgwc_metrics_inst_global[_SGWC_METR_GLOB_MAX];

    int sgwc_metrics_init_inst_global(void);
    int sgwc_metrics_free_inst_global(void);

    static inline void sgwc_metrics_inst_global_set(sgw_metric_type_global_t t, int val)
    {
        ogs_metrics_inst_set(sgwc_metrics_inst_global[t], val);
    }
    static inline void sgwc_metrics_inst_global_add(sgw_metric_type_global_t t, int val)
    {
        ogs_metrics_inst_add(sgwc_metrics_inst_global[t], val);
    }
    static inline void sgwc_metrics_inst_global_inc(sgw_metric_type_global_t t)
    {
        ogs_metrics_inst_inc(sgwc_metrics_inst_global[t]);
    }
    static inline void sgwc_metrics_inst_global_dec(sgw_metric_type_global_t t)
    {
        ogs_metrics_inst_dec(sgwc_metrics_inst_global[t]);
    }

    int sgwc_metrics_open(void);
    int sgwc_metrics_close(void);

#ifdef __cplusplus
}
#endif

#endif /* SGWC_METRICS_H */
