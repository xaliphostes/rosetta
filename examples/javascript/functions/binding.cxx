#include <cmath>
#include <rosetta/function_registry.h>
#include <rosetta/generators/js.h>

double calculateDistance(double x1, double y1, double x2, double y2) {
    return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

std::string greet(const std::string &name) {
    return "Hello, " + name;
}

int add(int a, int b) {
    return a + b;
}

REGISTER_FUNCTION(calculateDistance);
REGISTER_FUNCTION(greet);
REGISTER_FUNCTION(add);

// Bind ALL registered functions automatically!
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    rosetta::JsGenerator generator(env, exports);
    rosetta::bindAllFunctions(generator);
    return exports;
}

NODE_API_MODULE(rosetta, Init)