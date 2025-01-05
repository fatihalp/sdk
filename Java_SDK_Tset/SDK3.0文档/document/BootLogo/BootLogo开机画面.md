# BootLogo开机画面

开机画面相关SDK

## GetBootLogo获取开机画面状态

```xml
<!--请求-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetBootLogo"/>
</sdk>

<!--反馈
##exist 是否设置了开机画面{"true": 已设置, "false": 未设置}
##md5 开机图片的md5值
##name 开机图片名
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="0a35b47e0821c4ec26d3075b9737a3b5">
    <out method="GetBootLogo" result="kSuccess">
        <logo exist="##exist" md5="##md5" name="##name"/>
    </out>
</sdk>
```

## **SetBo**otLogoName设置开机画面

```xml
<!--请求
##md5 开机图片的md5值, 可为空
##name 开机图片名
-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="SetBootLogoName">
        <logo md5="##value" name="##name"/>
    </in>
</sdk>

<!--反馈-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="0a35b47e0821c4ec26d3075b9737a3b5">
    <out method="SetBootLogoName" result="kSuccess"/>
</sdk>
```

## ClearBootLogo清除开机画面

```xml
<!--请求-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="ClearBootLogo"/>
</sdk>

<!--反馈-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="0a35b47e0821c4ec26d3075b9737a3b5">
    <out method="ClearBootLogo" result="kSuccess"/>
</sdk>
```