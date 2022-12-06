#include <iostream>
#include "ikcp.h"
#include "kcpobject.h"

Napi::Object KcpObject::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func =
        DefineClass(env,
                    "KCP",
                    {InstanceMethod("release", &KcpObject::Release),
                     InstanceMethod("context", &KcpObject::GetContext),
                     InstanceMethod("recv", &KcpObject::Recv),
                     InstanceMethod("send", &KcpObject::Send),
                     InstanceMethod("input", &KcpObject::Input),
                     InstanceMethod("output", &KcpObject::Output),
                     InstanceMethod("update", &KcpObject::Update),
                     InstanceMethod("check", &KcpObject::Check),
                     InstanceMethod("flush", &KcpObject::Flush),
                     InstanceMethod("peeksize", &KcpObject::Peeksize),
                     InstanceMethod("setmtu", &KcpObject::Setmtu),
                     InstanceMethod("wndsize", &KcpObject::Wndsize),
                     InstanceMethod("waitsnd", &KcpObject::Waitsnd),
                     InstanceMethod("nodelay", &KcpObject::Nodelay),
                     InstanceMethod("stream", &KcpObject::Stream)});

    Napi::FunctionReference *constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);

    exports.Set("KCP", func);
    return exports;
}

int KcpObject::kcp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    KcpObject *thiz = (KcpObject *)user;
    if (!thiz->output)
    {
        return len;
    }

    Napi::Buffer<char> buffer = Napi::Buffer<char>::Copy(thiz->env, buf, len);
    Napi::Number length = Napi::Number::New(thiz->env, len);
    if (thiz->context)
    {
        thiz->output.Call({buffer, length, thiz->context.Value()});
    }
    else
    {
        thiz->output.Call({buffer, length});
    }
    return len;
}

KcpObject::KcpObject(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<KcpObject>(info), env(info.Env())
{
    IUINT32 conv = info[0].As<Napi::Number>().Uint32Value();
    kcp = ikcp_create(conv, this);
    kcp->output = KcpObject::kcp_output;
    if (!info[1].IsEmpty())
    {
        context = Napi::Persistent(info[1].As<Napi::Object>());
    }
    recvBuff = (char *)realloc(recvBuff, recvBuffSize);
}

KcpObject::~KcpObject()
{
    if (kcp)
    {
        ikcp_release(kcp);
        kcp = NULL;
    }
    if (recvBuff)
    {
        free(recvBuff);
        recvBuff = NULL;
    }
}

void KcpObject::Release(const Napi::CallbackInfo &info)
{
    if (this->kcp)
    {
        ikcp_release(this->kcp);
        this->kcp = NULL;
    }
    if (this->recvBuff)
    {
        free(this->recvBuff);
        this->recvBuff = NULL;
    }
}

Napi::Value KcpObject::GetContext(const Napi::CallbackInfo &info)
{
    return this->context.Value();
}

Napi::Value KcpObject::Recv(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    int bufsize = 0;
    unsigned int allsize = 0;
    int buflen = 0;
    unsigned int len = 0;
    while (1)
    {
        bufsize = ikcp_peeksize(this->kcp);
        if (bufsize <= 0)
        {
            break;
        }
        allsize += bufsize;
        if (allsize > this->recvBuffSize)
        {
            int align = allsize % 4;
            if (align)
            {
                allsize += 4 - align;
            }
            this->recvBuffSize = allsize;
            this->recvBuff = (char *)realloc(this->recvBuff, this->recvBuffSize);
            if (!this->recvBuff)
            {
                len = 0;
                Napi::Error::New(env, "realloc error").ThrowAsJavaScriptException();
                break;
            }
        }

        buflen = ikcp_recv(this->kcp, this->recvBuff + len, bufsize);
        if (buflen <= 0)
        {
            break;
        }
        len += buflen;
        if (this->kcp->stream == 0)
        {
            break;
        }
    }
    if (len > 0)
    {
        return Napi::Buffer<char>::Copy(env, this->recvBuff, len);
    }

    return env.Undefined();
}

Napi::Value KcpObject::Input(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!info[0].IsBuffer())
    {
        Napi::Error::New(env, "input must be buffer").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    Napi::Buffer<char> buff = info[0].As<Napi::Buffer<char>>();
    int t = ikcp_input(this->kcp, buff.Data(), buff.ByteLength());
    return Napi::Number::New(env, t);
}

Napi::Value KcpObject::Send(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    int len = 0;
    Napi::Value arg0 = info[0];
    Napi::String str;
    if (arg0.IsString())
    {
        str = arg0.ToString();
    }
    else if (arg0.IsBuffer())
    {
        str = arg0.As<Napi::Buffer<char>>().ToString();
    }
    else
    {
        Napi::Error::New(env, "arg type error").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    std::string stdstr = (std::string)str;
    const char *data = stdstr.data();
    len = stdstr.length();
    if (len == 0)
    {
        Napi::Error::New(env, "Send len error").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    int t = ikcp_send(this->kcp, data, len);
    return Napi::Number::New(env, t);
}

Napi::Value KcpObject::Output(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!info[0].IsFunction())
    {
        Napi::Error::New(env, "callback must be a function").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    this->output = Napi::Persistent(info[0].As<Napi::Function>());

    return env.Undefined();
}

Napi::Value KcpObject::Update(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!info[0].IsNumber())
    {
        Napi::Error::New(env, "KCP update() first argument must be number").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    int arg0 = info[0].As<Napi::Number>().Int32Value();
    IUINT32 current = (IUINT32)(arg0 & 0xfffffffful);
    ikcp_update(this->kcp, current);

    return env.Undefined();
}

Napi::Value KcpObject::Check(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!info[0].IsNumber())
    {
        Napi::Error::New(env, "KCP check() first argument must be number").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    int arg0 = info[0].As<Napi::Number>().Int32Value();
    IUINT32 current = (IUINT32)(arg0 & 0xfffffffful);
    IUINT32 ret = ikcp_check(this->kcp, current) - current;
    return Napi::Number::New(env, ret);
}

Napi::Value KcpObject::Flush(const Napi::CallbackInfo &info)
{
    ikcp_flush(this->kcp);
    return info.Env().Undefined();
}

Napi::Value KcpObject::Peeksize(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    int ret = ikcp_peeksize(this->kcp);
    return Napi::Number::New(env, ret);
}

Napi::Value KcpObject::Setmtu(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    int mtu = 1400;
    if (info[0].IsNumber())
    {
        mtu = info[0].As<Napi::Number>().Int32Value();
    }
    int ret = ikcp_setmtu(this->kcp, mtu);
    return Napi::Number::New(env, ret);
}

Napi::Value KcpObject::Wndsize(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    int sndwnd = 32;
    int rcvwnd = 32;
    if (info[0].IsNumber())
    {
        sndwnd = info[0].As<Napi::Number>().Int32Value();
    }
    if (info[1].IsNumber())
    {
        rcvwnd = info[1].As<Napi::Number>().Int32Value();
    }
    int ret = ikcp_wndsize(this->kcp, sndwnd, rcvwnd);
    return Napi::Number::New(env, ret);
}

Napi::Value KcpObject::Waitsnd(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    int ret = ikcp_waitsnd(this->kcp);
    return Napi::Number::New(env, ret);
}

Napi::Value KcpObject::Nodelay(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    int nodelay = 0;
    int interval = 100;
    int resend = 0;
    int nc = 0;
    if (info[0].IsNumber())
    {
        nodelay = info[0].As<Napi::Number>().Int32Value();
    }
    if (info[1].IsNumber())
    {
        interval = info[1].As<Napi::Number>().Int32Value();
    }
    if (info[2].IsNumber())
    {
        resend = info[2].As<Napi::Number>().Int32Value();
    }
    if (info[3].IsNumber())
    {
        nc = info[3].As<Napi::Number>().Int32Value();
    }
    int ret = ikcp_nodelay(this->kcp, nodelay, interval, resend, nc);
    return Napi::Number::New(env, ret);
}

Napi::Value KcpObject::Stream(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info[0].IsNumber())
    {
        int stream = info[0].As<Napi::Number>().Int32Value();
        this->kcp->stream = stream;
        return Napi::Number::New(env, this->kcp->stream);
    }
    return env.Undefined();
}