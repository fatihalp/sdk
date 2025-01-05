

#include <HCatOpenSSL.h>
#include <vector>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <signal.h>

#define BUFF_MAX    (1024 * 9)


using namespace cat;



std::unique_ptr<HCatOpenSSL> HCatOpenSSL::instance_;


static void FreeContent(SSL_CTX *p) {
    if (p) {
        SSL_CTX_free(p);
    }
}

static void FreeSSL(SSL *p) {
    if (p) {
        SSL_shutdown(p);
        SSL_free(p);
    }
}


HCatOpenSSL *HCatOpenSSL::GetInstance()
{
    if (!instance_) {
#if defined(SIGPIPE) && !defined(_WIN32)
        signal(SIGPIPE, SIG_IGN);
#endif
        instance_.reset(new HCatOpenSSL());
    }

    return instance_.get();
}

HCatOpenSSL::~HCatOpenSSL()
{
    EVP_cleanup();
}


HCatOpenSSL::CtxPtr HCatOpenSSL::CreateMethodContent()
{
    return std::shared_ptr<SSL_CTX>(SSL_CTX_new(TLS_method()), FreeContent);
}

HCatOpenSSL::CtxPtr HCatOpenSSL::CreateClientMethodContent()
{
    return std::shared_ptr<SSL_CTX>(SSL_CTX_new(TLS_client_method()), FreeContent);
}

HCatOpenSSL::CtxPtr HCatOpenSSL::CreateSetviceMethodContent()
{
    return std::shared_ptr<SSL_CTX>(SSL_CTX_new(TLS_server_method()), FreeContent);
}

HCatOpenSSL::SSLPtr HCatOpenSSL::CreateSSL(SSL_CTX *ctx)
{
    return SSLPtr(SSL_new(ctx), FreeSSL);
}


int HCatOpenSSL::SetSSLFd(SSL *ssl, int fd)
{
    return SSL_set_fd(ssl, fd);
}

int HCatOpenSSL::SSLConnect(SSL *ssl)
{
    for (;;) {
        int status = SSL_connect(ssl);
        if (status == 1) {
            return 1;
        }

        if (status == -1) {
            int code = SSL_get_error(ssl, status);
            if (SSL_ERROR_WANT_READ == code || SSL_ERROR_WANT_WRITE == code) {
                //需要更多时间来进行握手
                continue;
            }
            return kError;
        } else {
            return kError;
        }
    }

    return kError;
}

int HCatOpenSSL::SSLDisconnect(SSL *ssl)
{
    return SSL_shutdown(ssl);
}

int HCatOpenSSL::SSLAccept(SSL *ssl)
{
    return SSL_accept(ssl);
}


int HCatOpenSSL::Write(SSL *ssl, const HCatBuffer &buff)
{
    if (buff.Empty()) {
        return 0;
    }

    for (;;) {
        int size = SSL_write(ssl, buff.Data(), buff.Size());
        if (size > 0) {
            return size;
        }

        int code = SSL_get_error(ssl, size);
        switch (code) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            break;
        default:
            return kError;
            break;
        }
    }

    return kError;
}

int HCatOpenSSL::Read(SSL *ssl, HCatBuffer &buff, int count)
{
    std::vector<char> readBuff(BUFF_MAX, 0);
    for (int i = 0;; ++i) {
        int size = SSL_read(ssl, readBuff.data(), readBuff.size());
        if (size > 0) {
            buff = HCatBuffer(readBuff.data(), size);
            return size;
        }

        int code = SSL_get_error(ssl, size);
        switch (code) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            if (i >= count) {
                return kRetryFaild;
            }
            break;
        default:
            return kError;
            break;
        }
    }

    return kError;
}

HCatOpenSSL::HCatOpenSSL()
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}
