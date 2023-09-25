/*------------------------------------------------*/
/* TJpgDec Quick Evaluation Program for PCs       */
/*------------------------------------------------*/

#include <rtthread.h>
#include <stdio.h>
#include <string.h>
#include "tjpgd.h"
#include "drv_lcd.h"
//添加打印调试函数
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void dump_hex(const void *ptr1, rt_size_t buflen)
{
    const rt_uint8_t *ptr = (const rt_uint8_t *)ptr1;
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;

    for (i = 0; i < buflen; i += 16)
    {
        rt_kprintf("%08X: ", i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%02X ", buf[i + j]);
            else
                rt_kprintf("   ");
        rt_kprintf(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
        rt_kprintf("\n");
    }
}

//添加文件操作使用需要的头文件
#include <dfs_posix.h>
/* User defined device identifier */
typedef struct {
    FILE *fp;          /* File pointer for input function */
    uint8_t *fbuf;     /* Pointer to the frame buffer for output function */
    uint16_t wfbuf;    /* Width of the frame buffer [pix] */
} IODEV;


/*------------------------------*/
/* User defined input funciton  */
/*------------------------------*/

uint16_t in_func (JDEC* jd, uint8_t* buff, uint16_t nbyte)
{
    IODEV *dev = (IODEV*)jd->device;   /* Device identifier for the session (5th argument of jd_prepare function) */


    if (buff) {
        /* Read bytes from input stream */
        return (uint16_t)fread(buff, 1, nbyte, dev->fp);
    } else {
        /* Remove bytes from input stream */
        return fseek(dev->fp, nbyte, SEEK_CUR) ? 0 : nbyte;
    }
}


/*------------------------------*/
/* User defined output funciton */
/*------------------------------*/

uint16_t out_func (JDEC* jd, void* bitmap, JRECT* rect)
{
    IODEV *dev = (IODEV*)jd->device;
    uint8_t *src, *dst,*src2;
    uint16_t y, bws, bwd;


    /* Put progress indicator */
    if (rect->left == 0) {
        rt_kprintf("\r%lu%%", (rect->top << jd->scale) * 100UL / jd->height);
    }

    /* Copy the decompressed RGB rectanglar to the frame buffer (assuming RGB888 cfg) */
    src = (uint8_t*)bitmap;
    src2 = (uint8_t*)bitmap;
    lcd_open_window(rect->left,rect->top,(rect->right - rect->left)+1,(rect->bottom - rect->top)+1);
    dst = dev->fbuf + 3 * (rect->top * dev->wfbuf + rect->left);  /* Left-top of destination rectangular */
    bws = 3 * (rect->right - rect->left + 1);     /* Width of source rectangular [byte] */
    bwd = 3 * dev->wfbuf;                         /* Width of frame buffer [byte] */
    for (y = rect->top; y <= rect->bottom; y++) {
        memcpy(dst, src, bws);   /* Copy a line */
        src += bws; dst += bwd;  /* Next line */

        for (int x = 0; x < (rect->right - rect->left) + 1; x++)
        {
            lcd_write_half_word(*(uint16_t *)src2);
            src2 += 2;
        }

    }

    return 1;    /* Continue to decompress */
}


/*------------------------------*/
/* Program Jpeg_Dec             */
/*------------------------------*/

int Jpeg_Dec (int argc, char* argv[])
{
    void *work;       /* Pointer to the decompressor work area */
    JDEC jdec;        /* Decompression object */
    JRESULT res;      /* Result code of TJpgDec API */
    IODEV devid;      /* User defined device identifier */

    /* Open a JPEG file */
    if (argc < 2) return -1;
    devid.fp = fopen(argv[1], "rb");
    if (!devid.fp) return -1;

    /* Allocate a work area for TJpgDec */
    work = rt_malloc(3100);

    /* Prepare to decompress */
    res = jd_prepare(&jdec, in_func, work, 3100, &devid);
    // 清屏显示为背景-黑色显示解码后的图片 
    lcd_clear(BLACK);
    if (res == JDR_OK) {
        /* Ready to dcompress. Image info is available here. */
        rt_kprintf("Image dimensions: %u by %u. %u bytes used.\n", jdec.width, jdec.height, 3100 - jdec.sz_pool);
        devid.fbuf = rt_malloc(3 * jdec.width * jdec.height); /* Frame buffer for output image (assuming RGB888 cfg) */
        devid.wfbuf = jdec.width;
        res = jd_decomp(&jdec, out_func, 1);   /* Start to decompress with 1/1 scaling */
        if (res == JDR_OK) {
            /* Decompression succeeded. You have the decompressed image in the frame buffer here. */
            rt_kprintf("\rOK  \n");
        } else {
            rt_kprintf("Failed to decompress: rc=%d\n", res);
        }
        rt_free(devid.fbuf);    /* Discard frame buffer */

    } else {
        rt_kprintf("Failed to prepare: rc=%d\n", res);
    }

    rt_free(work);             /* Discard work area */

    fclose(devid.fp);       /* Close the JPEG file */

    return res;
}
MSH_CMD_EXPORT(Jpeg_Dec, Jpeg Decode Test);


/*解码保存在 SD 卡上的照片 */
int Decode_Jpg(rt_uint8_t *file_name)
{
    void *work;  /* Pointer to the decompressor work area */
    JDEC jdec;   /* Decompression object */
    JRESULT res; /* Result code of TJpgDec API */
    IODEV devid; /* User defined device identifier */


     /* Open a JPEG file */
    devid.fp = fopen(file_name, "rb");
    if (!devid.fp) return -1;

    /* Allocate a work area for TJpgDec */
    work = rt_malloc(3100);

    /* Prepare to decompress */
    res = jd_prepare(&jdec, in_func, work, 3100, &devid);
    // 清屏显示为背景-黑色显示解码后的图片 
    lcd_clear(BLACK);
    if (res == JDR_OK) {
        /* Ready to dcompress. Image info is available here. */
        rt_kprintf("Image dimensions: %u by %u. %u bytes used.\n", jdec.width, jdec.height, 3100 - jdec.sz_pool);
        devid.fbuf = rt_malloc(3 * jdec.width * jdec.height); /* Frame buffer for output image (assuming RGB888 cfg) */
        devid.wfbuf = jdec.width;
        res = jd_decomp(&jdec, out_func, 1);   /* Start to decompress with 1/1 scaling */
        if (res == JDR_OK) {
            /* Decompression succeeded. You have the decompressed image in the frame buffer here. */
            rt_kprintf("\rOK  \n");
        } else {
            rt_kprintf("Failed to decompress: rc=%d\n", res);
        }
        rt_free(devid.fbuf);    /* Discard frame buffer */

    } else {
        rt_kprintf("Failed to prepare: rc=%d\n", res);
    }

    rt_free(work);             /* Discard work area */

    fclose(devid.fp);       /* Close the JPEG file */

    return res;
}