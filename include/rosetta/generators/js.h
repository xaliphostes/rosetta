/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include "details/js/js_generator.h"
#include "details//js/js_arrays.h"
#include "details/js/js_common.h"
#include "details/js/js_functions.h"
#include "details/js/js_functors.h"
#include "details/js/js_pointers.h"
#include "details/js/js_vectors.h"

#define BEGIN_JS(generatorName)                              \
    Napi::Object Init(Napi::Env env, Napi::Object exports) { \
        rosetta::JsGenerator generatorName(env, exports);

#define END_JS()    \
    return exports; \
    }               \
    NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)