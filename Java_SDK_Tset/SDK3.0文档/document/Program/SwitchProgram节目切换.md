# SwitchProgram节目切换

> 请确保关闭多屏同步

可以使用节目的guid或者使用索引来进行节目切换

```xml
<!--
节目切换请求, guid和index选一个属性即可
如guid属性和index属性只存在一个时
##guid 节目不存在该guid则返回 kInvalidXmlIndex
##index [0-max) 节目索引, 超出max时返回 kInvalidXmlIndex
如两者都存在时, guid属性优先级比index高, 当guid找不到时才会使用index
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
    <in method="SwitchProgram">
        <!-- guid属性和index属性选一个存在即可 -->
        <program guid="##guid" index="##index"/>
    </in>
</sdk>

<!--
节目切换反馈
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="d77e142ea74b179d648e497ea820e6e2">
    <out result="kSuccess" method="SwitchProgram"/>
</sdk>
```

