#include "ogs-redis.h"
#include "smf-redis.h"
#include "context.h"


static redisContext* connection = NULL;
static const char *available_ip_sset_key = "available_ips";
static const char *reserved_ip_hset_key = "reserved_ips";

static ogs_pfcp_ue_ip_t *alloc_ue_ip(const char* apn, uint32_t ipv4);
static bool redis_clear_ip_data(void);
static bool pfcp_ue_ip_pool_to_redis(void);
static bool redis_select_available_ip(uint32_t* ipv4);
static bool redis_reserve_ip(const char* imsi_bcd, const char* apn, uint32_t ipv4);
static bool redis_unreserve_ip(const char* imsi_bcd, const char* apn, uint32_t ipv4);
static bool redis_add_reserved_ip(const char* imsi_bcd, const char* apn, uint32_t ipv4);
static bool redis_get_reserved_ip(const char* imsi_bcd, const char* apn, uint32_t *ipv4);
static bool redis_remove_reserved_ip(const char* imsi_bcd, const char* apn, uint32_t ipv4);
static bool redis_ip_is_reserved(uint32_t ipv4);
static bool redis_hold_ip(const char* imsi_bcd, const char* apn, uint32_t ipv4);
static bool redis_add_ip_hold(const char* imsi_bcd, const char* apn, uint32_t ipv4);
static bool redis_get_ip_hold(const char* imsi_bcd, const char* apn, uint32_t* ipv4);
static bool redis_update_available_ip(uint32_t ipv4, size_t available_time);
static bool redis_add_available_ip(uint32_t ipv4, size_t available_time);
static ogs_pfcp_ue_ip_t *redis_ue_ip_alloc_static(const char* imsi_bcd, const char* apn, uint32_t requested_ipv4);
static ogs_pfcp_ue_ip_t *redis_ue_ip_alloc_dynamic(const char* imsi_bcd, const char* apn);

void smf_redis_init(void) {
    connection = ogs_redis_initialise(
        smf_self()->redis_server_config.address,
        smf_self()->redis_server_config.port
    );

    if (connection)
    {
        ogs_info("Connected to redis server on %s:%u",
            smf_self()->redis_server_config.address,
            smf_self()->redis_server_config.port
        );        
    }

    if (smf_self()->redis_ip_reuse.enabled) {
        /* Clear all the previous reuse data in redis */
        if (!redis_clear_ip_data()) {
            ogs_fatal("Error: Failed to remove previous ip reuse data from redis");
           return;
        }

        if (!pfcp_ue_ip_pool_to_redis()) {
            ogs_fatal("Error: Failed to store all potential ue ips in redis");
        }

        ogs_debug("Number of IPs loaded onto redis: %i", redis_get_num_available_ips());
    }
}

void smf_redis_final(void) {
    if (smf_self()->redis_ip_reuse.enabled) {
        ogs_redis_finalise(connection);
    }
}

int redis_get_num_available_ips(void) {
    int available = 0;
    
    if (NULL == connection) {
        ogs_error("Cannot call redis_ue_ip_alloc without a valid redis connection");
        return 0;
    }

    redisReply *reply = redisCommand(connection, "ZCOUNT %s 1 +inf", available_ip_sset_key);
    
    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return 0;
    }

    if (REDIS_REPLY_INTEGER == reply->type) {
        available = (int)reply->integer;
    }
    
    freeReplyObject(reply);

    return available;
}

bool redis_get_rand_p_cscf_ipv4(ogs_ipsubnet_t *p_cscf, const char *redis_key) {
    bool result = true;

    if (NULL == connection) {
        ogs_error("Cannot call redis_get_rand_p_cscf_ipv4 without a valid redis connection");
        return false;
    }

    /* Randomly pick ipv4 address from the p_cscf_ipv4_set set */
    redisReply *reply = redisCommand(connection, "SRANDMEMBER %s", redis_key);

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    if (REDIS_REPLY_STRING != reply->type) {
        ogs_error("Redis reply was not string!");
        freeReplyObject(reply);
        return false;
    }

    if (OGS_OK != ogs_ipsubnet(p_cscf, reply->str, NULL)) {
        ogs_error("Failed to encode IPv4 address from redis: '%s'", reply->str);
        result = false;
    }

    freeReplyObject(reply);

    return result;
}

ogs_pfcp_ue_ip_t *redis_ue_ip_alloc(const char* imsi_bcd, const char* apn, uint32_t requested_ipv4)
{
    if (0 == requested_ipv4) {
        ogs_debug("Doing dynamic IP allocation");
        return redis_ue_ip_alloc_dynamic(imsi_bcd, apn);
    } else {
        ogs_debug("Doing static IP allocation");
        return redis_ue_ip_alloc_static(imsi_bcd, apn, requested_ipv4);
    }
}

bool redis_ue_ip_free(const char* imsi_bcd, const char* apn, uint32_t ipv4)
{
    // is ue_ip = ogs_calloc(1, sizeof(ogs_pfcp_ue_ip_t)); being freed anywhere?
    bool isSuccess = redis_unreserve_ip(imsi_bcd, apn, ipv4);

    if (false == isSuccess) {
        return false;
    }

    return redis_hold_ip(imsi_bcd, apn, ipv4);
}

static ogs_pfcp_ue_ip_t *redis_ue_ip_alloc_static(const char* imsi_bcd, const char* apn, uint32_t requested_ipv4)
{
    bool isSuccess = false;
    uint32_t ipv4 = 0;

    if (redis_ip_is_reserved(requested_ipv4)) {
        ogs_error("Cannot provide assigned IP as its already been reserved, falling back to dynamic IP selection!");
        return redis_ue_ip_alloc_dynamic(imsi_bcd, apn);
    }

    isSuccess = redis_get_reserved_ip(imsi_bcd, apn, &ipv4);

    if (false == isSuccess) {
        ogs_error("Failed to check if there was a reserved IP");
    }

    if (0 != ipv4) {
        ogs_error("Trying to do a static address assignment when we already have a reserved IP");
        isSuccess = redis_ue_ip_free(imsi_bcd, apn, ipv4);

        if (false == isSuccess) {
            ogs_error("Failed to free our existing reserved IP");
        }
        ipv4 = 0;
    }

    isSuccess = redis_reserve_ip(imsi_bcd, apn, requested_ipv4);
    if (false == isSuccess) {
        ogs_error("Failed to reserve ip!");
        return false;
    }

    return alloc_ue_ip(apn, requested_ipv4);
}

static ogs_pfcp_ue_ip_t *redis_ue_ip_alloc_dynamic(const char* imsi_bcd, const char* apn)
{
    bool isSuccess = false;
    uint32_t ipv4 = 0;

    isSuccess = redis_get_ip_hold(imsi_bcd, apn, &ipv4);

    if (false == isSuccess) {
        ogs_error("Failed to check if there was an IP hold");
    }

    isSuccess = redis_get_reserved_ip(imsi_bcd, apn, &ipv4);

    if (false == isSuccess) {
        ogs_error("Failed to check if there was a reserved IP");
    }

    if (0 == ipv4) {
        isSuccess = redis_select_available_ip(&ipv4);

        if (false == isSuccess) {
            ogs_error("Failed to select an available ip!");
            return false;
        }
    }

    isSuccess = redis_reserve_ip(imsi_bcd, apn, ipv4);
    if (false == isSuccess) {
        ogs_error("Failed to reserve ip!");
        return false;
    }

    return alloc_ue_ip(apn, ipv4);
}

static bool redis_clear_ip_data(void)
{
    bool isSuccess = true;
    redisReply *reply = redisCommand(connection, "DEL %s", available_ip_sset_key);
    
    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        isSuccess = false;
    } else {
        freeReplyObject(reply);
    }

    reply = redisCommand(connection, "DEL %s", reserved_ip_hset_key);
    
    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        isSuccess = false;
    } else {
        freeReplyObject(reply);
    }

    return isSuccess;
}

static bool pfcp_ue_ip_pool_to_redis(void)
{
    ogs_pfcp_subnet_t *subnet = NULL;
    ogs_list_for_each(&ogs_pfcp_self()->subnet_list, subnet) {
        for (int i = 0; i < subnet->pool.size; ++i) {
            ogs_pfcp_ue_ip_t *ue_ip = NULL;
            ue_ip = &subnet->pool.array[i];
            ogs_assert(ue_ip);

            if (!redis_add_available_ip(ue_ip->addr[0], i)) {
                return false;
            }
        }
    }

    return true;
}

static bool redis_select_available_ip(uint32_t* ipv4)
{
    time_t currentTime;
    time(&currentTime);

    /* This call essentiall does the following:
     * - Selects ips that have an expiery time between 1 (0 means that that IP has been reserved) and the current time
     * - Orders the available_ips by expiery time (lowest first)
     * - Finally it returns the ip with the oldest expiery time */
    redisReply *reply = redisCommand(connection, "ZRANGEBYSCORE %s 1 %li LIMIT 0 1", available_ip_sset_key, currentTime);

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    if ((REDIS_REPLY_ARRAY == reply->type) && (1 == reply->elements)) {
        /* We can only SET integers as strings using hiredis so we need
        * to convert the string integer back to an uint32_t */
        redisReply *firstElement = reply->element[0];
        *ipv4 = (uint32_t)atoi(firstElement->str);
    } else {
        freeReplyObject(reply);
        return false;
    }

    freeReplyObject(reply);

    return true;
}

static bool redis_reserve_ip(const char* imsi_bcd, const char* apn, uint32_t ipv4)
{
    bool isSuccess = redis_add_reserved_ip(imsi_bcd, apn, ipv4);

    if (false == isSuccess) {
        return false;
    }

    return redis_update_available_ip(ipv4, 0);
}

static bool redis_unreserve_ip(const char* imsi_bcd, const char* apn, uint32_t ipv4)
{
    return redis_remove_reserved_ip(imsi_bcd, apn, ipv4);
}

static bool redis_add_reserved_ip(const char* imsi_bcd, const char* apn, uint32_t ipv4)
{
    /* In the reserved HSET we add the following:
     *   - [imsi:apn] -> ip
     *   - ip -> [imsi:apn] */
    redisReply *reply = redisCommand(
        connection,
        "HSET %s [%s:%s] %i %i [%s:%s]",
        reserved_ip_hset_key,
        imsi_bcd,
        apn,
        ipv4,
        ipv4,
        imsi_bcd,
        apn
    );

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    freeReplyObject(reply);

    return true;
}

static bool redis_get_reserved_ip(const char* imsi_bcd, const char* apn, uint32_t *ipv4)
{
    /* In the reserved HSET we get the following:
     *   - [imsi:apn] -> ip */
    redisReply *reply = redisCommand(
        connection, 
        "HGET %s [%s:%s]",
        reserved_ip_hset_key,
        imsi_bcd,
        apn
    );

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    if (REDIS_REPLY_STRING != reply->type) {
        freeReplyObject(reply);
        return false;
    }

    *ipv4 = (uint32_t)atoi(reply->str);
    freeReplyObject(reply);

    return true;
}

static bool redis_remove_reserved_ip(const char* imsi_bcd, const char* apn, uint32_t ipv4)
{
    /* In the reserved hash we remove the following:
     *   - [imsi:apn] -> ip
     *   - ip -> [imsi:apn] */
    redisReply *reply = redisCommand(
        connection, 
        "HDEL %s [%s:%s] %i %i [%s:%s]",
        reserved_ip_hset_key,
        imsi_bcd,
        apn,
        ipv4,
        ipv4,
        imsi_bcd,
        apn
    );

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    freeReplyObject(reply);
    
    return true;
}

static bool redis_ip_is_reserved(uint32_t ipv4)
{
    redisReply *reply = redisCommand(
        connection, 
        "HGET %s %i",
        reserved_ip_hset_key,
        ipv4
    );

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    if (REDIS_REPLY_STRING != reply->type) {
        freeReplyObject(reply);
        return false;
    }

    freeReplyObject(reply);

    return true;
}

static bool redis_hold_ip(const char* imsi_bcd, const char* apn, uint32_t ipv4)
{
    bool isSuccess = redis_add_ip_hold(imsi_bcd, apn, ipv4);

    if (false == isSuccess) {
        return false;
    }

    time_t current_time_sec;
    time(&current_time_sec);

    return redis_update_available_ip(ipv4, current_time_sec + smf_self()->redis_ip_reuse.ip_hold_time_sec);
}

static bool redis_add_ip_hold(const char* imsi_bcd, const char* apn, uint32_t ipv4)
{
    /* Add the address to the holding list */
    redisReply *reply = redisCommand(
        connection, 
        "SET [%s:%s] %i EX %i",
        imsi_bcd,
        apn,
        ipv4,
        smf_self()->redis_ip_reuse.ip_hold_time_sec
    );

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    freeReplyObject(reply);

    return true;
}

static bool redis_get_ip_hold(const char* imsi_bcd, const char* apn, uint32_t* ipv4)
{
    /* Do the getting part of a pop */
    redisReply *reply = redisCommand(connection, "GET [%s:%s]", imsi_bcd, apn);

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    if (REDIS_REPLY_STRING != reply->type) {
        freeReplyObject(reply);
        return false;
    }

    *ipv4 = (uint32_t)atoi(reply->str);
    freeReplyObject(reply);

    return true;    
}

static bool redis_update_available_ip(uint32_t ipv4, size_t available_time)
{
    if (NULL == connection) {
        ogs_error("Cannot call redis_ue_ip_alloc without a valid redis connection");
        return false;
    }
    
    redisReply *reply = redisCommand(connection, "ZADD %s XX %u %u", available_ip_sset_key, available_time, ipv4);

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    freeReplyObject(reply);
    
    return true;
}

static bool redis_add_available_ip(uint32_t ipv4, size_t available_time)
{
    if (NULL == connection) {
        ogs_error("Cannot call redis_ue_ip_alloc without a valid redis connection");
        return false;
    }
    
    redisReply *reply = redisCommand(connection, "ZADD %s %u %u", available_ip_sset_key, available_time, ipv4);

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    freeReplyObject(reply);
    
    return true;
}

static ogs_pfcp_ue_ip_t *alloc_ue_ip(const char* apn, uint32_t ipv4)
{
    ogs_pfcp_ue_ip_t *ue_ip = ogs_calloc(1, sizeof(ogs_pfcp_ue_ip_t));
    if (!ue_ip) {
        ogs_error("Failed to calloc for ue ip");
        return NULL;
    }
    ue_ip->subnet = ogs_pfcp_find_subnet_by_dnn(AF_INET, apn);
    ue_ip->static_ip = true;
    ue_ip->addr[0] = ipv4;

    return ue_ip;
}
