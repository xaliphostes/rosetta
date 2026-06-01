// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// REST backend: implements the rosetta::walk visitor concept.
//
// Routes registered for each bound type T (base path = "/T" by default):
//   POST   /T                          -> create instance, returns {"id": N}
//   GET    /T/{id}                     -> full state as JSON
//   DELETE /T/{id}                     -> destroy instance
//   GET    /T/{id}/<field>             -> read field as JSON
//   PUT    /T/{id}/<field>             -> write field (JSON body)
//   POST   /T/{id}/<method>            -> invoke instance method (JSON array body)
//   POST   /T/<static_method>          -> invoke static method (JSON array body)
//
// Annotations behaviour:
//   readonly      -> PUT returns 403
//   range{lo,hi}  -> PUT returns 400 if value out of bounds
//   doc           -> currently ignored (would feed an OpenAPI generator)
//
// The implementation lives in inline/rest_visitor.hxx.

#pragma once

#include <atomic>
#include <experimental/meta>
#include <httplib.h>
#include <mutex>
#include <nlohmann/json.hpp>
#include <rosetta/walk.h>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rosetta {

    template <typename T> class Store; // defined in inline/rest_visitor.hxx
    template <typename T> void bind_rest(httplib::Server &, const std::string &, Store<T> &);

}

#include "inline/rest_visitor.hxx"
