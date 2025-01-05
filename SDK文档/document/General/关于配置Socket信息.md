# 关于配置Socket信息 (勿操作)

正常情况无需修改. 尽量不修改它, 避免导致Socket被设备不断关闭

## GetSocketTimeInfo获取Socket信息

```xml
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetSocketTimeInfo"/>
</sdk>
```

## SetSocketTimeInfo设置Socket信息

```xml
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="SetSocketTimeInfo">
        <heartbeat timeout="##timeout" delay="##delay"/>
        <conncet delay="##delay"/>
    </in>
</sdk>

```

