# DeleteFiles从设备删除文件

兼容旧版本, 增加md5值, 准确删除指定文件

> 不指定md5值, 和旧版本相同, 但碰到相同name文件时, 删除第一个

```xml
<!-- 旧版本, 删除首个相同name文件-->
<?xml version="1.0"?>
<sdk guid="##GUID">
    <in method="DeleteFiles">
        <files>
            <file name="##name"/>
        </files>
    </in>
</sdk>


<!-- 新增加属性, 准确删除指定文件-->
<?xml version="1.0"?>
<sdk guid="##GUID">
    <in method="DeleteFiles">
        <files>
            <file name="##name" md5="##md5"/>
        </files>
    </in>
</sdk>

<!--两者反馈
删除成功则没有files标签和子标签, 存在删除文件未找到的则返回files标签, 子标签为未找到的文件
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="d77658a8565669346a60862dba08f2e1">
    <out result="kSuccess" method="DeleteFiles">
        <files>
            <file result="kFileNotFound" name="##name"/>
        </files>
    </out>
</sdk>
```

## 根据md5删除文件

使用DeleteFiles相同SDK, 这个是新增功能, 可不使用name属性, 而是使用md5删除文件.

```xml
<!-- 只使用md5删除文件 -->
<?xml version="1.0"?>
<sdk guid="##GUID">
    <in method="DeleteFiles">
        <files>
            <file md5="##md5"/>
        </files>
    </in>
</sdk>

<!--反馈
删除成功则没有files标签和子标签, 存在删除文件未找到的则返回files标签, 子标签为未找到的文件
-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="d77658a8565669346a60862dba08f2e1">
    <out result="kSuccess" method="DeleteFiles">
        <files>
            <file result="kFileNotFound" md5="##md5"/>
        </files>
    </out>
</sdk>
```

