#include "ogs-app.h"
#include "context.h"

#include "metrics.h"

typedef struct sgwu_metrics_spec_def_s
{
    unsigned int type;
    const char *name;
    const char *description;
    int initial_val;
    unsigned int num_labels;
    const char **labels;
} sgwu_metrics_spec_def_t;

/* Helper generic functions: */
static int sgwu_metrics_init_inst(ogs_metrics_inst_t **inst,
                                  ogs_metrics_spec_t **specs, unsigned int len,
                                  unsigned int num_labels, const char **labels)
{
    unsigned int i;
    for (i = 0; i < len; i++)
        inst[i] = ogs_metrics_inst_new(specs[i], num_labels, labels);
    return OGS_OK;
}

static int sgwu_metrics_free_inst(ogs_metrics_inst_t **inst,
                                  unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
        ogs_metrics_inst_free(inst[i]);
    memset(inst, 0, sizeof(inst[0]) * len);
    return OGS_OK;
}

static int sgwu_metrics_init_spec(ogs_metrics_context_t *ctx,
                                  ogs_metrics_spec_t **dst, sgwu_metrics_spec_def_t *src, unsigned int len)
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
ogs_metrics_spec_t *sgwu_metrics_spec_global[_SGWU_METR_GLOB_MAX];
ogs_metrics_inst_t *sgwu_metrics_inst_global[_SGWU_METR_GLOB_MAX];
sgwu_metrics_spec_def_t sgwu_metrics_spec_def_global[_SGWU_METR_GLOB_MAX] = {
    /* Global Counters: */
    [SGWU_METR_GLOB_GAUGE_GTP_OUTDATAOCTS1USGW] = {
        .type = OGS_METRICS_METRIC_TYPE_GAUGE,
        .name = "fivegs_servinggwfunction_sm_outdataocts1usgw",
        .description = "Number of octets of outgoing GTP data packets on the S1-U interface which have been generated by the GTP-U protocol entity on the S1-U interface",
    },
    [SGWU_METR_GLOB_GAUGE_GTP_INDATAOCTS1USGW] = {
        .type = OGS_METRICS_METRIC_TYPE_GAUGE,
        .name = "fivegs_servinggwfunction_sm_indataocts1usgw",
        .description = "Number of octets of incoming GTP data packets on the S1-U interface which have been accepted and processed by the GTP-U protocol entity on the S1-U interface.",
    },
};

int sgwu_metrics_init_inst_global(void)
{
    return sgwu_metrics_init_inst(sgwu_metrics_inst_global, sgwu_metrics_spec_global,
                                  _SGWU_METR_GLOB_MAX, 0, NULL);
}

int sgwu_metrics_free_inst_global(void)
{
    return sgwu_metrics_free_inst(sgwu_metrics_inst_global, _SGWU_METR_GLOB_MAX);
}

int sgwu_metrics_open(void)
{
    ogs_metrics_context_t *ctx = ogs_metrics_self();
    ogs_metrics_context_open(ctx);

    sgwu_metrics_init_spec(ctx, sgwu_metrics_spec_global, sgwu_metrics_spec_def_global,
                           _SGWU_METR_GLOB_MAX);

    sgwu_metrics_init_inst_global();

    return 0;
}

int sgwu_metrics_close(void)
{
    ogs_metrics_context_t *ctx = ogs_metrics_self();

    ogs_metrics_context_close(ctx);
    return OGS_OK;
}
