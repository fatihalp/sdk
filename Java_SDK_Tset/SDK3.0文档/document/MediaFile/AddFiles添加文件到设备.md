# AddFiles添加文件到设备

设备支持http下载

```xml
<!--请求
##type image和video
##size 文件的大小
##md5 文件的md5值
##name 文件名
##remote 资源位置, http://... https://...
-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="AddFiles">
        <files>
            <file type="##type" size="##size" md5="##md5" name="##name" remote="remote"/>
        </files>
    </in>
</sdk>

<!--反馈-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="41f8d777a9d83984e49fce04ea4c77d0">
    <out method="AddFiles" result="kSuccess">
        <files/>
    </out>
</sdk>
```

