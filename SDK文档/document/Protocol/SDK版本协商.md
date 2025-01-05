## SDK版本协商

用于首次SDK协议版本协商进行的SDK接口数据, 因为guid值还不清楚.

在当前连接的SDK通信, **sdk标签中的guid属性** 必须是**SDK版本协商**返回的那个**guid**值.

## SDK版本协商请求

要求:

1. 在协商完传输协议版本后, 需发送下面SDK版本请求给下位机进行SDK版本协商
2. 该SDK只有value属性的值可变, 其他均为固定值, 现在使用 1000000

```xml
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="##GUID">
  	<in method="GetIFVersion">
		<version value="1000000"/>
  	</in>
</sdk>
```

## SDK版本协商反馈

反馈信息:

1. **sdk**标签的**guid**属性的**值**, 这个值是当前连接**后续**进行**SDK协议交互**使用, 需记录保留
2. out标签的result属性的值, 这个值表示状态值, 如kSuccess表示正常状态, 详情可到[错误码定义](./错误码定义.md)查看

```xml
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="f7b3b451c4f3cf3310e2d6714fbb50e8">
  	<out method="GetIFVersion" result="kSuccess">
		<version value="1000000"/>
  	</out>
</sdk>
```

