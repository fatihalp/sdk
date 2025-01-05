# 获取当前播放节目的GUID

```xml
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetCurrentPlayProgramGUID"/>
</sdk>

<!--
##program.guid 当前播放的节目的guid
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="fafaed9fdb9668a5bb06c01f55e95518">
    <out result="kSuccess" method="GetCurrentPlayProgramGUID">
        <program guid="##guid"/>
    </out>
</sdk>
```

