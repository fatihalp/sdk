# 介绍节目单在xml的结构构成

例子:

```xml
<?xml version='1.0' encoding='utf-8'?>
<sdk guid="##GUID">
    <in method="AddProgram">
        <screen timeStamps="0">
            <program guid="##program-guid" type="normal">
                <backgroundMusic/>
                <playControl count="1" disabled="false"/>
                <area alpha="255" guid="##area-guid">
                    <rectangle x="0" height="128" width="128" y="0"/>
                    <resources>
                        <text guid="text-guid" singleLine="false">
                            <style valign="middle" align="center"/>
                            <string>例子</string>
                            <font name="SimSun" italic="false" bold="false" underline="false" size="28" color="#ffffff"/>
                        </text>
                    </resources>
                </area>
            </program>
        </screen>
    </in>
</sdk>

```

