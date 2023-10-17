#include "ogs-redis.h"
#include "mme-redis.h"
#include "mme-context.h"


static redisContext* connection = NULL;


void mme_redis_init(void) {
    if (mme_self()->redis_dup_detection.enabled) {
        connection = ogs_redis_initialise(
            mme_self()->redis_server_config.address,
            mme_self()->redis_server_config.port
        );
    }
}

void mme_redis_final(void) {
    if (mme_self()->redis_dup_detection.enabled) {
        ogs_redis_finalise(connection);
    }
}

bool redis_is_message_dup(uint8_t *buf, size_t buf_sz) {
    bool is_dup = true;
    redisReply *reply = NULL;

    if (NULL == connection) {
        ogs_error("Cannot call redis_is_message_dup without a valid redis connection");
        return false;
    }

    /* Have we seen this exact message recently? */
    reply = redisCommand(connection, "GET %b", buf, buf_sz);

    if (NULL == reply) {
        ogs_error("Failed to get a reply from redis server");
        return false;
    }

    if (reply->type != REDIS_REPLY_NIL) {
        ogs_debug("S1AP message was a duplicate");
        is_dup = true;
    }
    else {
        ogs_debug("S1AP message was not a duplicate");
        is_dup = false;
    }
    freeReplyObject(reply);

    /* Tell redis to remember this message for expire_time_sec */
    reply = redisCommand(connection, "INCR %b", buf, buf_sz);
    freeReplyObject(reply);
    reply = redisCommand(connection, "EXPIRE %b %i", buf, buf_sz, mme_self()->redis_dup_detection.expire_time_sec);
    freeReplyObject(reply);

    return is_dup;
}