# RealTimeUpdate区域更新SDK (安卓系列和直接使用UpdateProgram相同)

用于更新指定区域, 而不是更新当前节目的所有区域(当前支持文本和图片)

> 注: 有部分情况会导致更新该区域时判断节目需要全部区域重新播放时, 这时会重新播放全部区域

## 添加需要区域实时更新的节目演示

> 将需要修改的节目绑定上guid属性. 并给唯一的guid值, 为了不影响观看, 使用(...)省略无关属性, 演示如下:  
> 
> 1. 首先先添加节目
> 2. 给需要实时更新的节目添加guid, 可以看到text标签添加了guid属性. 这个在后续用于实时更新

```xml
<?xml version = "1.0" encoding = "utf-8" ?>
<sdk guid = "##GUID">
    <in method="AddProgram">
        <screen ...>
            <program ...>
                <playControl .../>
                <area ...>
                    <rectangle .../>
                    <resources>
                        <text guid="c544d34e-fd6c-11ea-8edc-00005f6af662" name="" singleLine="false">
                            <string>text区域加上guid属性</string>
                        </text>
                    </resources>
                </area>
            </program>
        </screen>
    </in>
</sdk>
```

<div STYLE="page-break-after: always;"></div>

## RealTimeUpdate 区域更新

区域更新要求guid必须对应的节目必须存在, 并且节目类型为同一类型, 否则会返回错误码

允许更新多节点. 在最新版中该功能是异步功能, 等节目那边更新后才会通知回馈.

如属于远程资源, 那么file标签必须填写远程资源的md5属性, 否则将反馈**kSeekFileFailed**

注: 当处于下载状态时. 而再进行发送一次需要资源下载的实时更新, 则会导致下载失败. 且这次的需要资源下载的任务则被取消. 

> text: 要求相同的guid即可更新
> 
> image: 要求file标签需要name和图片的md5属性, 允许只使用name属性, 但遇到相同name时只更新第一个

```xml
<!--
content的子节点是需要更新节目
##guid 属性对应需要更新的节目的guid, 要求两者必须相同, 不存在则返回错误码
##type 属性为local和remote, local表示资源在本地上, remote表示资源在需要下载, 此时文件标签必须存在name和md5属性
-->
<?xml version = "1.0" encoding = "utf-8" ?>
<sdk guid = "##GUID">
    <in method="RealTimeUpdate">
        <content guid="c544d34e-fd6c-11ea-8edc-00005f6af662" type="##type" >
            <text guid="c544d34e-fd6c-11ea-8edc-00005f6af662" singleLine="false">
                <string>Hello World</string>
                <font color="#0000ff"/>
            </text>
        </content>

        <content guid="6e55b916-a9c1-4522-bbfd-63e6c6a63436" type="local">
            <image guid="6e55b916-a9c1-4522-bbfd-63e6c6a63436" fit="stretch">
                <!-- 这个表示资源在本地的 -->
                <file name="test.jpg" md5="449fa54b8ad97ac56dd0f4b1d1f66dbd"/>
            </image>
        </content>

        <content guid="6e55b916-a9c1-4522-bbfd-63e6c6a63436" type="remote">
            <image guid="6e55b916-a9c1-4522-bbfd-63e6c6a63436" fit="stretch">
                <!-- 这个表示资源需要下载的, content的type属性是remote, 因需要下载资源, 此时file标签需要完备, 必须存在md5属性 -->
                <file name="http://127.0.0.1/test.jpg" md5="449fa54b8ad97ac56dd0f4b1d1f66dbd"/>
            </image>
        </content>

        <!-- 如还有需要更新的, 可继续添加content标签 -->
        <content guid="..." type="##type">
            ...
        </content>
    </in>
</sdk>


<!--反馈-->
<?xml version="1.0" encoding="utf-8"?>
<sdk guid="09e83c4a6fca57b429482ccbee93bd6a">
    <out method="RealTimeUpdate" result="kSuccess"/>
</sdk>
```
