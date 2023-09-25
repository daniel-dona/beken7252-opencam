#include <rtthread.h>
#include <stdio.h>
#include <string.h>
#include "tjpgd.h"
#include "drv_lcd.h"

/* User defined device identifier */
typedef struct {
    uint8_t *in_buf;     /* Pointer to the frame buffer for input function */
    uint8_t *out_buf;     /* Pointer to the frame buffer for output function */
    uint16_t out_width;    /* Width of the output image */
    uint16_t out_height;    /* Width of the output image */
} Image_Info_t;

static Image_Info_t devid;      /* User defined device identifier */

static uint16_t in_func (JDEC* jd, uint8_t* buff, uint16_t nbyte)
{
    Image_Info_t *dev = (Image_Info_t*)jd->device;   /* Device identifier for the session (5th argument of jd_prepare function) */

    if (buff) {
        /* Read bytes from input buffer */
        memcpy(buff, dev->in_buf, nbyte);
    }
    dev->in_buf += nbyte;

    return nbyte;
}

static uint16_t out_func (JDEC* jd, void* bitmap, JRECT* rect)
{
    Image_Info_t *dev = (Image_Info_t*)jd->device;
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
    dst = dev->out_buf + 3 * (rect->top * (dev->out_width >> jd->scale) + rect->left);  /* Left-top of destination rectangular */
    bws = 3 * (rect->right - rect->left + 1);     /* Width of source rectangular [byte] */
    bwd = 3 * (dev->out_width >> jd->scale);                        /* Width of frame buffer [byte] */
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

static int jpeg_decode(Image_Info_t *dev)
{
    void *work;       /* Pointer to the decompressor work area */
    JDEC jdec;        /* Decompression object */
    JRESULT res;      /* Result code of TJpgDec API */

    /* Allocate a work area for TJpgDec */
    work = rt_malloc(3100);

    /* Prepare to decompress */
    res = jd_prepare(&jdec, in_func, work, 3100, (void *)dev);

    // 清屏显示为背景-黑色显示解码后的图片 
    lcd_clear(BLACK);

    if (res == JDR_OK) {
        /* Ready to dcompress. Image info is available here. */
        rt_kprintf("Image dimensions: %u by %u. %u bytes used.\n", jdec.width, jdec.height, 3100 - jdec.sz_pool);
        dev->out_buf = rt_malloc(3 * jdec.width * jdec.height); /* Frame buffer for output image (assuming RGB888 cfg) */
        dev->out_width= jdec.width;
        dev->out_height = jdec.height;
        res = jd_decomp(&jdec, out_func, 1);   /* Start to decompress with 1/1 scaling */
        if (res == JDR_OK) {
            /* Decompression succeeded. You have the decompressed image in the frame buffer here. */
            rt_kprintf("\rOK  \n");
        } else {
            rt_kprintf("Failed to decompress: rc=%d\n", res);
        }
         dev->out_width >>=  jdec.scale;
        dev->out_height >>=  jdec.scale;

       // rt_free(devid.fbuf);    /* Discard frame buffer */

    } else {
        rt_kprintf("Failed to prepare: rc=%d\n", res);
    }

    rt_free(work);             /* Discard work area */

    return res;

}


int mjpeg_decoder_jpeg(uint8_t *img_buf)
{
    int ret = -1;
    extern int jpeg_decode(Image_Info_t *dev);
    devid.in_buf = img_buf;
    
    ret = jpeg_decode(&devid);

    return ret;

}