#include <rosetta/JsGenerator.h>
#include "../demo.h"

// Node.js addon initialization - ONLY 3 LINES for both classes!
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    rosetta::JsGenerator(env, exports).bind_classes<Person, Vehicle>().add_utilities();
    return exports;
}

NODE_API_MODULE(jsrosetta, Init)
