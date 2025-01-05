# GetProgram获取的数据用于节目数据时常出现的问题

1. 常见出现黑屏问题: **resources** 标签不正确

> 使用 **GetProgram** 获取的节目数据中, 资源标签是 **resource**, 而添加节目和更新节目的资源标签是带 **s** 的 **resources** 如果资源标签错误. 那么将导致这个节目不存在. 只有区域节目. 这时就会显示只有黑屏了.

## GetProgram的数据

```xml
<!--这个是请求的数据-->
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="GetProgram"/>
</sdk>


<!--这个是返回的数据-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="15811595b4daf72fe80e7e078de6edd4">
    <out result="kSuccess" method="GetProgram">
        <screen ...>
            <program ...>
                <playControl count="1"/>
                <border .../>
                <area ...>
                    <rectangle height="32" x="0" y="0" width="32"/>
                    <border .../>
                    <!--把这些数据用于更新节目的数据时, 就是这个资源标签需要注意, 这个是没有s, 正确的更新数据的资源标签是有s的. resources这种-->
                    <resource>
                        <text ...>
                            <effect .../>
                            <style .../>
                            <string>test</string>
                            <font size="20" name="Arial" italic="false" underline="false" color="#ffff00" bold="true"/>
                        </text>
                    </resource>
                </area>
            </program>
        </screen>
    </out>
</sdk>
```

