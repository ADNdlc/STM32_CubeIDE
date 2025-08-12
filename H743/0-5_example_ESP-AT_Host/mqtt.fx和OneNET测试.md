# mqtt.fx和OneNET测试

> 参考内容
> [MQTT篇目录](http://www.taichi-maker.com/homepage/esp8266-nodemcu-iot/iot-tuttorial/mqtt-tutorial/)



## oneNET云平台
进入云平台，在头像左边进入开发者中心

左边栏选择产品开发，创建一个产品，品类任意，智能化方式为设备接入，接入协议MQTT，数据协议选择“OneJson”，联网方式蜂窝或WiFi，其余默认![image-20250518140327069](../../../AppData/Roaming/Typora/typora-user-images/image-20250518140327069.png)

 

点击刚创建的产品右边的产品开发，选择设置物模型->添加自定义功能点，设置此功能的 “标识符”((比如currentTemperature)，读写，其余按需求或随意。![image-20250518140337548](../../../AppData/Roaming/Typora/typora-user-images/image-20250518140337548.png)

 

 跳过其余，发布产品(删除要先撤销发布) ![image-20250518140347635](../../../AppData/Roaming/Typora/typora-user-images/image-20250518140347635.png)



添加一个设备，所属产品选择刚创建的产品，设置 “设备名称/ID” ![image-20250518140358747](../../../AppData/Roaming/Typora/typora-user-images/image-20250518140358747.png)

 

在设备管理里，查看刚刚创建的设备详情，可看到所属的 “产品ID”和此 “设备密钥”![image-20250518140406998](../../../AppData/Roaming/Typora/typora-user-images/image-20250518140406998.png)

***

## mqtt.fx

​	打开齿轮点，击左下角 + 新建一个mqtt连接
​		General里取消勾选  MQTT Version的  Use Default 选项
​		Broker Address填写OneNet的服务地址：mqtts.heclouds.com
​		端口：1883

![image-20250516173400948](../../../AppData/Roaming/Typora/typora-user-images/image-20250516173400948.png)

​	**CONNECT报文设置：**

​		Client ID填写云平台上的  “设备名称”

> [!NOTE]
> *OneNet只能使用系统Topic，格式由平台严格定义，设备只能发布或订阅与自身关联的Topic（包含自身设备名称的Topic），无法跨设备或跨产品操作*

​		Clean Session选项勾选，启用表示不重要的对话

​		User Credentials里：
​			User Name填写刚填写设备对应的“产品ID”如：SQKg9n0Ii0
​			password密码使用OneNET-token计算工具生成：
​				res:	oducts/SQKg9n0Ii0/devices/temperatureAndHumidity
​					      oducts/<产品ID>/devices/<设备名称>
​				et:	  当前时间戳(要改大一点，使用未来的时间戳)
​				key:	“设备密钥”
​				方法:      md5

​	设置好后点OK保存，点击 Connect 右侧圆点变绿，连接成功! 此时云平台设备应显示在线

***

## 客户端mqtt主题发布和订阅



![image-20250516174345176](../../../AppData/Roaming/Typora/typora-user-images/image-20250516174345176.png)





### SUBSCRIBE – 订阅主题：

> 网站开发文档
> [协议规范_  开发者文档_OneNET](https://open.iot.10086.cn/doc/mqtt/book/device-develop/protocol.html)
> [设备接入测试（MQTT协议）OneNET篇–CSDN博客](https://blog.csdn.net/qq_36347513/article/details/126433507)

- 平台支持connect、subscribe、publish、ping、unsubscribe、disconnect等报文
- 不支持pubrec、pubrel、pubcomp报文	

**mqtt.fx订阅OneNet操作：**

已连接成功，点击Subscribe，在下拉栏输入：
		$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply
订阅答复消息主题(数据上报和网站回复用的是不同主题)，当数据上传时服务器回复上传时间和相关消息：
17-05-2025 13:05:04.47104665	SoS……等…
{"id":"123","code":200,"msg":"success"}

---

### PUBLISH – 发布消息：

MQTT客户端发布消息时，会向服务端发送PUBLISH报文。

Publish报文结构：
![image-20250516174413096](../../../AppData/Roaming/Typora/typora-user-images/image-20250516174413096.png)

**retainFlag – 保留标志**
在默认情况下，当客户端订阅了某一主题后，并不会马上接收到该主题的信息。只有在，服务端接收到该主题的新信息时，服务端才会将最新接收到的该主题信息推送给客户端。retainFlag为ture时，服务端会将发布的数据内容存储，有新的订阅时会将记录的内容马上发给新的订阅者。



**mqtt.fx发布OneNet操作：**

已连接成功，点击Publish，在下拉栏输入：
		$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post

在发布内容处严格按照网站要求格式填写:	

```c
{
    "id": "123",
    "version": "1.0",
    "params": {
        "<功能点1的标识符>": {
            "value": 33,
            "time": 1747458287111
        },
        "功能点2的标识符": {
            "value": 66,
            "time": 1747458287111
        }   
    }
}
//注意time是毫秒，当前时间戳要增加三位
//id是此条消息的id
//空格和回车只是方便阅读，如下的格式也可以
//{"id":"123","version":"1.0","params":{"currentTemperature":{"value":22,"time":1747458287111},"currenthumidity":{"value":33,"time":1747458287111}}}
```

点击下拉框旁边Publish，回到Subscribe可看到回复消息，打开云平台，打开设备详情，查看属性栏，可看到两个功能点的数据已经刷新。





