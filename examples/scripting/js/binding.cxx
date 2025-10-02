#include <rosetta/JsGenerator.h>
#include "../demo.h"
#include <napi.h>

// Node.js addon initialization - ONLY 3 LINES for both classes!
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    rosetta::JsGenerator generator(env, exports);
    generator.bind_classes<Person, Vehicle>();
    generator.add_utilities();
    return exports;
}

NODE_API_MODULE(jsrosetta, Init)
