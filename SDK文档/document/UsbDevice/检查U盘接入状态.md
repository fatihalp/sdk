# 检查U盘接入状态 (安卓系列不支持)

## CheckUDiskInsert检查U盘是否接入

```xml
<!--请求-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="CheckUDiskInsert"/>
</sdk>

<!--反馈
##value {"true"接入 "false"未接入}
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="7a8fe8bb88ccfab7dc57adc9c5f70e2e">
    <out result="kSuccess" method="CheckUDiskInsert">
        <insert value="##value"/>
    </out>
</sdk>
```

