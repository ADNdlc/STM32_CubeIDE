# esp32-MQTT

> 官方文档
> [硬件连接ESP-AT 用户指南 文档](https://espressif-docs.readthedocs-hosted.com/projects/esp-at/zh-cn/release-v2.2.0.0_esp8266/Get_Started/Hardware_connection.html)

> [!NOTE]
>
> 以下使用ESP32-WROOM-32模组ESP32-DEVKITV1开发板进行，不同模组和开发板引脚和固件可能不同，详情见官方文档，以下是DEV开发板引脚定义：

![ESP32_Dev引脚定义](C:\Users\12114\Desktop\Arduino开发\ESP32_Dev引脚定义.png)

***

## AT固件烧录

> 烧录工具固件下载地址
> [发布的固件 - ESP32 - — ESP-AT 用户指南 latest 文档](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Binary_Lists/esp_at_binaries.html)
> [Flash 下载工具用户指南 - ESP32 - — ESP 测试工具 latest 文档](https://docs.espressif.com/projects/esp-test-tools/zh_CN/latest/esp32/production_stage/tools/flash_download_tool.html)

> [!IMPORTANT]
>
> 注意！要烧录`下载的固件/factory` 目录下的 `factory_XXX.bin`至 `0x0` 地址：勾选 “DoNotChgBin”

![image-20250517174332912](../../../AppData/Roaming/Typora/typora-user-images/image-20250517174332912.png)

烧录完成后，连接串口2(RX2,TX2)到电脑，发送 ==AT+GMR+回车==，若返回以下信息代表烧录成功：

>AT version:3.4.0.0(s-c31b833 - ESP32 - Jun  7 2024 03:48:17)
>SDK version:v5.0.6-dirty
>compile time(70ff5889):Jun  7 2024 04:46:00
>Bin version:v3.4.0.0(WROOM-32)
>
>OK


---



## 基本功能



### [开启或关闭 AT 回显功能](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/Basic_AT_Commands.html#ate-at)

```c
ATE0
ATE1
//响应：
OK
```

- **ATE0**：关闭回显
- **ATE1**：开启回显

---



## WiFi连接

> 官方AT命令集
> [ESP-AT命令集 — 总目录](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/index.html)
> [Wi-Fi AT 命令集 - ESP32 — ESP-AT 用户指南 latest 文档](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/Wi-Fi_AT_Commands.html)

> [!NOTE]
>
> 要保证WiFi模块供电稳定，建议使用充电器供电

### 查看模工作模式

```c
AT+CWMODE?
//响应：
+CWMODE:<mode>
    
OK
```

<mode>：模式
0: 无 Wi-Fi 模式，并且关闭 Wi-Fi RF
1: Station 模式
2: SoftAP 模式
3: SoftAP+Station 模式

### [设置 Wi-Fi 模式](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/Wi-Fi_AT_Commands.html#at-cwmode-wi-fi-station-softap-station-softap)

```c
AT+CWMODE=<mode>,<auto_connect>
			   //auto_connect:Station或混合模式自动连接已保存的AP
AT+CWMODE=3,1
//响应：
OK
```

### 连接网络

#### 扫描WiFi:

```c
AT+CWLAP
//响应：
+CWLAP:(3,"test2",-24,"ea:54:08:89:21:20",1,-1,-1,4,4,7,1)
......
```

#### 连接WiFi：

```c
AT+CWJAP=<"ssid">,<"pwd">,<"bssid">.......(详细参数见文档)
AT+CWJAP="test2","12345678"
//响应：
WIFI CONNECTED
WIFI GOT IP

OK
```

#### 查询当前WiFi：

```c
AT+CWJAP?
//响应：
busy p...
+CWJAP:"test2","ea:54:08:89:21:20",1,-27,0,1,3,0,1

OK 
```

#### 断开当前WiFi：

```c
AT+CWQAP
//响应
busy p...
WIFI DISCONNECT

OK
```

#### 重连上次的WiFi：

```c
AT+CWJAP
```

默认上电重连WiFi



---

## 获取时间戳



**连接上WiFi后**

[AT+CIPSNTPCFG](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/TCP-IP_AT_Commands.html#cmd-sntpcfg) **：查询/设置时区和 SNTP 服务器**

```c
AT+CIPSNTPCFG=<enable>,<timezone>,<"SNTP server1">,<"SNTP server2">,<"SNTP server3">
AT+CIPSNTPCFG=1,8,"cn.ntp.org.cn"
//响应：
OK
//间隔一会
+TIME_UPDATED
```

[AT+CIPSNTPTIME](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/TCP-IP_AT_Commands.html#cmd-sntpt) **：查询 SNTP 时间**

```c
AT+CIPSNTPTIME?
//响应：
+CIPSNTPTIME:Fri May 30 18:06:18 2025
OK
```



***

## OneNet云平台连接

> 官方MQTT-AT命令集
> [MQTT AT 命令集 - ESP32 — ESP-AT 用户指南 latest 文档](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#)



### [设置 MQTT 用户属性](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttusercfg-mqtt)

```c
AT+MQTTUSERCFG=<LinkID>,<scheme>,<"client_id">,<"username">,<"password">,<cert_key_ID>,<CA_ID>,<"path">

AT+MQTTUSERCFG=0,1,"temperatureAndHumidity","SQKg9n0Ii0","",0,0,""
//						<设备名称/ID>		<产品ID>
```

- **<LinkID>**：当前仅支持 link ID 0。
- **<”client_id”>**：MQTT 客户端 ID，最大长度：256 字节。
- **<”username”>**：用户名，用于登陆 MQTT broker，最大长度：64 字节。
- **<”password”>**：密码，用于登陆 MQTT broker，最大长度：64 字节。
- **<scheme>**：**==这里使用TCP连接==**
	- 1: MQTT over TCP；
	- 2: MQTT over TLS（不校验证书）；
	- 3: MQTT over TLS（校验 server 证书）；
	- 4: MQTT over TLS（提供 client 证书）；
	- 5: MQTT over TLS（校验 server 证书并且提供 client 证书）；
	- 6: MQTT over WebSocket（基于 TCP）；
	- 7: MQTT over WebSocket Secure（基于 TLS，不校验证书）；
	- 8: MQTT over WebSocket Secure（基于 TLS，校验 server 证书）；
	- 9: MQTT over WebSocket Secure（基于 TLS，提供 client 证书）；
	- 10: MQTT over WebSocket Secure（基于 TLS，校验 server 证书并且提供 client 证书）

**由于OneNet平台的密码非常长，需使用MQTT LONG PASSWORD来设置密码**

> **密码使用工具生成**
> [token生成工具_开发者文档_OneNET](https://open.iot.10086.cn/doc/mqtt/book/manual/auth/tool.html)（[点击下载](https://open.iot.10086.cn/doc/mqtt/images/tools/token.exe)）
>
> - res:	oducts/产品id/devices/设备名称/id
> - et:	  当前时间戳(要改大一点，使用未来的时间戳)
> - key:	“设备密钥”
> - 方法:      md5



---

### [设置 MQTT 客户端 ID](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttlongclientid-mqtt-id)

```c
AT+MQTTLONGCLIENTID=<LinkID>,<length>

//响应
OK

>

```

上述响应表示 AT 已准备好接收 MQTT 客户端 ID，此时您可以输入客户端 ID，当 AT 接收到的客户端 ID 长度达到 `<length>` 后，返回：

```c
OK
```

- **<LinkID>**：当前仅支持 link ID 0。

- **<length>**：MQTT 客户端 ID 长度。范围：[1,1024]。

	

---

### [设置 MQTT 登陆用户名](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttlongusername-mqtt)

```c
AT+MQTTLONGUSERNAME=<LinkID>,<length>

//响应
OK

>
```

上述响应表示 AT 已准备好接收 MQTT 用户名，此时您可以输入 MQTT 用户名，当 AT 接收到的 MQTT 用户名长度达到 `<length>` 后，返回：

```
OK
```

- **<LinkID>**：当前仅支持 link ID 0。

- **<length>**：MQTT 用户名长度。范围：[1,1024]。

	

---

### [设置 MQTT 登陆密码](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttlongpassword-mqtt)

> <length>长度计算
> [在线字符串长度计算工具](https://www.lddgo.net/string/stringlength)
>
> [**OneNET - Token算法**](https://open.iot.10086.cn/doc/v5/fuse/detail/1486)

```c
AT+MQTTLONGPASSWORD=<LinkID>,<length>

AT+MQTTLONGPASSWORD=0,140
//响应
OK

>
//输入：
//version=2018-10-31&res=products%2FSQKg9n0Ii0%2Fdevices%2FtemperatureAndHumidity&et=1757458587&method=md5&sign=YCozJxz%2BPX0Qf1coFGDd0A%3D%3D
//响应：
busy p...

OK
```

上述响应表示 AT 已准备好接收 MQTT 密码，此时您可以输入 MQTT 密码，当 AT 接收到的 MQTT 密码长度达到 `<length>` 后，返回：

```c
busy p...

OK
```

- **<LinkID>**：当前仅支持 link ID 0。

- **<length>**：MQTT 密码长度。范围：[1,1024]

	

---

## [连接 MQTT Broker](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttconn-mqtt-broker)

> [OneNet服务地址](https://open.iot.10086.cn/doc/mqtt/book/device-develop/manual.html)
>
> [OneNET - 通信主题列表](https://open.iot.10086.cn/doc/v5/fuse/detail/920)


```c
AT+MQTTCONN=<LinkID>,<"host">,<port>,<reconnect>
AT+MQTTCONN=0,"mqtts.heclouds.com",1883,0
//响应：
    
OK
//打开云平台可见设备变为在线
```

- **<”host”>**：MQTT broker 域名，最大长度：128 字节。
- **<port>**：MQTT broker 端口，最大端口：65535。
- **<reconnect>**：
	- 0: MQTT 不自动重连。如果 MQTT 建立连接后又断开，则无法再次使用本命令重新建立连接，您需要先发送 [AT+MQTTCLEAN=0](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#cmd-mqttclean) 命令清理信息，重新配置参数，再建立新的连接。
	- 1: MQTT 自动重连，会消耗较多的内存资源。



---

#### 查询 ESP32 设备已连接的 MQTT broker：

```c
AT+MQTTCONN?
//响应
+MQTTCONN:<LinkID>,<state>,<scheme>,<"host">,<port>,<"path">,<reconnect>
OK
    
+MQTTCONN:0,4,1,"mqtts.heclouds.com","1883","",0

OK
```

- **<state>**：MQTT 状态：

	- 0: MQTT 未初始化；

	- 1: 已设置 [AT+MQTTUSERCFG](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#cmd-mqttusercfg)；

	- 2: 已设置 [AT+MQTTCONNCFG](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#cmd-mqttconncfg)；

	- 3: 连接已断开；

	- 4: 已建立连接；

	- 5: 已连接，但未订阅 topic；

	- 6: 已连接，已订阅过 topic。

		

---

### [订阅 MQTT Topic](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttsub-mqtt-topic)

> [OneNET - 通信主题](https://open.iot.10086.cn/doc/v5/fuse/detail/920)



订阅指定 MQTT topic 的指定 QoS，支持订阅多个 topic（最多支持订阅 10 个 topic）

```c
AT+MQTTSUB=<LinkID>,<"topic">,<qos>
//这里订阅设备属性上报响应
AT+MQTTSUB=0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply",0
//响应：
    
OK
```

#### 查询已订阅的 topic

```c
AT+MQTTSUB?
//响应：
+MQTTSUB:<LinkID>,<state>,<"topic1">,<qos>
+MQTTSUB:<LinkID>,<state>,<"topic2">,<qos>
...
OK

+MQTTSUB:0,6,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply",0

OK
```

---

#### [取消订阅 MQTT Topic](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttunsub-mqtt-topic)

客户端取消订阅指定 topic，可多次调用本命令，以取消订阅不同的 topic。

```c
AT+MQTTUNSUB=<LinkID>,<"topic">
//响应
OK

AT+MQTTUNSUB=0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply"
OK
```

若未订阅过该 topic，则返回：

```c
NO UNSUBSCRIBE

OK
```



#### 接收云平台命令

> [OneNET - 5. 设备属性设置](https://open.iot.10086.cn/doc/v5/fuse/detail/922)

```c
//设备侧需要收到平台下发的数据，需要订阅：
$sys/{pid}/{device-name}/thing/property/set
//订阅设备属性设置主题：
AT+MQTTSUB=0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/set",0
```

**这里定义一个bool功能点:**

![image-20250609125823779](../../../AppData/Roaming/Typora/typora-user-images/image-20250609125823779.png)

**设置属性ture-ON**

> [发布 MQTT 消息（字符串](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttpub-mqtt)

```c
//发送命令后可看到模块:
+MQTTSUBRECV:0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/set",49,{"id":"12","version":"1.0","params":{"LED":true}}
//注意"id":"1"回复时需要匹配
//如果一定时间内不回复：
```

![image-20250609130740399](../../../AppData/Roaming/Typora/typora-user-images/image-20250609130740399.png)

```c
//设置成功回复：
$sys/{pid}/{device-name}/thing/property/set_reply	//回复主题
//回复内容：
{
	"id": "2",
	"code": 200,
	"msg": "success"
}
其中id为下行数据的id，需要匹配，code为200代表成功，按需匹配，msg可以自定义。
//模块命令：
AT+MQTTPUBRAW=0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/set_reply",38,0,0
//等待 > 然后发送：
{"id":"12","code":200,"msg":"success"}
```

然后网站可看到成功设置和模块回复消息：

![image-20250609133050883](../../../AppData/Roaming/Typora/typora-user-images/image-20250609133050883.png)

---

### [发布长 MQTT 消息](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttpubraw-mqtt)

> [OneNET - 设备属性上报](https://open.iot.10086.cn/doc/v5/fuse/detail/902)

通过 topic 发布长 MQTT 消息。如果您发布消息的数据量相对较少，不大于单条 AT 命令的长度阈值 `256` 字节，也可以使用 [AT+MQTTPUB](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#cmd-mqttpub) 命令。

```c
AT+MQTTPUBRAW=<LinkID>,<"topic">,<length>,<qos>,<retain>
//响应：
OK
>
```

符号 `>` 表示 AT 准备好接收串口数据，此时您可以输入数据，当数据长度达到参数 `<length>` 的值时，数据传输开始。

```c
//若传输成功，则 AT 返回：
+MQTTPUB:OK
```

```c
//若传输失败，则 AT 返回：
+MQTTPUB:FAIL
```

示例：

```c
AT+MQTTPUBRAW=0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post",146,0,0
//响应：
OK

>
{"id":"123","version":"1.0","params":{"currentTemperature":{"value":22,"time":1747458287111},"currenthumidity":{"value":33,"time":1747458287111}}}
//响应：
busy p...
+MQTTPUB:OK
    
//若订阅了该模块的消息网站会回复：
+MQTTSUBRECV:0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply",39,{"id":"123","code":200,"msg":"success"}

//打开网站查看设备详情里的属性栏，可见数据点值刷新，推送成功
```



---

### [断开 MQTT 连接](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttclean-mqtt)

断开 MQTT 连接，释放资源。

```c
AT+MQTTCLEAN=<LinkID>
AT+MQTTCLEAN=0
//响应：
OK

//打开云平台，可见设备变为离线
```

- **<LinkID>**：当前仅支持 link ID 0。



## 消息回复类型

> [AT消息](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/index.html#at-messages)

从 ESP-AT 命令端口返回的 ESP-AT 消息有两种类型：ESP-AT 响应（被动）和 ESP-AT 消息报告（主动）。

- ESP-AT 响应（被动）

	每个输入的 ESP-AT 命令都会返回响应，告诉发送者 ESP-AT 命令的执行结果。响应的最后一条消息必然是 `OK` 或者 `ERROR`。

	| AT 响应               | 说明                                                         |
	| --------------------- | ------------------------------------------------------------ |
	| OK                    | AT 命令处理完毕，返回 OK                                     |
	| ERROR                 | AT 命令错误或执行过程中发生错误                              |
	| SEND OK               | 数据已发送到协议栈（针对于 [AT+CIPSEND](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/TCP-IP_AT_Commands.html#cmd-send) 和 [AT+CIPSENDEX](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/TCP-IP_AT_Commands.html#cmd-sendex) 命令），但不代表数据已经发到对端 |
	| SEND FAIL             | 向协议栈发送数据时发生错误（针对于 [AT+CIPSEND](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/TCP-IP_AT_Commands.html#cmd-send) 和 [AT+CIPSENDEX](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/TCP-IP_AT_Commands.html#cmd-sendex) 命令） |
	| SET OK                | URL 已经成功设置（针对于 [AT+HTTPURLCFG](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/HTTP_AT_Commands.html#cmd-httpurlcfg) 命令） |
	| +<Command Name>:`...` | 详细描述 AT 命令处理结果                                     |

- ESP-AT 消息报告（主动）

	ESP-AT 会报告系统中重要的状态变化或消息。

	| ESP-AT 消息报告                                              | 说明                                                         |
	| ------------------------------------------------------------ | ------------------------------------------------------------ |
	| ready                                                        | ESP-AT 固件已经准备就绪                                      |
	| busy p…                                                      | 系统繁忙，正在处理上一条命令，无法处理新的命令               |
	| ERR CODE:`<0x%08x>`                                          | 不同命令的错误代码                                           |
	| Will force to restart!!!                                     | 立即重启模块                                                 |
	| smartconfig type:`<xxx>`                                     | Smartconfig 类型                                             |
	| Smart get wifi info                                          | Smartconfig 已获取 SSID 和 PASSWORD                          |
	| +SCRD:`<length>`,``<reserved data>``                         | ESP-Touch v2 已获取自定义数据                                |
	| smartconfig connected wifi                                   | Smartconfig 完成，ESP-AT 已连接到 Wi-Fi                      |
	| WIFI CONNECTED                                               | Wi-Fi station 接口已连接到 AP                                |
	| WIFI GOT IP                                                  | Wi-Fi station 接口已获取 IPv4 地址                           |
	| WIFI GOT IPv6 LL                                             | Wi-Fi station 接口已获取 IPv6 链路本地地址                   |
	| WIFI GOT IPv6 GL                                             | Wi-Fi station 接口已获取 IPv6 全局地址                       |
	| WIFI DISCONNECT                                              | Wi-Fi station 接口已与 AP 断开连接                           |
	| +ETH_CONNECTED                                               | 以太网接口已连接                                             |
	| +ETH_GOT_IP                                                  | 以太网接口已获取 IPv4 地址                                   |
	| +ETH_DISCONNECTED                                            | 以太网接口已断开                                             |
	| [<conn_id>,]CONNECT                                          | ID 为 `<conn_id>` 的网络连接已建立（默认情况下，ID 为 0）    |
	| [<conn_id>,]CLOSED                                           | ID 为 `<conn_id>` 的网络连接已断开（默认情况下，ID 为 0）    |
	| +LINK_CONN                                                   | TCP/UDP/SSL 连接的详细信息                                   |
	| +STA_CONNECTED: <sta_mac>                                    | station 已连接到 ESP-AT 的 Wi-Fi softAP 接口                 |
	| +DIST_STA_IP: <sta_mac>,<sta_ip>                             | ESP-AT 的 Wi-Fi softAP 接口给 station 分配 IP 地址           |
	| +STA_DISCONNECTED: <sta_mac>                                 | station 与 ESP-AT 的 Wi-Fi softAP 接口的连接断开             |
	| >                                                            | ESP-AT 正在等待用户输入数据                                  |
	| Recv `<xxx>` bytes                                           | ESP-AT 从命令端口已接收到 `<xxx>` 字节                       |
	| +IPD                                                         | ESP-AT 在非透传模式下，已收到来自网络的数据。有以下的消息格式：如果 AT+CIPMUX=0，AT+CIPRECVTYPE=1，打印：`+IPD,<length>`如果 AT+CIPMUX=1，AT+CIPRECVTYPE=<link_id>,1，打印：`+IPD,<link_id>,<length>`如果 AT+CIPMUX=0，AT+CIPRECVTYPE=0，AT+CIPDINFO=0，打印：`+IPD,<length>:<data>`如果 AT+CIPMUX=1，AT+CIPRECVTYPE=<link_id>,0，AT+CIPDINFO=0，打印：`+IPD,<link_id>,<length>:<data>`如果 AT+CIPMUX=0，AT+CIPRECVTYPE=0，AT+CIPDINFO=1，打印：`+IPD,<length>,<"remote_ip">,<remote_port>:<data>`如果 AT+CIPMUX=1，AT+CIPRECVTYPE=<link_id>,0，AT+CIPDINFO=1，打印：`+IPD,<link_id>,<length>,<"remote_ip">,<remote_port>:<data>`其中的 `link_id` 为连接 ID，`length` 为数据长度，`remote_ip` 为远端 IP 地址，`remote_port` 为远端端口号，`data` 为数据。注意：当这是个 SSL 连接时，在被动接收模式下（AT+CIPRECVTYPE=1），AT 命令口回复的 `length` 可能和实际可读的 SSL 数据长度不一致。因为 AT 会优先返回 SSL 层可读的数据长度，如果 SSL 层可读的数据长度为 0，AT 会返回套接字层可读的数据长度。 |
	| [透传模式](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/index_of_abbreviations.html#term-6) 下的数据 | ESP-AT 在透传模式下，已收到来自网络或蓝牙的数据              |
	| SEND Canceled                                                | 取消在 Wi-Fi [普通传输模式](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/index_of_abbreviations.html#term-5) 下发送数据 |
	| Have `<xxx>` Connections                                     | 已达到服务器的最大连接数                                     |
	| +QUITT                                                       | ESP-AT 退出 Wi-Fi [透传模式](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/index_of_abbreviations.html#term-6) |
	| NO CERT FOUND                                                | 在自定义分区中没有找到有效的设备证书                         |
	| NO PRVT_KEY FOUND                                            | 在自定义分区中没有找到有效的私钥                             |
	| NO CA FOUND                                                  | 在自定义分区中没有找到有效的 CA 证书                         |
	| +TIME_UPDATED                                                | 系统时间已更新。只在发送 [AT+CIPSNTPCFG](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/TCP-IP_AT_Commands.html#cmd-sntpcfg) 命令后或者掉电重启后，系统从 SNTP 服务器获取到新的时间，才会打印此消息。 |
	| +MQTTCONNECTED                                               | MQTT 已连接到 broker                                         |
	| +MQTTDISCONNECTED                                            | MQTT 与 broker 已断开连接                                    |
	| +MQTTSUBRECV                                                 | MQTT 已从 broker 收到数据                                    |
	| +MQTTPUB:FAIL                                                | MQTT 发布数据失败                                            |
	| +MQTTPUB:OK                                                  | MQTT 发布数据完成                                            |
	| +BLECONN                                                     | Bluetooth LE 连接已建立                                      |
	| +BLEDISCONN                                                  | Bluetooth LE 连接已断开                                      |
	| +READ                                                        | 通过 Bluetooth LE 连接进行读取操作                           |
	| +WRITE                                                       | 通过 Bluetooth LE 进行写入操作                               |
	| +NOTIFY                                                      | 来自 Bluetooth LE 连接的 notification                        |
	| +INDICATE                                                    | 来自 Bluetooth LE 连接的 indication                          |
	| +BLESECNTFYKEY                                               | Bluetooth LE SMP 密钥                                        |
	| +BLESECREQ:<conn_index>                                      | 收到来自 Bluetooth LE 连接的加密配对请求                     |
	| +BLEAUTHCMPL:<conn_index>,<enc_result>                       | Bluetooth LE SMP 配对完成                                    |
	| +BLUFIDATA:<len>,<data>                                      | ESP 设备收到从手机端发送的 BluFi 用户自定义数据              |
	| +WS_DISCONNECTED:<link_id>                                   | 连接 ID 为 <link_id> 的 WebSocket 连接已断开                 |
	| +WS_CONNECTED:<link_id>                                      | 连接 ID 为 <link_id> 的 WebSocket 连接已建立                 |
	| +WS_DATA:<link_id>,<data_len>,<data>                         | 连接 ID 为 <link_id> 的 WebSocket 连接收到数据               |
	| +WS_CLOSED:<link_id>                                         | 连接 ID 为 <link_id> 的 WebSocket 连接已关闭                 |
	| +BLESCANDONE                                                 | 扫描结束                                                     |
	| +BLESECKEYREQ:<conn_index>                                   | 对端已经接受配对请求，ESP 设备可以输入密钥了                 |