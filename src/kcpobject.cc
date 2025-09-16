#include <iostream>
#include "ikcp.h"
#include "kcpobject.h"
#include "encoding.h"

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

    FEC fec = thiz->fec;
    if (fec.isEnabled())
    {
        // extend to len + fecHeaderSizePlus2
        // i.e. 4B seqid + 2B flag + 2B size
        memcpy(thiz->outBuffer.data() + fecHeaderSizePlus2, buf, static_cast<size_t>(len));
        fec.MarkData(reinterpret_cast<byte *>(thiz->outBuffer.data()), static_cast<uint16_t>(len));
        thiz->output_wrapper(thiz, thiz->outBuffer.data(), len + fecHeaderSizePlus2);

        // FEC calculation
        // copy "2B size + data" to shards
        auto slen = len + 2;
        thiz->shards[thiz->pkt_idx] =
            std::make_shared<std::vector<byte>>(&thiz->outBuffer[fecHeaderSize], &thiz->outBuffer[fecHeaderSize + slen]);

        // count number of data shards
        thiz->pkt_idx++;
        if (thiz->pkt_idx == thiz->dataShards)
        { // we've collected enough data shards
            fec.Encode(thiz->shards);
            // send parity shards
            for (size_t i = thiz->dataShards; i < thiz->dataShards + thiz->parityShards; i++)
            {
                // append header to parity shards
                // i.e. fecHeaderSize + data(2B size included)
                memcpy(thiz->outBuffer.data() + fecHeaderSize, thiz->shards[i]->data(), thiz->shards[i]->size());
                fec.MarkFEC(reinterpret_cast<byte *>(thiz->outBuffer.data()));
                thiz->output_wrapper(thiz, thiz->outBuffer.data(), thiz->shards[i]->size() + fecHeaderSize);
            }

            // reset indexing
            thiz->pkt_idx = 0;
        }
    }
    else
    {
        thiz->output_wrapper(thiz, buf, len);
    }

    return len;
}

void KcpObject::output_wrapper(KcpObject *thiz, const char *buf, int len)
{
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
}

KcpObject::KcpObject(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<KcpObject>(info),
      env(info.Env()),
      kcp(nullptr, ikcp_release) // RAII: custom deleter for ikcpcb
{
    try
    {
        IUINT32 conv = info[0].As<Napi::Number>().Uint32Value();

        // Create KCP with RAII management
        ikcpcb *raw_kcp = ikcp_create(conv, this);
        if (!raw_kcp)
        {
            throw std::runtime_error("Failed to create KCP instance");
        }
        kcp.reset(raw_kcp);
        kcp->output = KcpObject::kcp_output;

        if (!info[1].IsEmpty())
        {
            context = Napi::Persistent(info[1].As<Napi::Object>());
        }

        // Initialize receive buffer with RAII
        recvBuffer.resize(recvBuffSize);

        // 判断是否有FEC参数
        if (!context.IsEmpty() && context.Value().Has("fec"))
        {
            Napi::Object fecOpts = context.Value().Get("fec").As<Napi::Object>();
            dataShards = fecOpts.Get("dataShards").As<Napi::Number>().Uint32Value();
            parityShards = fecOpts.Get("parityShards").As<Napi::Number>().Uint32Value();
            fec = FEC::New(3 * (dataShards + parityShards), dataShards, parityShards);
            shards.resize(dataShards + parityShards, nullptr);

            // Initialize output buffer with RAII
            outBuffer.resize((dataShards + parityShards) * 1024);
        }
    }
    catch (const std::exception &e)
    {
        Napi::Error::New(env, std::string("KCP initialization failed: ") + e.what())
            .ThrowAsJavaScriptException();
        throw;
    }
}

KcpObject::~KcpObject()
{
    // RAII handles all cleanup automatically
    // No need to call release() explicitly
}

void KcpObject::Release(const Napi::CallbackInfo &info)
{
    // RAII handles cleanup automatically
    // Just reset the unique_ptr to trigger cleanup
    kcp.reset();

    // Clear buffers (vectors will automatically clean up)
    recvBuffer.clear();
    outBuffer.clear();
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
        bufsize = ikcp_peeksize(this->kcp.get());
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
            // RAII: resize vector instead of realloc
            try
            {
                this->recvBuffer.resize(allsize);
                this->recvBuffSize = allsize;
            }
            catch (const std::bad_alloc &e)
            {
                len = 0;
                Napi::Error::New(env, "Buffer resize failed: out of memory").ThrowAsJavaScriptException();
                break;
            }
        }

        buflen = ikcp_recv(this->kcp.get(), this->recvBuffer.data() + len, bufsize);
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
        return Napi::Buffer<char>::Copy(env, this->recvBuffer.data(), len);
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
    if (fec.isEnabled())
    {
        // decode FEC packet
        auto pkt = fec.Decode(reinterpret_cast<byte *>(buff.Data()), buff.ByteLength());
        if (pkt.flag == typeData)
        {
            auto ptr = pkt.data->data();
            // we have 2B size, ignore for typeData
            int t = ikcp_input(this->kcp.get(), (char *)(ptr + 2), pkt.data->size() - 2);
            return Napi::Number::New(env, t);
        }

        // allow FEC packet processing with correct flags.
        if (pkt.flag == typeData || pkt.flag == typeFEC)
        {
            // input to FEC, and see if we can recover data.
            auto recovered = fec.Input(pkt);

            // we have some data recovered.
            int t = 0;
            for (auto &r : recovered)
            {
                // recovered data has at least 2B size.
                if (r->size() > 2)
                {
                    auto ptr = r->data();
                    // decode packet size, which is also recovered.
                    uint16_t sz;
                    decode16u(ptr, &sz);

                    // the recovered packet size must be in the correct range.
                    if (sz >= 2 && sz <= r->size())
                    {
                        // input proper data to kcp
                        t += ikcp_input(this->kcp.get(), (char *)(ptr + 2), sz - 2);
                        // std::cout << "sz:" << sz << std::endl;
                    }
                }
            }
            return Napi::Number::New(env, t);
        }
        return Napi::Number::New(env, -1);
    }
    else
    {
        int t = ikcp_input(this->kcp.get(), buff.Data(), buff.ByteLength());
        return Napi::Number::New(env, t);
    }
}

Napi::Value KcpObject::Send(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!info[0].IsBuffer())
    {
        Napi::Error::New(env, "Send data type must be buffer").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    Napi::Buffer<char> buff = info[0].As<Napi::Buffer<char>>();
    int len = buff.ByteLength();
    if (len < 1)
    {
        Napi::Error::New(env, "Send len error").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    int t = ikcp_send(this->kcp.get(), buff.Data(), len);
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

    IUINT32 current = info[0].As<Napi::Number>().Uint32Value();
    ikcp_update(this->kcp.get(), current);

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

    IUINT32 current = info[0].As<Napi::Number>().Uint32Value();
    IUINT32 ret = ikcp_check(this->kcp.get(), current) - current;
    return Napi::Number::New(env, ret);
}

Napi::Value KcpObject::Flush(const Napi::CallbackInfo &info)
{
    ikcp_flush(this->kcp.get());
    return info.Env().Undefined();
}

Napi::Value KcpObject::Peeksize(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    int ret = ikcp_peeksize(this->kcp.get());
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
    int ret = ikcp_setmtu(this->kcp.get(), mtu);
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
    int ret = ikcp_wndsize(this->kcp.get(), sndwnd, rcvwnd);
    return Napi::Number::New(env, ret);
}

Napi::Value KcpObject::Waitsnd(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    int ret = ikcp_waitsnd(this->kcp.get());
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
    int ret = ikcp_nodelay(this->kcp.get(), nodelay, interval, resend, nc);
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