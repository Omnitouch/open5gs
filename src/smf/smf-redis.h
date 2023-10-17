#include <stdbool.h>
#include <stdint.h>
#include "ogs-pfcp.h"

void smf_redis_init(void);
void smf_redis_final(void);

bool redis_ip_recycle(const char* imsi_bcd, const char* apn, uint32_t ipv4);
ogs_pfcp_ue_ip_t *redis_ue_ip_alloc(const char* imsi_bcd, const char* apn);
int redis_get_num_available_ips(void);