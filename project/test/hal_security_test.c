#include <rtthread.h>
#include <rthw.h>
#include <stdio.h>
#include <string.h>
#include "sys_config.h"

#include "include.h"
#include "uart_pub.h"
#include "hal_aes.h"
#include "mem_pub.h"
#include "str_pub.h"

void hal_sec_test(int argc, char** argv)
{
    int sec = 0;

    if(0 == os_strcmp(argv[1], "aes"))
    {
        unsigned char aes_key[32] = {
            0x27, 0x73, 0x1a, 0xe6, 0x23, 0x4e, 0xd5, 0xab, 0xfb, 0x07, 0xd5, 0xc8, 0x69, 0x5f, 0x4a, 0x30,
            0x1c, 0x38, 0x2a, 0x09, 0x5b, 0x6f, 0x47, 0xd0, 0x2c, 0x9e, 0x1a, 0xf3, 0x0b, 0x54, 0x6d, 0x82 };
            
        unsigned char aes_block[16] = {
            0xa1, 0x2e, 0x84, 0x5c, 0x10, 0x93, 0xa6, 0x9f, 0x4e, 0xb0, 0x6d, 0xa4, 0x85, 0xa3, 0x8f, 0x5d };

        unsigned char aes_enc_128_expected[16] = {
            0xd0, 0x17, 0xb6, 0x34, 0x7e, 0x4a, 0x35, 0xea, 0x8f, 0x59, 0x01, 0x4a, 0x89, 0xbf, 0xe1, 0x85 };

        unsigned char aes_enc_192_expected[16] = {
            0x9b, 0x05, 0xeb, 0x89, 0xf3, 0x8e, 0x8a, 0xce, 0x7e, 0x26, 0x23, 0x08, 0xb2, 0x2b, 0x32, 0x9f };

        unsigned char aes_enc_256_expected[16] = {
            0xef, 0xac, 0xc5, 0x8a, 0x57, 0xd5, 0xcb, 0x0c, 0x40, 0xa4, 0xb6, 0x0c, 0xbb, 0x70, 0xd4, 0xac };

        unsigned char aes_enc_data[16];
         
        void *ctx = NULL;
        int i, key_len, loop = 0;

        if(argc == 2)
        {
            loop = 3;
            key_len = 128;
        }
        else if(argc == 3)
        {
            loop = 1;
            key_len = os_strtoul(argv[2], NULL, 10);;
        }
        else if(argc == 5)
        {
            key_len = os_strtoul(argv[2], NULL, 10);;

            os_memset(aes_block, 0, 16);
            os_memset(aes_key, 0, 32);
            os_memcpy(aes_block, argv[4], os_strlen(argv[4]));
            os_memcpy(aes_key, argv[3], os_strlen(argv[3]));
        }
        else
        {
            os_printf("Usage: hal_sec_test aes [key_len] [key] [data]\r\n");
            return;
        }

        if((key_len != 128) && (key_len != 192) && (key_len != 256))
        {
            os_printf("key_len must be 128/192/256 \r\n");
            return;
        }

        hal_aes_init(ctx);

aes_loop:
        os_printf("key len: %d: \r\n", key_len);
        for(i = 0; i < key_len/8; i++)
            os_printf("0x%02X, ", aes_key[i]);
        os_printf("\r\n");

        os_printf("block: \r\n");
        for(i = 0; i < 16; i++)
            os_printf("0x%02X, ", aes_block[i]);
        os_printf("\r\n");

        hal_aes_setkey_enc(ctx, aes_key, key_len);

        os_printf("hw encrypt: %d\r\n", loop);
        hal_aes_crypt_ecb(ctx, ENCODE, aes_block, aes_enc_data);
        os_printf("en: \r\n");
        for(i = 0; i < 16; i++)
            os_printf("0x%02X, ", aes_enc_data[i]);
        os_printf("\r\n");

        if(loop)
        {
            unsigned char *cmp_tab;
            if(key_len == 128) 
                cmp_tab = aes_enc_128_expected;
            else if(key_len == 192) 
                cmp_tab = aes_enc_192_expected; 
            else if(loop == 256) 
                cmp_tab = aes_enc_256_expected; 
            
            if(0 != os_memcmp(aes_enc_data, cmp_tab, 16))
            {
                os_printf("expected: \r\n");
                for(i = 0; i < 16; i++)
                    os_printf("0x%02X, ", cmp_tab[i]);
                os_printf("\r\n");
                os_printf("encrypt err\r\n");
            }
            os_printf("*****cmp pass!******\r\n");
        }

        hal_aes_setkey_dec(ctx, aes_key, key_len);

        os_printf("hw decrypt: %d\r\n", loop);
        hal_aes_crypt_ecb(ctx, DECODE, aes_enc_data, aes_enc_data);
        os_printf("de: \r\n");
        for(i = 0; i < 16; i++)
            os_printf("0x%X, ", aes_enc_data[i]);
        os_printf("\r\n");

        if(0 != os_memcmp(aes_enc_data, aes_block, 16))
        {
            os_printf("decrypt err\r\n");
        }
        os_printf("-----cmp pass!-----\r\n");

        if(loop)
        {
            loop--;
            key_len += 64;

            if(loop)
            {
                os_printf("\r\n");
                os_printf("*******************************************\r\n");
                os_printf("\r\n");
                goto aes_loop;
            }
        }

        hal_aes_free(ctx);

    }
}
MSH_CMD_EXPORT(hal_sec_test, hal_sec_test);
