#include "ogs-app.h"
#include "mme-context.h"

#include "metrics.h"

typedef struct mme_metrics_spec_def_s {
    unsigned int type;
    const char *name;
    const char *description;
    int initial_val;
    unsigned int num_labels;
    const char **labels;
} mme_metrics_spec_def_t;

enum { MAX_NUM_KEY_LOCAL_LABELS = 8 };
typedef struct mme_metric_key_local_s {
    const char* labels[MAX_NUM_KEY_LOCAL_LABELS];
    mme_metric_type_local_t t;
} mme_metric_key_local_t;

ogs_hash_t *metrics_hash_local = NULL;   /* hash table for LOCAL labels */

static void mme_metrics_connected_enb_set(char *ip_address, int val);
static void mme_metrics_connected_enb_id_set(char* ip_address, char* cell_id, int val);
static void mme_metrics_ue_session_set(char* imsi, char* apn, int val);
static void mme_metrics_ue_connected_set(char* imsi, int val);
static void mme_metrics_ue_idle_set(char* imsi, int val);
static ogs_metrics_inst_t *get_dynamically_initialised_metric(mme_metric_type_local_t t, const char** labels, unsigned int num_labels);

/* Helper generic functions: */
static int mme_metrics_init_inst(ogs_metrics_inst_t **inst, ogs_metrics_spec_t **specs,
        unsigned int len, unsigned int num_labels, const char **labels)
{
    unsigned int i;
    for (i = 0; i < len; i++)
        inst[i] = ogs_metrics_inst_new(specs[i], num_labels, labels);
    return OGS_OK;
}

static int mme_metrics_free_inst(ogs_metrics_inst_t **inst,
        unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
        ogs_metrics_inst_free(inst[i]);
    memset(inst, 0, sizeof(inst[0]) * len);
    return OGS_OK;
}

static int mme_metrics_init_spec(ogs_metrics_context_t *ctx,
        ogs_metrics_spec_t **dst, mme_metrics_spec_def_t *src, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++) {
        dst[i] = ogs_metrics_spec_new(ctx, src[i].type,
                src[i].name, src[i].description,
                src[i].initial_val, src[i].num_labels, src[i].labels,
                NULL);
    }
    return OGS_OK;
}

/* GLOBAL */
ogs_metrics_spec_t *mme_metrics_spec_global[_MME_METR_GLOB_MAX];
ogs_metrics_inst_t *mme_metrics_inst_global[_MME_METR_GLOB_MAX];
mme_metrics_spec_def_t mme_metrics_spec_def_global[_MME_METR_GLOB_MAX] = {
/* Global Gauges: */
[MME_METR_GLOB_GAUGE_ENB_UE] = {
    .type = OGS_METRICS_METRIC_TYPE_GAUGE,
    .name = "enb_ue",
    .description = "Number of UEs connected to eNodeBs",
},
[MME_METR_GLOB_GAUGE_MME_SESS] = {
    .type = OGS_METRICS_METRIC_TYPE_GAUGE,
    .name = "mme_session",
    .description = "MME Sessions",
},
[MME_METR_GLOB_GAUGE_EMERGENCY_BEARERS] = {
    .type = OGS_METRICS_METRIC_TYPE_GAUGE,
    .name = "emergency_bearers",
    .description = "Number of emergency bearers connected",
},
};

int mme_metrics_init_inst_global(void)
{
    return mme_metrics_init_inst(mme_metrics_inst_global, mme_metrics_spec_global,
                _MME_METR_GLOB_MAX, 0, NULL);
}
int mme_metrics_free_inst_global(void)
{
    return mme_metrics_free_inst(mme_metrics_inst_global, _MME_METR_GLOB_MAX);
}

/* LOCAL */
const char *labels_enb_id[] = {
    "ip_address",
    "cell_id",
};

const char *labels_enb[] = {
    "connected"
};

const char *labels_mme_ue_session[] = {
    "imsi",
    "apn"
};

const char *labels_mme_ue_connected[] = {
    "imsi"
};

const char *labels_mme_ue_idle[] = {
    "imsi"
};

ogs_metrics_inst_t *mme_metrics_inst_local = NULL;
ogs_metrics_spec_t *mme_metrics_spec_local[_MME_METR_LOCAL_MAX];
mme_metrics_spec_def_t mme_metrics_spec_def_local[_MME_METR_LOCAL_MAX] = {
/* Gauges: */
[MME_METR_LOCAL_GAUGE_ENB] = {
        .type = OGS_METRICS_METRIC_TYPE_GAUGE,
        .name = "enb",
        .description = "Status and IP address of eNBs that have connected to this MME",
        .num_labels = OGS_ARRAY_SIZE(labels_enb),
        .labels = labels_enb,
},
[MME_METR_LOCAL_GAUGE_ENB_ID] = {
        .type = OGS_METRICS_METRIC_TYPE_GAUGE,
        .name = "enb_cell_id",
        .description = "Connection status of eNB with eNB ID",
        .num_labels = OGS_ARRAY_SIZE(labels_enb_id),
        .labels = labels_enb_id,
},
[MME_METR_LOCAL_GAUGE_MME_UE_SESSION] = {
        .type = OGS_METRICS_METRIC_TYPE_GAUGE,
        .name = "mme_ue_session_status",
        .description = "Status of a session for MME UEs, if the session is active 1 otherwise 0",
        .num_labels = OGS_ARRAY_SIZE(labels_mme_ue_session),
        .labels = labels_mme_ue_session,
},
[MME_METR_LOCAL_GAUGE_MME_UE_CONNECTED] = {
        .type = OGS_METRICS_METRIC_TYPE_GAUGE,
        .name = "mme_ue_connection_status",
        .description = "Connection status for MME UEs, if UE is attached to MME 1 otherwise 0",
        .num_labels = OGS_ARRAY_SIZE(labels_mme_ue_connected),
        .labels = labels_mme_ue_connected,
},
[MME_METR_LOCAL_GAUGE_MME_UE_IDLE] = {
        .type = OGS_METRICS_METRIC_TYPE_GAUGE,
        .name = "mme_ue_idle_status",
        .description = "Idle status for MME UEs, UEs that have gone idle at least once before will appear here, if idle 1 otherwise 0",
        .num_labels = OGS_ARRAY_SIZE(labels_mme_ue_idle),
        .labels = labels_mme_ue_idle,
},
};

void mme_metrics_connected_enb_add(char *ip_address)
{
    ogs_metrics_inst_inc(mme_metrics_inst_local);
    mme_metrics_connected_enb_set(ip_address, 1);
}

void mme_metrics_connected_enb_clear(char *ip_address)
{
    ogs_metrics_inst_dec(mme_metrics_inst_local);
    mme_metrics_connected_enb_set(ip_address, 0);
}

int mme_metrics_init_inst_local(void)
{
    /* To get around a quirk of the prometheus lib we
     * pass in the key we want as first gauge key/val
     * pair instead of passing in the lables which seems
     * to be what its expecting. */
    const char *total_gauge_key[] = { "total" };
    mme_metrics_inst_local = ogs_metrics_inst_new(mme_metrics_spec_local[MME_METR_LOCAL_GAUGE_ENB], 1, total_gauge_key);

    return OGS_OK;
}

int mme_metrics_free_inst_local(void)
{
    return mme_metrics_free_inst(&mme_metrics_inst_local, _MME_METR_LOCAL_MAX);
}

void mme_metrics_connected_enb_id_add(char* ip_address, char* cell_id)
{
    mme_metrics_connected_enb_id_set(ip_address, cell_id, 1);
}

void mme_metrics_connected_enb_id_clear(char* ip_address, char* cell_id)
{
    mme_metrics_connected_enb_id_set(ip_address, cell_id, 0);
}

void mme_metrics_ue_session_add(char* imsi, char* apn) 
{
    mme_metrics_ue_session_set(imsi, apn, 1);
}

void mme_metrics_ue_session_clear(char* imsi, char* apn) 
{
    mme_metrics_ue_session_set(imsi, apn, 0);
}

void mme_metrics_ue_connected_add(char* imsi)
{
    mme_metrics_ue_connected_set(imsi, 1);
}

void mme_metrics_ue_connected_clear(char* imsi)
{
    mme_metrics_ue_connected_set(imsi, 0);
}

void mme_metrics_ue_idle_add(char* imsi)
{
    mme_metrics_ue_idle_set(imsi, 1);
}

void mme_metrics_ue_idle_clear(char* imsi)
{
    mme_metrics_ue_idle_set(imsi, 0);
}

void mme_metrics_init_local(void)
{
    metrics_hash_local = ogs_hash_make();
    ogs_assert(metrics_hash_local);
}

void mme_metrics_init(void)
{
    ogs_metrics_context_t *ctx = ogs_metrics_self();
    ogs_metrics_context_init();

    mme_metrics_init_spec(ctx, mme_metrics_spec_global, mme_metrics_spec_def_global,
            _MME_METR_GLOB_MAX);

    mme_metrics_init_spec(ctx, mme_metrics_spec_local,
            mme_metrics_spec_def_local, _MME_METR_LOCAL_MAX);

    mme_metrics_init_inst_global();
    mme_metrics_init_inst_local();

    mme_metrics_init_local();
}

void mme_metrics_final(void)
{
    if (metrics_hash_local) {
        ogs_hash_index_t *hi;

        for (hi = ogs_hash_first(metrics_hash_local); hi; hi = ogs_hash_next(hi)) {
            mme_metric_key_local_t *key =
                (mme_metric_key_local_t *)ogs_hash_this_key(hi);

            ogs_hash_set(metrics_hash_local, key, sizeof(*key), NULL);

            ogs_free(key);
            /* don't free val (metric ifself) -
             * it will be free'd by ogs_metrics_context_final() */
            //ogs_free(val);
        }
        ogs_hash_destroy(metrics_hash_local);
    }

    ogs_metrics_context_final();
}

/* Gets an existing one or creates a new one and returns it. 
 * The dynamical ones are different because their labels values
 * aren't known at initialisation time (e.g. an imsi of a UE) */
static ogs_metrics_inst_t *get_dynamically_initialised_metric(mme_metric_type_local_t t, const char** labels, unsigned int num_labels)
{
    ogs_metrics_inst_t *metrics = NULL;
    mme_metric_key_local_t *slice_key;

    slice_key = ogs_calloc(1, sizeof(*slice_key));
    ogs_assert(slice_key);
    ogs_assert(num_labels <= MAX_NUM_KEY_LOCAL_LABELS);

    for (int i = 0; i < num_labels; ++i) {
        slice_key->labels[i] = labels[i];
    }
    slice_key->t = t;

    metrics = ogs_hash_get(metrics_hash_local,
            slice_key, sizeof(*slice_key));

    /* Create the metric if it doesn't already exist.
     * If it does exist then we can just return that. */
    if (!metrics) {
        metrics = ogs_metrics_inst_new(
            mme_metrics_spec_local[t],
            num_labels,
            labels
        );

        ogs_assert(metrics);
        ogs_hash_set(metrics_hash_local,
                slice_key, sizeof(*slice_key), metrics);
    } else {
        ogs_free(slice_key);
    }
    return metrics;
}

static void mme_metrics_connected_enb_set(char *ip_address, int val)
{
    if ((NULL != mme_metrics_inst_local) &&
        (NULL != ip_address)) {
        ogs_metrics_inst_set_with_labels(mme_metrics_inst_local, (const char *[]){ ip_address }, val);
    } else {
        ogs_error("Failed to change eNB metrics as instance or ip address doesn't exist");
    }
}

static void mme_metrics_connected_enb_id_set(char* ip_address, char* cell_id, int val)
{
    const char **labels = (const char *[]){ ip_address, cell_id };

    if ((NULL == ip_address) ||
        (NULL == cell_id))
    {
        ogs_error("Parameter error: Failed to record metric");
        return;
    }

    ogs_metrics_inst_t *metrics = get_dynamically_initialised_metric(MME_METR_LOCAL_GAUGE_ENB_ID, labels, 2);

    if (NULL != metrics) {
        ogs_metrics_inst_set_with_labels(metrics, labels, val);
    } else {
        ogs_error("Failed to record eNB connection status metrics");
    }
}

static void mme_metrics_ue_session_set(char* imsi, char* apn, int val)
{
    const char **labels = (const char *[]){ imsi, apn };

    if ((NULL == imsi) ||
        (NULL == apn))
    { 
        ogs_error("Parameter error: Failed to record metric");
        return;
    }

    ogs_metrics_inst_t *metrics = get_dynamically_initialised_metric(MME_METR_LOCAL_GAUGE_MME_UE_SESSION, labels, 2);

    if (NULL != metrics) {
        ogs_metrics_inst_set_with_labels(metrics, labels, val);
    } else {
        ogs_error("Failed to record UE session status metrics");
    }
}

static void mme_metrics_ue_connected_set(char* imsi, int val)
{
    const char **labels = (const char *[]){ imsi };
    
    if (NULL == imsi)
    { 
        ogs_error("Parameter error: Failed to record metric");
        return;
    }

    ogs_metrics_inst_t *metrics = get_dynamically_initialised_metric(MME_METR_LOCAL_GAUGE_MME_UE_CONNECTED, labels, 1);

    if (NULL != metrics) {
        ogs_metrics_inst_set_with_labels(metrics, labels, val);
    } else {
        ogs_error("Failed to record UE connection status metrics");
    }
}

static void mme_metrics_ue_idle_set(char* imsi, int val)
{
    const char **labels = (const char *[]){ imsi };
    
    if (NULL == imsi) { 
        ogs_error("Parameter error: Failed to record metric");
        return;
    }

    ogs_metrics_inst_t *metrics = get_dynamically_initialised_metric(MME_METR_LOCAL_GAUGE_MME_UE_IDLE, labels, 1);

    if (NULL != metrics) {
        ogs_metrics_inst_set_with_labels(metrics, labels, val);
    } else {
        ogs_error("Failed to record UE idle status metrics");
    }
}