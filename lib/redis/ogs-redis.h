#include <stdint.h>
#include <hiredis.h>

redisContext* ogs_redis_initialise(const char* address, uint32_t port);
void ogs_redis_finalise(redisContext* connection);
