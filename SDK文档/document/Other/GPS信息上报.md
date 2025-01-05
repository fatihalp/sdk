# GPS信息上报

## GetGpsRespondEnable获取GPS信息上报使能标志

```xml
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetGpsRespondEnable"/>
</sdk>

<!--反馈
##enable 是否使能 {"true", "false"}
##delay GPS信息上传的间隔 (单位:秒)
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="7a8fe8bb88ccfab7dc57adc9c5f70e2e">
    <out result="kSuccess" method="GetGpsRespondEnable">
        <gps enable="##enable" delay="##delay"/>
    </out>
</sdk>
```

## SetGpsRespondEnable设置GPS信息上报使能标志

```xml
<!--请求
##enable 是否使能 {"true", "false"}
##delay GPS信息上传的间隔 (单位:秒)
-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="SetGpsRespondEnable">
        <gps enable="##enable" delay="##delay"/>
    </in>
</sdk>

<!--反馈-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="7a8fe8bb88ccfab7dc57adc9c5f70e2e">
    <out result="kSuccess" method="SetGpsRespondEnable"/>
</sdk>
```

## GPS信息报文

当二次开发用户调用“**SetGpsRespondEnable**”接口打开GPS信息上报使能标志后，当控制卡与服务器建立好TCP连接后即会按照二次开发用户设置的间隔时间往服务器周期性的主动上传GPS信息报文。

| 字段   | 长度(Len) | 命令(Cmd)      | 纬度     | 经度      | 时间    | 保留     |
| ------ | --------- | -------------- | -------- | --------- | ------- | -------- |
| 字节数 | 2字节     | 2字节          | 4字节    | 4字节     | 4字节   | 4字节    |
| 取值   | 20        | kGPSInfoAnswer | latitude | longitude | seconds | reserved |

```c++
typedef struct HGPSInfoAnswer 
{
    huint16 len;    	///<命令包长度 
    huint16 cmd;    	///<命令值 
    hfloat  latitude;  	///<纬度 
    hfloat  longitude; 	///<经度 
    huint32 seconds;  	///<时间 
    huint32 reserved;  	///<保留 
}  HGPSInfoAnswer; 
/**  字段说明
 * 1. latitude: 纬度; > 0表示北半球; < 0表示南半球
 * 2. longitude: 经度; >0表示东半球; < 0表示西半球
 * 3. seconds: 距离1970年1月1日 00:00:00的秒数
 * 4. reserved: 保留
 */
```

