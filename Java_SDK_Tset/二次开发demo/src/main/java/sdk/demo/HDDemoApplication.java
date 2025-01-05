package sdk.demo;


import cn.huidu.sdk.server.SdkMessageHandlerConfig;
import cn.huidu.sdk.server.SdkServer;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;


/**
 *  HD全彩 sdk 服务启动 demo
 *
 */
@SpringBootApplication
public class HDDemoApplication {

     public static void main(String[] args) {
          SpringApplication.run(HDDemoApplication.class, args);

          // 添加处理Event的handler
          SdkMessageHandlerConfig.initSdkMessage(new HDEventMessageListener());

          // 启动 sdk服务
          SdkServer server = new SdkServer();

          // port： 监听的tcp端口 keepLive：超时时间 单位秒
          server.startServer(1884, 120);

          // 服务停止
          Runtime.getRuntime().addShutdownHook(new Thread() {
               @Override
               public void run() {
                    //sdk 停止
                    server.stopServer();
               }
          });
     }
}
