#include <stdbool.h>
#include <stdint.h>
#include "ogs-pfcp.h"

void smf_redis_init(void);
void smf_redis_final(void);

ogs_pfcp_ue_ip_t *redis_ue_ip_alloc(const char* imsi_bcd, const char* apn, uint32_t paa_ip);
bool redis_ue_ip_free(const char* imsi_bcd, const char* apn, uint32_t requested_ipv4);
int redis_get_num_available_ips(void);
bool redis_get_rand_p_cscf_ipv4(ogs_ipsubnet_t *p_cscf, const char *redis_key);
