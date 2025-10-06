#include <rosetta/rosetta.h>
#include <rosetta/generators/js.h>
#include <rosetta/enum_registry.h>

// Method 1: Simple macro (works on most compilers for enum class)
enum class Status { Active = 0, Inactive = 1, Pending = 2 };
BEGIN_ENUM_REGISTRATION(Status)
    ENUM_VALUE(Active)
    ENUM_VALUE(Inactive)
    ENUM_VALUE(Pending)
END_ENUM_REGISTRATION()
//REGISTER_ENUM_3(Status, Active, Inactive, Pending);

// Method 2: Alternative syntax using BEGIN/END (guaranteed to work)
enum class Priority { Low = 0, Medium = 1, High = 2, Critical = 3 };
REGISTER_ENUM_4(Priority, Low, Medium, High, Critical);
// BEGIN_ENUM_REGISTRATION(Priority)
// ENUM_VALUE(Low)
// ENUM_VALUE(Medium)
// ENUM_VALUE(High)
// ENUM_VALUE(Critical)
// END_ENUM_REGISTRATION()

// For plain enums (not enum class), use REGISTER_PLAIN_ENUM:
// enum Color { Red, Green, Blue };
// REGISTER_PLAIN_ENUM(Color, Red, Green, Blue);

// Use in a class
class Task : public rosetta::Introspectable {
    INTROSPECTABLE(Task)
public:
    Task() : status_(Status::Pending), priority_(Priority::Medium) {}

    Status getStatus() const { return status_; }
    void   setStatus(Status s) { status_ = s; }

    Priority getPriority() const { return priority_; }
    void     setPriority(Priority p) { priority_ = p; }

    std::string getStatusName() const {
        const auto *info = rosetta::EnumRegistry::instance().getEnumInfo<Status>();
        return info->getName(static_cast<int64_t>(status_));
    }

private:
    Status   status_;
    Priority priority_;
};

void Task::registerIntrospection(rosetta::TypeRegistrar<Task> reg) {
    reg.constructor<>()
        .member("status", &Task::status_)
        .member("priority", &Task::priority_)
        .method("getStatus", &Task::getStatus)
        .method("setStatus", &Task::setStatus)
        .method("getPriority", &Task::getPriority)
        .method("setPriority", &Task::setPriority)
        .method("getStatusName", &Task::getStatusName);
}

// JavaScript binding
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    rosetta::JsGenerator generator(env, exports);

    // Register enums first
    rosetta::registerEnumType<Status>(generator);
    rosetta::registerEnumType<Priority>(generator);

    // Then bind class
    generator.bind_class<Task>();

    return exports;
}

NODE_API_MODULE(task_module, Init)