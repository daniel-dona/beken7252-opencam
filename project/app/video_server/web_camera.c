#include "arm_arch.h"
#include "error.h"
#include "include.h"
#include "net.h"
#include "rtos_pub.h"
#include "typedef.h"
#include "uart_pub.h"
#include "video_transfer.h"
#include <dfs_posix.h>
#include <finsh.h>
#include <rtthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
// #include "test_config.h"
#include "fake_clock_pub.h"

#define MJPEG_BOUNDARY "boundarydonotcross"
#define MAX_BUF_SIZE 30 * 1024
static int g_mjpeg_stop = 0;
static int g_mjpeg_stream = 1;
static char g_send_buf[1024];

beken_semaphore_t buf_ready[2];
beken_semaphore_t buf_free[2];

typedef struct {
  void *buf;
  int *size;
} thread_vbuf_t;

typedef struct {
  void *buf0;
  void *buf1;
  int size0;
  int size1;
} thread_2vbuf_t;

int send_first_response(int client) {
  g_send_buf[0] = 0;

  rt_snprintf(g_send_buf, 1024,
              "HTTP/1.0 200 OK\r\n"
              "Connection: close\r\n"
              "Server: MJPG-Streamer/0.2\r\n"
              "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0,"
              " post-check=0, max-age=0\r\n"
              "Pragma: no-cache\r\n"
              "Expires: Mon, 3 Jan 2000 12:34:56 GMT\r\n"
              "Content-Type: multipart/x-mixed-replace;boundary=" MJPEG_BOUNDARY
              "\r\n"
              "\r\n"
              "--" MJPEG_BOUNDARY "\r\n");
  if (send(client, g_send_buf, strlen(g_send_buf), 0) < 0) {
    close(client);
    return -1;
  }

  return 0;
}

int mjpeg_send_stream(int client, void *data, int size) {
  g_send_buf[0] = 0;

  if (!g_mjpeg_stop) {
    snprintf(g_send_buf, 1024,
             "Content-Type: image/jpeg\r\n"
             "Content-Length: %d\r\n"
             "\r\n",
             size);
    if (send(client, g_send_buf, strlen(g_send_buf), 0) < 0) {
      close(client);
      return -1;
    }

    if (send(client, data, size, 0) < 0) {
      close(client);
      return -1;
    }

    g_send_buf[0] = 0;
    snprintf(g_send_buf, 1024, "\r\n--" MJPEG_BOUNDARY "\r\n");
    if (send(client, g_send_buf, strlen(g_send_buf), 0) < 0) {
      close(client);
      return -1;
    }

    return 0;
  }

  return -1;
}

void mjpeg_server_thread_capture(void *arg) {

  uint8_t active_idx = 0;

  thread_2vbuf_t *params = (thread_2vbuf_t *)arg;
  rt_tick_t t0 = 0, t1 = 0;

  uint8_t stat_count = 0;

  while (!g_mjpeg_stop) {
    while (!g_mjpeg_stream) {

      switch (active_idx) {
      case 0:
        //bk_printf("Waiting to wrote on %d\r\n", active_idx);
        if (rtos_get_semaphore(&buf_free[active_idx], BEKEN_WAIT_FOREVER) ==
            kNoErr) {
          t0 = rt_tick_get();
          params->size0 = video_buffer_read_frame(params->buf0, MAX_BUF_SIZE);
          t1 = rt_tick_get();
          // bk_printf("Wrote on %d\r\n", active_idx);
          rtos_set_semaphore(&buf_ready[active_idx]);
          active_idx = 1;
          break;
        }

      case 1:
        //bk_printf("Waiting to wrote on %d\r\n", active_idx);
        if (rtos_get_semaphore(&buf_free[active_idx], BEKEN_WAIT_FOREVER) ==
            kNoErr) {
          t0 = rt_tick_get();
          params->size1 = video_buffer_read_frame(params->buf1, MAX_BUF_SIZE);
          t1 = rt_tick_get();
          // bk_printf("Wrote on %d\r\n", active_idx);
          //bk_printf("Size %d\r\n", params->size1);
          rtos_set_semaphore(&buf_ready[active_idx]);
          active_idx = 0;
          break;
        }
      }

      /*if((int) (100-(t1-t0)) > 5){
          //bk_printf(">>> %d %d\r\n", (int) 100-(t1-t0), (100-(t1-t0)) > 5);
          rtos_delay_milliseconds((int) 100-(t1-t0));
      }*/

      stat_count += 1;

      if (stat_count == 100) {
        bk_printf("Sensor delay: %d\r\n", t1 - t0);
        stat_count = 0;
      }
    }

    rtos_delay_milliseconds(100);
  }
}

void mjpeg_server_thread(void *arg) {

  uint8_t active_idx = 0;

  int on;
  int srv_sock = -1;
  // int fream_length=0;
  struct sockaddr_in addr;
  socklen_t sock_len = sizeof(struct sockaddr_in);

  thread_2vbuf_t *params = (thread_2vbuf_t *)arg;

  srv_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (srv_sock < 0) {
    rt_kprintf("mjpeg_server: create server socket failed due to (%s)\n",
               strerror(errno));
    goto exit;
  }

  bzero(&addr, sock_len);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);
  addr.sin_addr.s_addr = INADDR_ANY;

  /* ignore "socket already in use" errors */
  on = 1;
  lwip_setsockopt(srv_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  lwip_setsockopt(srv_sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

  if (bind(srv_sock, (struct sockaddr *)&addr, sock_len) != 0) {
    rt_kprintf("mjpeg_server: bind() failed due to (%s)\n", strerror(errno));
    goto exit;
  }

  if (listen(srv_sock, RT_LWIP_TCP_PCB_NUM) != 0) {
    rt_kprintf("mjpeg_server: listen() failed due to (%s)\n", strerror(errno));
    goto exit;
  }

  g_mjpeg_stop = 0;
  while (!g_mjpeg_stop) {
    struct sockaddr_in client_addr;
    int client = accept(srv_sock, (struct sockaddr *)&client_addr, &sock_len);
    if (client < 0)
      continue;

    rt_kprintf("mjpeg_server: client connected\n");
    if (send_first_response(client) < 0) {
      client = -1;
      continue;
    }

    rt_tick_t t0 = 0, t1 = 0;
    int res = 0;
    uint8_t stat_count = 0;

    while (1) {

      g_mjpeg_stream = 0;

      switch (active_idx) {
      case 0:
        // bk_printf("Waiting to read from %d\r\n", active_idx);
        if (rtos_get_semaphore(&buf_ready[active_idx], BEKEN_WAIT_FOREVER) ==
            kNoErr) {
          t0 = rt_tick_get();
          res = mjpeg_send_stream(client, (void *)params->buf0, params->size0);
          t1 = rt_tick_get();
          // bk_printf("Read from %d\r\n", active_idx);

          rtos_set_semaphore(&buf_free[active_idx]);
          active_idx = 1;

          break;
        }

      case 1:
        // bk_printf("Waiting to read from %d\r\n", active_idx);
        if (rtos_get_semaphore(&buf_ready[active_idx], BEKEN_WAIT_FOREVER) ==
            kNoErr) {
          t0 = rt_tick_get();
          res = mjpeg_send_stream(client, (void *)params->buf1, params->size1);
          t1 = rt_tick_get();
          // bk_printf("Read from %d\r\n", active_idx);

          rtos_set_semaphore(&buf_free[active_idx]);
          active_idx = 0;

          break;
        }
      }

      if (stat_count == 100) {
        bk_printf("Send frame latency %d ms\r\n", t1 - t0);
        stat_count = 0;
      }

      stat_count += 1;

      if (res < 0) {
        rt_kprintf("client disconnected!\n");
        g_mjpeg_stream = 1;
        break;
      }
    }
  }

exit:
  if (srv_sock >= 0) {
    close(srv_sock);
  }
}

int web_jpeg_stream(int argc, char **argv) {

  if (argc != 2) {
    rt_kprintf("%s start|stop\n", argv[0]);
    return 0;
  }

  video_buffer_open();

  if (strcmp(argv[1], "start") == 0) {
    rt_kprintf("start web camera\r\n");

    thread_2vbuf_t *params = malloc(sizeof(thread_2vbuf_t));

    params->buf0 = malloc(MAX_BUF_SIZE);
    params->buf1 = malloc(MAX_BUF_SIZE);
    params->size0 = 0;
    params->size1 = 0;

    rtos_init_semaphore(&buf_ready[0], 1);
    rtos_init_semaphore(&buf_ready[1], 1);
    rtos_init_semaphore(&buf_free[0], 1);
    rtos_init_semaphore(&buf_free[1], 1);

    rtos_set_semaphore(&buf_free[0]);
    rtos_set_semaphore(&buf_free[1]);

    if (params != NULLPTR && params->buf0 != NULLPTR &&
        params->buf1 != NULLPTR) {

      rt_thread_t tid;

      tid = rt_thread_create("jpeg_stream", mjpeg_server_thread, params, 2048,
                             10, 5);
      if (tid) {
        rt_thread_startup(tid);
      }

      tid = rt_thread_create("jpeg_stream_capture", mjpeg_server_thread_capture,
                             params, 2048, 10, 5);
      if (tid) {
        rt_thread_startup(tid);
      }
    } else {
      bk_printf("Malloc failed!\r\n");
    }

  } else {
    g_mjpeg_stop = 1;
  }

  return 0;
}

static struct dfs_fd fd;

void save_pic(int argc, char **argv) {

  if (argc == 2) {

    video_buffer_open();

    uint8_t *buf = (uint8_t *)malloc(MAX_BUF_SIZE);

    int frame_length = 0;

    frame_length = video_buffer_read_frame(buf, MAX_BUF_SIZE);

    if (frame_length != 0) {
      os_printf("Got frame! Len: %d\n", frame_length);
    }

    if (dfs_file_open(&fd, argv[1], O_WRONLY | O_CREAT) < 0) {
      rt_kprintf("Open failed\n");
    } else {

      dfs_file_write(&fd, buf, frame_length);
      dfs_file_close(&fd);
    }

    video_buffer_close();

    free(buf);

  } else {
    os_printf("Use save_pic <file_name>");
  }
}

MSH_CMD_EXPORT(web_jpeg_stream, web_jpeg_stream server);
// MSH_CMD_EXPORT(save_pic, save_pic to sd);
