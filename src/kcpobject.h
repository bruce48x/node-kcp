#ifndef KCPOBJECT_H
#define KCPOBJECT_H

#define NAPI_VERSION 8
#include <napi.h>
#include "ikcp.h"

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

    static int kcp_output(const char *buf, int len, ikcpcb *kcp, void *user);

    ikcpcb *kcp;
    Napi::Env env;
    Napi::FunctionReference output;
    Napi::ObjectReference context;
    char *recvBuff = NULL;
    unsigned int recvBuffSize = 1024;
};

#endif