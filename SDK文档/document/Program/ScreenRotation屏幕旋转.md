# ScreenRotation屏幕旋转

设置屏幕旋转时, 会导致宽高调整和节目重播, 请不要在多屏同步时旋转.

> rotation标签的value值对应顺时针旋转
>
> 0 -> 0°
>
> 1 -> 90°
>
> 2 -> 180°
>
> 3 -> 270°

```xml
<!--
屏幕旋转请求
##value [0-3], 不属于均返回 kInvalidXmlIndex
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
    <in method="ScreenRotation">
        <rotation value="##value"/>
    </in>
</sdk>

<!--
屏幕旋转反馈
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="d77e142ea74b179d648e497ea820e6e2">
    <out result="kSuccess" method="ScreenRotation"/>
</sdk>
```

