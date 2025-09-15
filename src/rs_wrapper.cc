#include <napi.h>
#include <vector>
#include <sys/types.h>  // for ssize_t
#include "rs_wrapper.h"

Napi::Object ReedSolomon::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(env, "ReedSolomon", {
        InstanceMethod("encode", &ReedSolomon::Encode), 
        InstanceMethod("decode", &ReedSolomon::Decode)
    });

    Napi::FunctionReference *constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);

    exports.Set("ReedSolomon", func);
    return exports;
}

ReedSolomon::ReedSolomon(const Napi::CallbackInfo &info) : ObjectWrap<ReedSolomon>(info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber())
    {
        Napi::TypeError::New(env, "Expected (symsize, gfpoly)").ThrowAsJavaScriptException();
        return;
    }

    int symsize = info[0].As<Napi::Number>().Int32Value();
    int gfpoly = info[1].As<Napi::Number>().Int32Value();

    // 初始化 RS 编码器 (参数可根据需求调整)
    rs = correct_reed_solomon_create(symsize, gfpoly, 1, 32);
    if (!rs)
    {
        Napi::Error::New(env, "Failed to create Reed-Solomon encoder").ThrowAsJavaScriptException();
    }
}

ReedSolomon::~ReedSolomon()
{
    if (rs)
    {
        correct_reed_solomon_destroy(rs);
        rs = nullptr;
    }
}

Napi::Value ReedSolomon::Encode(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (!info[0].IsBuffer())
    {
        Napi::TypeError::New(env, "Expected a Buffer").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Buffer<uint8_t> input = info[0].As<Napi::Buffer<uint8_t>>();
    std::vector<uint8_t> output(input.Length() + 32); // 多留32字节冗余

    ssize_t written = correct_reed_solomon_encode(rs, input.Data(), input.Length(), output.data());

    return Napi::Buffer<uint8_t>::Copy(env, output.data(), written);
}

Napi::Value ReedSolomon::Decode(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (!info[0].IsBuffer())
    {
        Napi::TypeError::New(env, "Expected a Buffer").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Buffer<uint8_t> input = info[0].As<Napi::Buffer<uint8_t>>();
    std::vector<uint8_t> output(input.Length());

    ssize_t written = correct_reed_solomon_decode(rs, input.Data(), input.Length(), output.data());

    if (written < 0)
    {
        Napi::Error::New(env, "Reed-Solomon decode failed").ThrowAsJavaScriptException();
        return env.Null();
    }

    return Napi::Buffer<uint8_t>::Copy(env, output.data(), written);
}
