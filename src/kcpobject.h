#ifndef KCPOBJECT_H
#define KCPOBJECT_H

#define NAPI_VERSION 8
#include <napi.h>
#include <memory>
#include <vector>
#include "ikcp.h"
#include "fec.h"

class KcpObject : public Napi::ObjectWrap<KcpObject>
{
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    explicit KcpObject(const Napi::CallbackInfo &info);
    ~KcpObject();

private:
    void Release(const Napi::CallbackInfo &info);
    Napi::Value GetContext(const Napi::CallbackInfo &info);
    Napi::Value Recv(const Napi::CallbackInfo &info);
    Napi::Value Send(const Napi::CallbackInfo &info);
    Napi::Value Output(const Napi::CallbackInfo &info);
    Napi::Value Input(const Napi::CallbackInfo &info);
    Napi::Value Update(const Napi::CallbackInfo &info);
    Napi::Value Check(const Napi::CallbackInfo &info);
    Napi::Value Flush(const Napi::CallbackInfo &info);
    Napi::Value Peeksize(const Napi::CallbackInfo &info);
    Napi::Value Setmtu(const Napi::CallbackInfo &info);
    Napi::Value Wndsize(const Napi::CallbackInfo &info);
    Napi::Value Waitsnd(const Napi::CallbackInfo &info);
    Napi::Value Nodelay(const Napi::CallbackInfo &info);
    Napi::Value Stream(const Napi::CallbackInfo &info);
    Napi::Value Fec(const Napi::CallbackInfo &info);

    static int kcp_output(const char *buf, int len, ikcpcb *kcp, void *user);

    // RAII-managed resources
    std::unique_ptr<ikcpcb, void (*)(ikcpcb *)> kcp;
    Napi::Env env;
    Napi::FunctionReference output;
    Napi::ObjectReference context;

    // RAII-managed buffers using std::vector
    std::vector<char> recvBuffer;
    std::vector<char> outBuffer;

    // Buffer size tracking
    unsigned int recvBuffSize = 1024;

    void output_wrapper(KcpObject *thiz, const char *buf, int len);

    FEC fec;
    uint32_t pkt_idx{0};
    std::vector<row_type> shards;
    size_t dataShards{0};
    size_t parityShards{0};
};

#endif