# SDK固件升级

## ExcuteUpgradeShell固件升级

```xml
<!--
固件升级请求
##name 为固件名, 请确保该固件已上传到设备
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
  	<in method="ExcuteUpgradeShell">
		<file name="##name"/>
  	</in>
</sdk>

<!--
固件升级反馈
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
    <out method="ExcuteUpgradeShell" result="kSuccess"/>
</sdk>
```

## ExcuteUpgradeShellHttp固件升级

> ExcuteUpgradeShellHttp支持http下载固件后进行升级

```xml
<!--
固件升级请求
##name 为固件名或者http的路径
##md5 固件的md5值
##size 固件的大小
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
  	<in method="ExcuteUpgradeShellHttp">
		<file name="##name" md5="##md5" size="##size"/>
  	</in>
</sdk>

<!--
固件升级反馈
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
    <out method="ExcuteUpgradeShellHttp" result="kSuccess"/>
</sdk>
```

## FirmwareUpgrade固件升级

> 和ExcuteUpgradeShellHttp相同, 返回下载中的状态值不同.

```xml
<!--
固件升级请求
##name 为固件名或者http的路径
##md5 固件的md5值
##size 固件的大小
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
  	<in method="FirmwareUpgrade">
		<file name="##name" md5="##md5" size="##size"/>
  	</in>
</sdk>

<!--
固件升级反馈
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
    <out method="FirmwareUpgrade" result="kSuccess"/>
</sdk>
```

## GetUpgradeResult获取固件升级状态

```xml
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
    <in method="GetUpgradeResult"/>
</sdk>

<!--
固件升级反馈
##value 为"success"(升级成功), "failed"(升级失败), "upgrading"(升级中) , "none"(没有升级)
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
    <out method="GetUpgradeResult" result="kSuccess">
        <upgrade result="##value"/>
    </out>
</sdk>
```

