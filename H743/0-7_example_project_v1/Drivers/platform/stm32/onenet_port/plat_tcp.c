#include "plat_tcp.h"
#include "SYSTEM/sys.h"
#include "esp8266.h"
#include "ring_buffer/ring_buffer.h"
#include <string.h>


#define TCP_RX_BUF_SIZE 2048

static ring_buffer_t *s_tcp_rx_rb = NULL;
static uint8_t *s_tcp_rx_buf = NULL;
static esp8266_t *s_esp8266 = NULL;

/**
 * @brief 内部 URC 数据接收回调
 */
static void tcp_recv_handler(const uint8_t *data, size_t len, void *user_data) {
  if (s_tcp_rx_rb) {
    rb_write(s_tcp_rx_rb, data, len);
  }
}

/**
 * @brief 设置用于适配层的 ESP8266 实例
 */
void plat_tcp_set_device(esp8266_t *esp) {
  s_esp8266 = esp;
  if (s_esp8266) {
    esp8266_set_tcp_recv_cb(s_esp8266, tcp_recv_handler, NULL);
  }

  // 初始化接收缓冲
  if (!s_tcp_rx_rb) {
    s_tcp_rx_buf = (uint8_t *)sys_malloc(SYS_MEM_INTERNAL, TCP_RX_BUF_SIZE);
    s_tcp_rx_rb = rb_create(s_tcp_rx_buf, TCP_RX_BUF_SIZE);
  }
}

handle_t plat_tcp_connect(const uint8_t *host, uint16_t port,
                          uint32_t timeout_ms) {
  if (!s_esp8266)
    return NULL;

  if (esp8266_tcp_connect(s_esp8266, (const char *)host, port, timeout_ms) ==
      0) {
    // 清空缓冲区
    uint8_t dummy;
    while (rb_pop(s_tcp_rx_rb, &dummy))
      ;
    return (handle_t)s_esp8266;
  }
  return NULL;
}

int32_t plat_tcp_send(handle_t handle, void *buf, uint32_t len,
                      uint32_t timeout_ms) {
  esp8266_t *esp = (esp8266_t *)handle;
  if (!esp)
    return -1;

  if (esp8266_tcp_send(esp, (const uint8_t *)buf, len, timeout_ms) == 0) {
    return (int32_t)len;
  }
  return -1;
}

int32_t plat_tcp_recv(handle_t handle, void *buf, uint32_t len,
                      uint32_t timeout_ms) {
  (void)handle;
  if (!s_tcp_rx_rb)
    return -1;

  uint32_t start = sys_get_systick_ms();
  size_t read_len = 0;

  while (read_len < len) {
    // 驱动处理 (非常重要！需要在这里驱动 AT 解析)
    esp8266_process(s_esp8266);

    size_t available = rb_available(s_tcp_rx_rb);
    if (available > 0) {
      size_t to_read =
          (len - read_len) > available ? available : (len - read_len);
      read_len += rb_read(s_tcp_rx_rb, (uint8_t *)buf + read_len, to_read);
      if (read_len >= len)
        break;
    }

    if ((sys_get_systick_ms() - start) > timeout_ms) {
      break;
    }
    // 不要阻塞太死
    sys_delay_ms(1);
  }

  return (int32_t)read_len;
}

int32_t plat_tcp_disconnect(handle_t handle) {
  esp8266_t *esp = (esp8266_t *)handle;
  if (esp) {
    esp8266_tcp_disconnect(esp);
  }
  return 0;
}
