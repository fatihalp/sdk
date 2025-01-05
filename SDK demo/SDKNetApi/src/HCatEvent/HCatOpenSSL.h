

#ifndef __HCATOPENSSL_H__
#define __HCATOPENSSL_H__


#include <memory>
#include <HCatBuffer.h>



typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;

namespace cat
{


class HCatOpenSSL
{
public:
    using CtxPtr = std::shared_ptr<SSL_CTX>;
    using CtxWeakPtr = std::weak_ptr<SSL_CTX>;

    using SSLPtr = std::shared_ptr<SSL>;
    using SSLWeakPtr = std::weak_ptr<SSL>;

    enum eStatus {
        kShutdown   = 0,    ///< 对端关闭
        kError      = -1,   ///< 错误
        kRetryFaild = -2,   ///< 超过次数
    };

public:
    // 使用单例就会忽略管道破裂的信号
    static HCatOpenSSL *GetInstance();
    ~HCatOpenSSL();

    // 创建ssl会话正文, 最后再释放会话
    static CtxPtr CreateMethodContent();
    static CtxPtr CreateClientMethodContent();
    static CtxPtr CreateSetviceMethodContent();

    // 创建ssl, 先释放ssl
    static SSLPtr CreateSSL(SSL_CTX *ctx);
    static SSLPtr CreateSSL(const CtxPtr &ctx) { return CreateSSL(ctx.get()); }

    // 将ssl和fd关联
    static int SetSSLFd(SSL *ssl, int fd);
    static int SetSSLFd(const SSLPtr &ssl, int fd) { return SetSSLFd(ssl.get(), fd); }

    static int SSLConnect(SSL *ssl);
    static int SSLConnect(const SSLPtr &ssl) { return SSLConnect(ssl.get()); }
    static int SSLDisconnect(SSL *ssl);
    static int SSLDisconnect(const SSLPtr &ssl) { return SSLDisconnect(ssl.get()); }
    static int SSLAccept(SSL *ssl);
    static int SSLAccept(const SSLPtr &ssl) { return SSLAccept(ssl.get()); }

    static int Write(SSL *ssl, const HCatBuffer &buff);
    static int Write(const SSLPtr &ssl, const HCatBuffer &buff) { return Write(ssl.get(), buff); }

    static int Read(SSL *ssl, HCatBuffer &buff, int count = 10);
    static int Read(const SSLPtr &ssl, HCatBuffer &buff, int count = 10) { return Read(ssl.get(), buff, count); }

private:
    HCatOpenSSL();

private:
    static std::unique_ptr<HCatOpenSSL> instance_;
};


}


#define OpenSSl() \
    cat::HCatOpenSSL::GetInstance()


#endif // __HCATOPENSSL_H__
