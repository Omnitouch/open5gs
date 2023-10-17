#include "ogs-redis.h"
#include "smf-redis.h"
#include "context.h"


static redisContext* connection = NULL;
static const char * available_ip_record_key = "available_ips";


static bool redis_clear_ip_reuse_from_redis(void);
static bool pfcp_ue_ip_pool_to_redis(void);
static bool redis_pop_available_ip(uint32_t* ipv4);
static bool redis_set_temp_ip_hold(const char* imsi_bcd, const char* apn, uint32_t ipv4);
static bool redis_get_temp_ip_hold(const char* imsi_bcd, const char* apn, uint32_t* ipv4);
static bool redis_remove_available_ip(uint32_t ipv4);
static bool redis_add_available_ip(uint32_t ipv4, size_t available_time);


void smf_redis_init(void) {

    if (smf_self()->redis_ip_reuse.enabled) {
        connection = ogs_redis_initialise(
            smf_self()->redis_server_config.address,
            smf_self()->redis_server_config.port
        );

        /* Clear all the previous reuse data in redis */
        if (!redis_clear_ip_reuse_from_redis()) {
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

bool redis_ip_recycle(const char* imsi_bcd, const char* apn, uint32_t ipv4) {
    ogs_assert(imsi_bcd);
    ogs_assert(apn);

    if (NULL == connection) {
        ogs_error("Cannot call redis_ue_ip_alloc without a valid redis connection");
        return false;
    }

    if (!redis_set_temp_ip_hold(imsi_bcd, apn, ipv4)) {
        char *ip_str = ogs_ipv4_to_string(ipv4);
        ogs_error("Failed to set temporary hold on IP '%s' for UE [%s:%s], something has gone terribly wrong", imsi_bcd, apn, ip_str);
        ogs_free(ip_str);
        return false;
    }

    time_t current_time_sec;
    time(&current_time_sec);

    if (!redis_add_available_ip(ipv4, current_time_sec + smf_self()->redis_ip_reuse.ip_hold_time_sec)) {
        return false;
    }

    return true;
}

ogs_pfcp_ue_ip_t *redis_ue_ip_alloc(const char* imsi_bcd, const char* apn) {
    ogs_assert(imsi_bcd);
    ogs_assert(apn);

    if (NULL == connection) {
        ogs_error("Cannot call redis_ue_ip_alloc without a valid redis connection");
        return false;
    }

    bool isSuccess = false;
    ogs_pfcp_ue_ip_t *ue_ip = NULL;
    uint32_t ipv4 = 0;

    if (redis_get_temp_ip_hold(imsi_bcd, apn, &ipv4)) {
        isSuccess = redis_remove_available_ip(ipv4);
        char *ip_str = ogs_ipv4_to_string(ipv4);
        ogs_debug("UE [%s:%s] has rejoined during the holding interval, it will keep the IP '%s'", imsi_bcd, apn, ip_str);
        ogs_free(ip_str);
    } else {
        isSuccess = redis_pop_available_ip(&ipv4);
        char *ip_str = ogs_ipv4_to_string(ipv4);
        ogs_debug("UE [%s:%s] has not been seen recently and has been given the IP '%s'", imsi_bcd, apn, ip_str);
        ogs_free(ip_str);
    }

    if (isSuccess) {
        ue_ip = ogs_calloc(1, sizeof(ogs_pfcp_ue_ip_t));
        if (!ue_ip) {
            ogs_error("Failed to calloc for ue ip");
            return NULL;
        }
        ue_ip->subnet = ogs_pfcp_find_subnet_by_dnn(AF_INET, apn);
        ue_ip->static_ip = true;
        ue_ip->addr[0] = ipv4;
    } else {
        ogs_fatal("Failed to create ip");
    }

    return ue_ip; 
}

int redis_get_num_available_ips(void) {
    int available = 0;
    
    if (NULL == connection) {
        ogs_error("Cannot call redis_ue_ip_alloc without a valid redis connection");
        return 0;
    }

    redisReply *reply = redisCommand(connection, "ZCOUNT %s -inf +inf", available_ip_record_key);
    
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

static bool redis_clear_ip_reuse_from_redis(void) {
    redisReply *reply = redisCommand(connection, "DEL %s", available_ip_record_key);
    
    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }
    
    freeReplyObject(reply);

    return true;
}

static bool pfcp_ue_ip_pool_to_redis(void) {
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

static bool redis_pop_available_ip(uint32_t* ipv4) {
    time_t currentTime;
    time(&currentTime);

    /* This call essentiall does the following:
     * - Selects ips that have an expiery time between -inf and the current time
     * - Orders the available_ips by expiery time (lowest first)
     * - Finally it returns the ip with the oldest expiery time */
    redisReply *reply = redisCommand(connection, "ZRANGEBYSCORE %s -inf %li LIMIT 0 1", available_ip_record_key, currentTime);

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

    /* As this ip will no longer be available we 
     * need to remove it from the available ip list */
    reply = redisCommand(connection, "ZREM %s %i", available_ip_record_key, *ipv4);
    
    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    freeReplyObject(reply);

    return true;
}

static bool redis_set_temp_ip_hold(const char* imsi_bcd, const char* apn, uint32_t ipv4) {
    if (NULL == connection) {
        ogs_error("Cannot call redis_ue_ip_alloc without a valid redis connection");
        return false;
    }

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

static bool redis_get_temp_ip_hold(const char* imsi_bcd, const char* apn, uint32_t* ipv4) {
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

static bool redis_remove_available_ip(uint32_t ipv4) {
    /* As this ip will no longer be available we 
     * need to remove it from the available ip list */
    redisReply *reply = redisCommand(connection, "ZREM %s %i", available_ip_record_key, ipv4);
    
    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    freeReplyObject(reply);

    return true;
}

static bool redis_add_available_ip(uint32_t ipv4, size_t available_time) {
    if (NULL == connection) {
        ogs_error("Cannot call redis_ue_ip_alloc without a valid redis connection");
        return false;
    }
    
    redisReply *reply = redisCommand(connection, "ZADD %s %u %u", available_ip_record_key, available_time, ipv4);

    if (NULL == reply) {
        ogs_error("Got NULL response from redis, something has gone terribly wrong");
        return false;
    }

    freeReplyObject(reply);
    
    return true;
}
