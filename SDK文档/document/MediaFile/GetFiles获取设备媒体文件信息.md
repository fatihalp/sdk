# GetFiles获取设备媒体文件信息

```xml
<!--请求-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetFiles"/>
</sdk>

<!--反馈
##existSize 文件在下位机存在的大小
##md5 文件的md5值
##name 文件名
##size 文件大小
##type 类型
0-图片 1-视频 2-字体 3-固件 4-fpga配置文件 
5-设置配置文件 6-项目资源 7-数据 8-临时文件
128- 临时图片文件 129-临时视频文件
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="41f8d777a9d83984e49fce04ea4c77d0">
    <out method="GetFiles" result="kSuccess">
        <files>
            <file existSize="##existSize" type="##type" md5="##md5" name="##name" size="##size"/>
        </files>
    </out>
</sdk>

```

