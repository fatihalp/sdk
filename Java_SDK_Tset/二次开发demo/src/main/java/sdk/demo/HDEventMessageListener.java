package sdk.demo;

import cn.huidu.sdk.dto.SdkPublishMessage;
import com.lmax.disruptor.EventHandler;


/**
 * 事件回调消息监听 demo
 *
 */
public class HDEventMessageListener implements EventHandler<SdkPublishMessage> {

    @Override
    public void onEvent(SdkPublishMessage sdkPublishMessage, long sequence, boolean endOfBatch) {
        // 接收消息
        try {
            // 业务代码.....
            System.err.println("sdk message : " + sdkPublishMessage.toString());
        } catch (Exception e) {
            // 注意 必须抓异常 否则影响后面消息监听！
        }
    }
}
