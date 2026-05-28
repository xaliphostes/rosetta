In C++26 (P2996), there's no direct "give me all free functions" call, but you can reach them by reflecting a namespace and filtering. The two gotchas: ^^foo is ill-formed when foo is overloaded, and function templates need substitute before you can call them.

## Basic pattern

```cpp
#include <experimental/meta>
namespace meta = std::meta;

namespace api {
    intadd(int a, int b){ return a + b; } 
    void greet(std::string_view s){ /* ... */ } 
} 

// Enumerate free functions in a namespace
constexpr auto fns = []{
    std::vector<meta::info> out;
    for (meta::info r : meta::members_of(^^api)) {
        if (meta::is_function(r)) out.push_back(r); 
        return out;
    }
}();
```

## What you can ask each reflection

```cpp
template <meta::info R> 
void describe() {
    constexpr auto name = meta::identifier_of(R); // "add"
    constexpr auto T= meta::type_of(R); // function type
    constexpr auto ret= meta::return_type_of(T);// ^^int
    constexpr auto ps = meta::parameters_of(R); // span<info> 
    // meta::display_string_of(R) gives a human-readable form 
}
```

## Calling a reflected function

Splice it back into an expression with `[: :]`:

```cpp
constexpr meta::info f = ^^api::add;
int x = [:f:](2, 3);// direct splice-call
int y = meta::reflect_invoke(f, {^^2, ^^3});// constexpr-only path
```

## Overloads

`^^api::add` fails if add is overloaded. Resolve by signature first:

```cpp
using sig = int(int,int);
constexpr meta::info f = ^^static_cast<sig*>(&api::add);
```

…or walk members_of and match on type_of.

## Function templates

```cpp
constexpr meta::info tpl= ^^my_template; // the template itself
constexpr meta::info inst = meta::substitute(tpl, {^^int});// my_template<int>
[:inst:](42);
```

## Typical use cases this unlocks

- Auto-generating RPC/CLI dispatch tables from a namespace's contents
- Building test runners that find every test_* function
- Producing serializers/bindings without macros