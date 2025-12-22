#include "esp8266.h"
#include "SYSTEM/sys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct esp8266_t {
  at_device_t *at_dev;
  esp8266_tcp_recv_cb recv_cb;
  void *recv_user_data;
  bool is_initialized;
  bool connected_wifi;
  bool connected_tcp;

  // 异步控制
  at_result_t last_res;
  bool async_done;
};

static void esp_at_cb(at_result_t result, const char *response,
                      void *user_data) {
  esp8266_t *esp = (esp8266_t *)user_data;
  esp->last_res = result;
  esp->async_done = true;
}

static void esp_urc_handler(at_device_t *device, const char *line,
                            void *user_data) {
  esp8266_t *esp = (esp8266_t *)user_data;

  // 处理 +IPD
  if (strstr(line, "+IPD")) {
    int len = 0;
    char *ptr = strchr(line, ':');
    if (ptr) {
      // 格式: +IPD,len: 或者 +IPD,id,len:
      char *comma = strrchr(line, ',');
      if (comma) {
        len = atoi(comma + 1);
      }
      if (len > 0) {
        at_device_enter_raw_mode(device, len);
      }
    }
  }
}

static void esp_raw_handler(uint8_t ch, void *user_data) {
  esp8266_t *esp = (esp8266_t *)user_data;
  if (esp->recv_cb) {
    esp->recv_cb(&ch, 1, esp->recv_user_data);
  }
}

esp8266_t *esp8266_create(uart_queue_t *uart_queue) {
  esp8266_t *esp = (esp8266_t *)sys_malloc(SYS_MEM_INTERNAL, sizeof(esp8266_t));
  if (esp) {
    memset(esp, 0, sizeof(esp8266_t));
    at_device_config_t config = {.uart_queue = uart_queue,
                                 .default_timeout = 5000,
                                 .urc_handler = esp_urc_handler,
                                 .raw_handler = esp_raw_handler,
                                 .user_data = esp};
    esp->at_dev = at_device_create(&config);
    if (!esp->at_dev) {
      sys_free(SYS_MEM_INTERNAL, esp);
      return NULL;
    }
  }
  return esp;
}

static at_result_t wait_async(esp8266_t *esp, uint32_t timeout) {
  uint32_t start = sys_get_systick_ms();
  esp->async_done = false;
  while (!esp->async_done) {
    at_device_process(esp->at_dev);
    if ((sys_get_systick_ms() - start) > timeout + 100) {
      return AT_RESULT_TIMEOUT;
    }
    sys_delay_ms(1);
  }
  return esp->last_res;
}

int esp8266_init(esp8266_t *esp) {
  if (!esp)
    return -1;

  // 测试通讯
  at_device_send_cmd(esp->at_dev, "AT", 1000, esp_at_cb, esp);
  if (wait_async(esp, 1000) != AT_RESULT_OK)
    return -2;

  // 关闭回显
  at_device_send_cmd(esp->at_dev, "ATE0", 1000, esp_at_cb, esp);
  if (wait_async(esp, 1000) != AT_RESULT_OK)
    return -3;

  // 设置为 Station 模式
  at_device_send_cmd(esp->at_dev, "AT+CWMODE=1", 1000, esp_at_cb, esp);
  if (wait_async(esp, 1000) != AT_RESULT_OK)
    return -4;

  esp->is_initialized = true;
  return 0;
}

int esp8266_wifi_connect(esp8266_t *esp, const char *ssid, const char *pwd,
                         uint32_t timeout) {
  if (!esp || !ssid)
    return -1;
  char cmd[128];
  sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd ? pwd : "");
  at_device_send_cmd(esp->at_dev, cmd, timeout, esp_at_cb, esp);
  if (wait_async(esp, timeout) != AT_RESULT_OK)
    return -2;
  esp->connected_wifi = true;
  return 0;
}

int esp8266_tcp_connect(esp8266_t *esp, const char *host, uint16_t port,
                        uint32_t timeout) {
  if (!esp || !host)
    return -1;
  char cmd[128];
  sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%u", host, port);
  at_device_send_cmd(esp->at_dev, cmd, timeout, esp_at_cb, esp);
  if (wait_async(esp, timeout) != AT_RESULT_OK)
    return -2;
  esp->connected_tcp = true;
  return 0;
}

int esp8266_tcp_send(esp8266_t *esp, const uint8_t *data, size_t len,
                     uint32_t timeout) {
  if (!esp || !data || len == 0)
    return -1;

  char cmd[32];
  sprintf(cmd, "AT+CIPSEND=%u", (unsigned int)len);

  // 这里需要同步处理，因为要等 >
  // 由于 at_device 目前还不支持 > 提示符的回调，我们需要特殊处理
  // 暂时假设 AT+CIPSEND 成功后立即发送数据 (某些模块支持，但标准ESP需要等 >)
  // TODO: 在 at_device 中支持 > 提示符

  at_device_send_cmd(esp->at_dev, cmd, 1000, esp_at_cb, esp);
  if (wait_async(esp, 1000) != AT_RESULT_OK)
    return -2;

  // 发送原始数据
  at_device_send_data(esp->at_dev, data, len);

  // 等待 SEND OK
  // 注意：at_device_send_cmd 会发送 \r\n，但对于纯数据不需要。
  // 我们在这里其实是等下一个 OK 响应。
  // 但是 at_device 已经回到 IDLE 状态了，我们需要手动切回 WAIT_RSP 或者让
  // at_device 支持

  // 改进：我们可以再次调用 at_device_send_cmd 发送空串来等待结果
  at_device_send_cmd(esp->at_dev, "", timeout, esp_at_cb, esp);
  if (wait_async(esp, timeout) != AT_RESULT_OK)
    return -3;

  return 0;
}

int esp8266_tcp_disconnect(esp8266_t *esp) {
  if (!esp)
    return -1;
  at_device_send_cmd(esp->at_dev, "AT+CIPCLOSE", 1000, esp_at_cb, esp);
  wait_async(esp, 1000); // 即使失败也清标记
  esp->connected_tcp = false;
  return 0;
}

void esp8266_set_tcp_recv_cb(esp8266_t *esp, esp8266_tcp_recv_cb cb,
                             void *user_data) {
  if (esp) {
    esp->recv_cb = cb;
    esp->recv_user_data = user_data;
  }
}

void esp8266_process(esp8266_t *esp) {
  if (esp) {
    at_device_process(esp->at_dev);
  }
}
