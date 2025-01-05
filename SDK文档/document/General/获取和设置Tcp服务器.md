# 获取和设置Tcp服务器

## GetSDKTcpServer获取Tcp服务器信息

```xml
<!--请求-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetSDKTcpServer"/>
</sdk>

<!--反馈
##port 端口
##host 主机地址
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="dedef080ba2082110c73404cceb17135">
    <out result="kSuccess" method="GetSDKTcpServer">
        <server port="##port" host="##host"/>
    </out>
</sdk>
```

## SetSDKTcpServer设置Tcp服务器

> 用于需要设备向tcp服务器进行连接, 而不是主动向设备连接的功能

```xml
<!--请求
##port 端口
##host 主机地址
-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="SetSDKTcpServer">
        <server port="##port" host="##host"/>
    </in>
</sdk>

<!--反馈-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="dedef080ba2082110c73404cceb17135">
    <out result="kSuccess" method="SetSDKTcpServer"/>
</sdk>
```

