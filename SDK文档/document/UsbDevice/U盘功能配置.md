# U盘功能配置 (安卓系列不支持)

## GetEnableUDiskFunction获取U盘功能配置

```xml
<!--请求-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetEnableUDiskFunction"/>
</sdk>

<!--反馈
##effective.value 在U盘什么状态的时候允许被修改 {"all"所有状态 "insert"仅U盘接入时}
##play.video 和image两者需相同, 允许播放U盘根目录视频和图片 {"true" "false"}
##play.image 和video两者需相同
##import.config 允许导入配置参数 {"true" "false"}
##import.immeProgram 允许直接播放U盘节目 {"true" "false"}
##import.updateProgram 允许更新U盘节目 {"true" "false"}
##import.upgrade 允许升级U盘固件 {"true" "false"}
##storage.enable 允许U盘存储 {"true" "false"}
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="7a8fe8bb88ccfab7dc57adc9c5f70e2e">
    <out result="kSuccess" method="GetEnableUDiskFunction">
        <effective value="##value"/>
        <play video="##video" image="##image"/>
        <import config="##config" immeProgram="##immeProgram" updateProgram="##updateProgram" upgrade="##upgrade"/>
        <storage enable="##enable"/>
    </out>
</sdk>
```

## SetEnableUDiskFunction开启或设置U盘功能配置

```xml
<!--请求
##effective.value 在U盘什么状态的时候允许被修改 {"all"所有状态 "insert"仅U盘接入时}
##play.video 和image两者需相同, 允许播放U盘根目录视频和图片 {"true" "false"}
##play.image 和video两者需相同
##import.config 允许导入配置参数 {"true" "false"}
##import.immeProgram 允许直接播放U盘节目 {"true" "false"}
##import.updateProgram 允许更新U盘节目 {"true" "false"}
##import.upgrade 允许升级U盘固件 {"true" "false"}
##storage.enable 允许U盘存储 {"true" "false"}
-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="SetEnableUDiskFunction">
        <effective value="##value"/>
        <play image="##image" video="##video"/>
        <import upgrade="##upgrade" updateProgram="##updateProgram" config="##config" immeProgram="##immeProgram"/>
        <storage enable="##enable"/>
    </in>
</sdk>

<!--反馈-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="7a8fe8bb88ccfab7dc57adc9c5f70e2e">
    <out result="kSuccess" method="SetEnableUDiskFunction"/>
</sdk>
```

## DisableUDiskFunction关闭U盘功能配置

> 在使用SetEnableUDiskFunction时, 将自动开启U盘功能配置

```xml
<!--请求-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="DisableUDiskFunction"/>
</sdk>

<!--反馈-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="7a8fe8bb88ccfab7dc57adc9c5f70e2e">
    <out result="kSuccess" method="DisableUDiskFunction"/>
</sdk>
```

