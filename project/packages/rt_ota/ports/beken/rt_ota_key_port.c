#include <stdio.h>
#include <string.h>
#include <stdint.h>

static const uint8_t iv_table[16] = "haiTABncadIob5DR";
static const uint8_t key_table[32] = "FToTU3ut5SmfKp33XbKZ5BLajJSAMN8s";

/**
 * Get the decryption key & iv
 *
 * @param iv_buf initialization vector
 * @param key_buf aes key
 */
void rt_ota_get_iv_key(uint8_t * iv_buf, uint8_t * key_buf)
{
    // TODO ∫Û∆⁄º”√‹¥Ê¥¢
    memcpy(iv_buf, iv_table, sizeof(iv_table));
    memcpy(key_buf, key_table, sizeof(key_table));
}
