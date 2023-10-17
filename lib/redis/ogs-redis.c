#include "ogs-redis.h"
#include "core/ogs-core.h"

redisContext* ogs_redis_initialise(const char* address, uint32_t port) {
    redisContext *connection = redisConnect(
        address,
        port
    );

    if (NULL == connection) {
        ogs_error("Failure: Redis config {address: '%s', port: %i}",
            address,
            port
        );
        
        return NULL;
    }

    if (0 != connection->err) {
        ogs_error("%s - Redis config {address: '%s', port: %i}",
            connection->errstr,
            address,
            port
        );
    } else {
        ogs_debug("Successful connection to redis {address: '%s', port: %i}",
            address,
            port
        );
    }

    return connection;
}

void ogs_redis_finalise(redisContext* connection) {
    if (NULL != connection) {
        redisFree(connection);
    }
}