## 通信主题

物联网平台中，服务端和设备端通过通信主题topic实现消息通信，设备可以通过发布消息到系统 topic 调用服务接口，也可以订阅系统 topic 用于接收服务消息通知，服务提供的系统 topic 见Topic列表。

## 主题列表

物联网平台预定义物模型通信topic，各物模型功能topic消息的数据格式，见OneJSON数据格式，另外为兼容旧设备保留了数据流，同步命令以及设备镜像相关的topic。
topic以正斜线（/）进行分层，区分每个类目。其中，有两个类目为既定类目：{pid}表示产品的产品id；{device-name}表示设备名称；{identifier}表示服务标识符，{cmdId}为平台生成命令id。

**tips**：主题订阅支持通配符“+”（单层）和“#”（多层），实现同时订阅多个主题消息，例如：

1、订阅全部物模型相关主题：$sys/{pid}/{device-name}/thing/#

2、订阅物模型属性类相关主题：$sys/{pid}/{device-name}/thing/property/#

3、订阅物模型服务调用类相关主题：
$sys/{pid}/{device-name}/thing/service/#

或者同时订阅$sys/{pid}/{device-name}/thing/service/+/invoke、$sys/{pid}/{device-name}/thing/service/+/invoke_reply

4、订阅数据流模式下的命令下发：

$sys/{pid}/{device-name}/cmd/#

或者同时订阅$sys/{pid}/{device-name}/cmd/request/+、$sys/{pid}/{device-name}/cmd/response/+/accepted、$sys/{pid}/{device-name}/cmd/response/+/rejected



#### 物模型通信主题

##### 直连/网关设备

**1. 属性**

| 功能                   | 主题                                                         | 操作权限 |
| :--------------------- | :----------------------------------------------------------- | :------- |
| 设备属性上报请求       | $sys/{pid}/{device-name}/thing/property/post                 | 发布     |
| 设备属性上报响应       | $sys/{pid}/{device-name}/thing/property/post/reply           | 订阅     |
| 设备属性设置请求       | $sys/{pid}/{device-name}/thing/property/set                  | 订阅     |
| 设备属性设置响应       | $sys/{pid}/{device-name}/thing/property/set_reply            | 发布     |
| 设备获取属性期望值请求 | $sys/{pid}/{device-name}/thing/property/desired/get          | 发布     |
| 设备获取属性期望值响应 | $sys/{pid}/{device-name}/thing/property/desired/get/reply    | 订阅     |
| 设备清除属性期望值请求 | $sys/{pid}/{device-name}/thing/property/desired/delete       | 发布     |
| 设备清除属性期望值响应 | $sys/{pid}/{device-name}/thing/property/desired/delete/reply | 订阅     |
| 设备属性获取请求       | $sys/{pid}/{device-name}/thing/property/get                  | 订阅     |
| 设备属性获取响应       | $sys/{pid}/{device-name}/thing/property/get_reply            | 发布     |

**2. 事件**

| 功能             | 主题                                            | 操作权限 |
| :--------------- | :---------------------------------------------- | :------- |
| 设备事件上报请求 | $sys/{pid}/{device-name}/thing/event/post       | 发布     |
| 设备事件上报响应 | $sys/{pid}/{device-name}/thing/event/post/reply | 订阅     |

**3. 服务**

| 功能             | 主题                                                         | 操作权限 |
| :--------------- | :----------------------------------------------------------- | :------- |
| 设备服务调用请求 | $sys/{pid}/{device-name}/thing/service/{identifier}/invoke   | 订阅     |
| 设备服务调用响应 | $sys/{pid}/{device-name}/thing/service/{identifier}/invoke_reply | 发布     |

**4. 脚本透传模式**

| 功能                 | 主题                                            | 操作权限 |
| :------------------- | :---------------------------------------------- | :------- |
| 脚本解析数据上行请求 | $sys/{pid}/{device-name}/custome/up             | 发布     |
| 脚本解析数据上行响应 | $sys/{pid}/{device-name}/custome/up_reply       | 订阅     |
| 脚本解析数据下行请求 | $sys/{pid}/{device-name}/custome/down/{id}      | 订阅     |
| 脚本解析数据下行响应 | $sys/{pid}/{devicename}/custome/down_reply/{id} | 发布     |

------


#### 数据流、同步命令及设备镜像通信主题

**1. 设备数据上传**

| 功能                   | 主题                                           | 操作权限 |
| :--------------------- | :--------------------------------------------- | :------- |
| 设备上传数据点请求     | $sys/{pid}/{device-name}/dp/post/json          | 发布     |
| 设备上传数据点成功响应 | $sys/{pid}/{device-name}/dp/post/json/accepted | 订阅     |
| 设备上传数据点失败响应 | $sys/{pid}/{device-name}/dp/post/json/rejected | 订阅     |

**2. 设备命令下发**

| 功能                 | 主题                                             | 操作权限 |
| :------------------- | :----------------------------------------------- | :------- |
| 设备同步命令请求     | $sys/{pid}/{device-name}/cmd/request/+           | 订阅     |
| 设备同步命令响应     | $sys/{pid}/{device-name}/cmd/response/{cmdId}    | 发布     |
| 设备同步命令响应成功 | $sys/{pid}/{device-name}/cmd/response/+/accepted | 订阅     |
| 设备同步命令响应失败 | $sys/{pid}/{device-name}/cmd/response/+/rejected | 订阅     |

**tips**：如果要订阅全部命令下发主题，可以使用：$sys/{pid}/{device-name}/cmd/#

**3. 设备镜像**

| 功能                  | 主题                                           | 操作权限 |
| :-------------------- | :--------------------------------------------- | :------- |
| 设备更新镜像请求      | $sys/{pid}/{device-name}/image/update          | 发布     |
| 设备更新镜像成功响应  | $sys/{pid}/{device-name}/image/update/accepted | 订阅     |
| 设备更新镜像失败响应  | $sys/{pid}/{device-name}/image/update/rejected | 订阅     |
| 设备获取镜像请求      | $sys/{pid}/{device-name}/image/get             | 订阅     |
| 设备获取镜像成功响应  | $sys/{pid}/{device-name}/image/get/accepted    | 订阅     |
| 设备获取镜像失败响应  | $sys/{pid}/{device-name}/image/get/rejected    | 订阅     |
| 设备镜像delta推送请求 | $sys/{pid}/{device-name}/image/update/delta    | 订阅     |