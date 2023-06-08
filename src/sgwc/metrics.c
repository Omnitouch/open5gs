#include "ogs-app.h"
#include "context.h"

#include "metrics.h"

typedef struct sgwc_metrics_spec_def_s
{
    unsigned int type;
    const char *name;
    const char *description;
    int initial_val;
    unsigned int num_labels;
    const char **labels;
} sgwc_metrics_spec_def_t;

/* Helper generic functions: */
static int sgwc_metrics_init_inst(ogs_metrics_inst_t **inst,
                                  ogs_metrics_spec_t **specs, unsigned int len,
                                  unsigned int num_labels, const char **labels)
{
    unsigned int i;
    for (i = 0; i < len; i++)
        inst[i] = ogs_metrics_inst_new(specs[i], num_labels, labels);
    return OGS_OK;
}

static int sgwc_metrics_free_inst(ogs_metrics_inst_t **inst,
                                  unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
        ogs_metrics_inst_free(inst[i]);
    memset(inst, 0, sizeof(inst[0]) * len);
    return OGS_OK;
}

static int sgwc_metrics_init_spec(ogs_metrics_context_t *ctx,
                                  ogs_metrics_spec_t **dst, sgwc_metrics_spec_def_t *src, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
    {
        dst[i] = ogs_metrics_spec_new(ctx, src[i].type,
                                      src[i].name, src[i].description,
                                      src[i].initial_val, src[i].num_labels, src[i].labels,
                                      NULL);
    }
    return OGS_OK;
}

/* GLOBAL */
ogs_metrics_spec_t *sgwc_metrics_spec_global[_SGWC_METR_GLOB_MAX];
ogs_metrics_inst_t *sgwc_metrics_inst_global[_SGWC_METR_GLOB_MAX];
sgwc_metrics_spec_def_t sgwc_metrics_spec_def_global[_SGWC_METR_GLOB_MAX] = {
    /* Global Counters: */
    /* Bearers */
    [SGWC_METR_GLOB_CTR_SM_CREATIONDEFAULTBEARERS11ATT] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_servinggwfunction_sm_creationdefaultbearers11att",
        .description = "Number of attempted default bearer creation over S11",
    },
    [SGWC_METR_GLOB_CTR_SM_CREATIONDEFAULTBEARERS11SUCC] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_servinggwfunction_sm_creationdefaultbearers11succ",
        .description = "Number of successful default bearer creation over S11",
    },
    [SGWC_METR_GLOB_CTR_SM_CREATIONDEDICATEDBEARERS11ATT] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_servinggwfunction_sm_creationdedicatedbearers11att",
        .description = "Number of attempted dedicated bearer creation over S11",
    },
    [SGWC_METR_GLOB_CTR_SM_CREATIONDEDICATEDBEARERS11SUCC] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_servinggwfunction_sm_creationdedicatedbearers11succ",
        .description = "Number of successful dedicated bearer creation over S11",
    },
    
    /* PFCP */
    [SGWC_METR_GLOB_CTR_SM_ESTABLISHPFCPSESSIONATT] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_sgwcfunction_sm_establishpfcpsessionatt",
        .description = "Number of attempted PFCP session establishment",
    },
    [SGWC_METR_GLOB_CTR_SM_ESTABLISHPFCPSESSIONSUCC] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_sgwcfunction_sm_establishpfcpsessionsucc",
        .description = "Number of successful PFCP session establishment",
    },
    [SGWC_METR_GLOB_CTR_SM_ESTABLISHPFCPSESSIONFAIL] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_sgwcfunction_sm_establishpfcpsessionfail",
        .description = "Number of failed PFCP session establishment",
    },
    [SGWC_METR_GLOB_CTR_SM_DELETIONPFCPSESSIONATT] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_sgwcfunction_sm_deletionpfcpsessionatt",
        .description = "Number of attempted PFCP session deletion",
    },
    [SGWC_METR_GLOB_CTR_SM_DELETIONPFCPSESSIONSUCC] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_sgwcfunction_sm_deletionpfcpsessionsucc",
        .description = "Number of successful PFCP session deletion",
    },
    [SGWC_METR_GLOB_CTR_SM_DELETIONPFCPSESSIONFAIL] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_sgwcfunction_sm_deletionpfcpsessionfail",
        .description = "Number of failed PFCP session deletion",
    },
    [SGWC_METR_GLOB_CTR_SM_MODIFYPFCPSESSIONATT] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_sgwcfunction_sm_modifypfcpsessionatt",
        .description = "Number of attempted PFCP session modify",
    },
    [SGWC_METR_GLOB_CTR_SM_MODIFYPFCPSESSIONSUCC] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_sgwcfunction_sm_modifypfcpsessionsucc",
        .description = "Number of successful PFCP session modify",
    },
    [SGWC_METR_GLOB_CTR_SM_MODIFYPFCPSESSIONFAIL] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_sgwcfunction_sm_modifypfcpsessionfail",
        .description = "Number of failed PFCP session modify",
    },

    /* Session */
    [SGWC_METR_GLOB_GAUGE_SGWC_SESSIONNBR] = {
        .type = OGS_METRICS_METRIC_TYPE_COUNTER,
        .name = "fivegs_sgwcfunction_sm_sessionnbr",
        .description = "Active Sessions",
    },
};

int sgwc_metrics_init_inst_global(void)
{
    return sgwc_metrics_init_inst(sgwc_metrics_inst_global, sgwc_metrics_spec_global,
                                  _SGWC_METR_GLOB_MAX, 0, NULL);
}

int sgwc_metrics_free_inst_global(void)
{
    return sgwc_metrics_free_inst(sgwc_metrics_inst_global, _SGWC_METR_GLOB_MAX);
}

int sgwc_metrics_open(void)
{
    ogs_metrics_context_t *ctx = ogs_metrics_self();
    ogs_metrics_context_open(ctx);

    sgwc_metrics_init_spec(ctx, sgwc_metrics_spec_global, sgwc_metrics_spec_def_global,
                           _SGWC_METR_GLOB_MAX);

    sgwc_metrics_init_inst_global();

    return 0;
}

int sgwc_metrics_close(void)
{
    ogs_metrics_context_t *ctx = ogs_metrics_self();

    ogs_metrics_context_close(ctx);
    return OGS_OK;
}
