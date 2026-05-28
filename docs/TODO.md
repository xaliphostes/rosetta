# Entities not visited at all

- Enums. There's no walk_enum<E> and no enum branch in walk<T>. You can't iterate enumerators_of(^^E) to emit IntEnum / TS
unions / REST enum: [...].
- Base classes. nonstatic_data_members_of(^^T, ctx) returns only members declared in T. Inherited fields/methods are silently
dropped — fine for POD-like records, broken for any polymorphic hierarchy. You need bases_of(^^T) plus either a recursive walk  
or a walk_inherited entrypoint.  
- Constructors. Filtered out by is_exportable_member_function. But every backend (pybind11 .def(py::init<…>()), Node .New(...), 
REST POST /T) needs the constructor signatures. Likely walk should expose them as a separate ctor<Fn> visitor call. 
- Static data members. nonstatic_data_members_of skips them by definition, and there's no field_static visitor signature.  
- Nested types (nested classes/structs/enums, type aliases). A Point::Coord enum nested in Point is invisible.
- Free functions / namespace-scope variables. walk is T-rooted. A walk_namespace<^^my::ns>(v) would let you bind module-level
helpers.
- Conversion operators and overloaded operators (operator(), operator+, operator[], …). operator() in particular is functor  
reflection — the project's prompt explicitly calls it out. The current filter rejects nothing for these by name, but there's no 
visitor signature distinguishing them, so backends can't route operator+ → __add__. 
  
# Details ignored about what is visited  
 
- Method qualifiers. A const method, an &&-ref-qualified method, a virtual method, a noexcept method all reach
method_instance<Fn> identically. Some backends care (pybind11 needs to know const-ness for py::const_; REST binding for safe vs.
unsafe verbs).  
- Parameter metadata. Parameter names and default arguments aren't surfaced. Python **kwargs-style binding really wants both.
identifier_of(param) and has_default_argument(param) exist; nothing currently propagates them. 
- Per-parameter annotations. [[=range{0,1}]] double t on a parameter is invisible — same plumbing as field annotations needs to 
repeat there. 
- Bit-fields, mutable, anonymous unions. Niche but real; a generator that claims "full reflection" should at least flag them so
backends can refuse cleanly rather than miscompile.  
- Return-type metadata. The backend re-derives return_type_of(Fn) itself, fine — but [[nodiscard]], ref/cv qualifiers, and
noexcept get lost unless surfaced. 
- Field traits. Is the type a std::optional, std::variant, container, smart pointer, or raw pointer? Each backend re-discovers
this; centralising it in the walker (or in a tiny shape-classifier) would deduplicate a lot of backend code.  
  
# The annotation-routing design itself  

This is the structural issue, not a missing item. Today the walker decides for each field which of three shapes to call
(field_plain / field_readonly / field_ranged). Every new annotation kind (alias, widget, deprecated, unit, …) requires:

1. a new shape on the visitor concept,
2. a new branch in the walker,
3. an update in every backend.

That's O(annotations × backends) churn. A better pivot: hand the visitor the annotation pack and let it decide.

v.template field<fld, Anns...>(name);// Anns... is the full annotation pack
v.template method<fn,  Anns...>(name);
v.template ctor<fn, Anns...>();
v.template enumerator<e, Anns...>(name);

Then field_readonly / field_ranged become helper queries the visitor can call on Anns..., not separate ABI surfaces of the
walker. Same shape covers any future annotation — widget, group, alias, deprecated — with zero walker changes.

# A practical "what's next" punch list

1. Annotation-pack refactor first — it changes the visitor signature, so doing it later means rewriting backends twice.
2. Enums — independent entrypoint, big payoff per LOC.
3. Constructors — every backend needs them.
4. Bases / inherited members — flips a lot of "doesn't work on real code" into "works."
5. Method qualifiers + parameter names/defaults.
6. Operators & static fields / nested types.

The first one is the only architectural change; the rest are additive and can land in any order.