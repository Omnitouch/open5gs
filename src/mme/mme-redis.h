#include <stdbool.h>
#include <stdint.h>

void mme_redis_init(void);
void mme_redis_final(void);

bool redis_is_message_dup(uint8_t *buf, size_t buf_sz);