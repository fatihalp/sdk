package sdk.demo;

import cn.huidu.sdk.component.CommonResult;
import cn.huidu.sdk.component.HDLuminanceMode;
import cn.huidu.sdk.dto.*;
import cn.huidu.sdk.server.HDSdkRequest;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Random;

import static cn.huidu.sdk.component.ScreenRotation.ANGLE_0;
import static cn.huidu.sdk.component.TimeInfoSync.NETWORK;

/**
 * 测试 api函数 demo
 *
 */
@RestController
public class HDHttpApiTest {

    /**
     * api函数 接口（仅方便展示 HDSdkRequest api调用方式 demo使用 不是规定sdk只能通过此方式使用）
     *
     * @param sn                    设备编号
     * @param type                  指令类型
     * @param xml                   节目xml(仅测试节目、自定义内容使用)
     * @return                       API是否调用成功
     */
    @GetMapping("/hd/api/send")
    public CommonResult apiSend(String sn, String type, String xml) {
        boolean apiUsedSuccess = false;
        switch (type) {
            // 获取在线设备列表
            case "DeviceList":
                return CommonResult.success(HDSdkRequest.getDevices());
            // 获取网络信息
            case "GetNetworkInfo":
                apiUsedSuccess = HDSdkRequest.getNetworkInfo(sn);
                break;
            // 开屏
            case "OpenScreen":
                apiUsedSuccess = HDSdkRequest.openScreen(sn);
                break;
            // 关屏
            case "CloseScreen":
                apiUsedSuccess = HDSdkRequest.closeScreen(sn);
                break;
            // 获取开关屏信息
            case "GetSwitchTime":
                apiUsedSuccess = HDSdkRequest.getSwitchTime(sn);
                break;
            // 设置开关屏信息
            case "SetSwitchTime":
                HDSwitchTimeParam hdSwitchTimeParam = new HDSwitchTimeParam();
                // 设置参数 ......
                apiUsedSuccess = HDSdkRequest.setSwitchTime(sn, hdSwitchTimeParam);
                break;
            // 截图
            case "GetScreenshot2":
                apiUsedSuccess = HDSdkRequest.getScreenshot2(sn);
                break;
            // 获取设备名
            case "GetDeviceName":
                apiUsedSuccess = HDSdkRequest.getDeviceName(sn);
                break;
            // 重启设备
            case "Reboot":
                apiUsedSuccess = HDSdkRequest.reboot(sn);
                break;
            // 获取亮度配置
            case "GetLuminancePloy":
                apiUsedSuccess = HDSdkRequest.getLuminancePloy(sn);
                break;
            // 获取多屏同步标志
            case "GetMulScreenSync":
                apiUsedSuccess = HDSdkRequest.getMulScreenSync(sn);
                break;
            // 获取时间校正数据
            case "GetTimeInfo":
                apiUsedSuccess = HDSdkRequest.getTimeInfo(sn);
                break;
            // 获取系统音量
            case "GetSystemVolume":
                apiUsedSuccess = HDSdkRequest.getSystemVolume(sn);
                break;
            // 屏幕旋转
         //   case "ScreenRotation":
          //      apiUsedSuccess = HDSdkRequest.screenRotation(sn, ANGLE_0);
          //      break;
            // 时间校正功能
            case "SetTimeInfo":
                HDTimeInfo hdTimeInfo = new HDTimeInfo();
                // 设置参数 ......
                apiUsedSuccess = HDSdkRequest.setTimeInfo(sn, hdTimeInfo);
                break;
            // 设置多屏同步
            case "SetMulScreenSync":
                apiUsedSuccess = HDSdkRequest.setMulScreenSync(sn, true);
                break;
            // 设置系统音量
            case "SetSystemVolume":
                HDSystemVolumeParam hdSystemVolumeParam = new HDSystemVolumeParam();
                // 设置参数 ......
                apiUsedSuccess = HDSdkRequest.setSystemVolume(sn, hdSystemVolumeParam);
                break;
            // 设置亮度配置
            case "SetLuminancePloy":
                HDLuminancePloyParam hdLuminancePloyParam = new HDLuminancePloyParam();
                // 设置参数 ......
                apiUsedSuccess = HDSdkRequest.setLuminancePloy(sn, hdLuminancePloyParam);
                break;
            //  固件升级
            case "ExcuteUpgradeShellHttp":
                HDFileInfo fileInfo = new HDFileInfo();
                // 设置参数 ......
                apiUsedSuccess = HDSdkRequest.httpUpgrade(sn, fileInfo);
                break;
            // 设置设备名
            case "SetDeviceName":
                apiUsedSuccess = HDSdkRequest.setDeviceName(sn, "newName");
                break;
            // 获取当前播放节目的GUID
            case "GetCurrentPlayProgramGUID":
                apiUsedSuccess = HDSdkRequest.getCurrentPlayProgramGUID(sn);
                break;
            // 获取设备信息
            case "GetDeviceInfo":
                apiUsedSuccess = HDSdkRequest.getDeviceInfo(sn);
                break;
            // 发节目 按照协议文档 编辑xml
            case "program":
                apiUsedSuccess = HDSdkRequest.sendProgram(sn, xml);
                break;
            case "custom":
                apiUsedSuccess = HDSdkRequest.sendCustomData(sn, xml);
                break;
            default:
                System.out.println("not support api");
        }

        return CommonResult.success(apiUsedSuccess);
    }


}
