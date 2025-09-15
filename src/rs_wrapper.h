#ifndef RSWRAPPER_H
#define RSWRAPPER_H

#include <napi.h>
extern "C" {
    #include <correct/reed-solomon.h>
}

class ReedSolomon : public Napi::ObjectWrap<ReedSolomon> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    explicit ReedSolomon(const Napi::CallbackInfo &info);
    ~ReedSolomon();

private:
    correct_reed_solomon* rs;
    Napi::Value Encode(const Napi::CallbackInfo& info);
    Napi::Value Decode(const Napi::CallbackInfo& info);
};

#endif