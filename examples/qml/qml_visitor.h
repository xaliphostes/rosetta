// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// QML backend: implements the rosetta::walk visitor concept.
//
// Provides:
//   - rosetta::QmlVisitor<T> — emits one row of metadata per field/method
//     into a ReflectedObject, plus a typed lambda for each getter/setter
//     and method invoker.
//   - rosetta::bind_qml<T>(obj, target) — entry point: runs the walk and
//     leaves `obj` ready to be exposed to QML via a context property.
//
// Annotation behaviour mirrors the other backends:
//   readonly      -> setField returns an error
//   range{lo,hi}  -> setField rejects out-of-range numeric values
//   doc           -> surfaced as `doc` on each field/method entry so the
//                    QML side can show it as a tooltip

#pragma once

#include "reflected_object.h"
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <experimental/meta>
#include <rosetta/walk.h>
#include <string>
#include <type_traits>
#include <utility>

namespace rosetta {

    namespace qml_detail {

        // Bridge std::string <-> QString; everything else round-trips via QVariant.
        template <typename F> QVariant to_variant(const F &v) {
            if constexpr (std::is_same_v<F, std::string>) {
                return QString::fromStdString(v);
            } else {
                return QVariant::fromValue(v);
            }
        }

        template <typename F> F from_variant(const QVariant &v) {
            if constexpr (std::is_same_v<F, std::string>) {
                return v.toString().toStdString();
            } else {
                return v.value<F>();
            }
        }

        // Coarse type tag for the QML side — enough for it to pick an editor.
        template <typename F> QString type_tag() {
            if constexpr (std::is_same_v<F, std::string>)
                return QStringLiteral("string");
            else if constexpr (std::is_same_v<F, bool>)
                return QStringLiteral("bool");
            else if constexpr (std::is_integral_v<F>)
                return QStringLiteral("int");
            else if constexpr (std::is_floating_point_v<F>)
                return QStringLiteral("double");
            else
                return QStringLiteral("other");
        }

        template <std::meta::info Fn, typename T, std::size_t... Is>
        QVariant invoke_method_impl(T &self, const QVariantList &args,
                                    std::index_sequence<Is...>) {
            using R               = [:std::meta::return_type_of(Fn):];
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));
            if constexpr (std::is_void_v<R>) {
                (self.[:Fn:])(
                    from_variant<
                        std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                        args[Is])...);
                return {};
            } else {
                R r = (self.[:Fn:])(
                    from_variant<
                        std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                        args[Is])...);
                return to_variant<R>(r);
            }
        }

        template <std::meta::info Fn, std::size_t... Is>
        QVariant invoke_static_impl(const QVariantList &args, std::index_sequence<Is...>) {
            using R               = [:std::meta::return_type_of(Fn):];
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));
            if constexpr (std::is_void_v<R>) {
                ([:Fn:])(
                    from_variant<
                        std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                        args[Is])...);
                return {};
            } else {
                R r = ([:Fn:])(
                    from_variant<
                        std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                        args[Is])...);
                return to_variant<R>(r);
            }
        }

    } // namespace qml_detail

    template <typename T> struct QmlVisitor {
        ReflectedObject *obj;
        T               *target;

        template <std::meta::info Fld, auto... Anns> void field(const char *name) {
            using F                 = [:std::meta::type_of(Fld):];
            constexpr auto dann     = ann::get_or<doc>(doc{""}, Anns...);
            const QString  qname    = QString::fromUtf8(name);

            QVariantMap info;
            info[QStringLiteral("name")]     = qname;
            info[QStringLiteral("type")]     = qml_detail::type_tag<F>();
            info[QStringLiteral("doc")]      = QString::fromUtf8(dann.text);
            info[QStringLiteral("readonly")] = ann::has<readonly>(Anns...);
            info[QStringLiteral("hasRange")] = ann::has<range>(Anns...);
            if constexpr (ann::has<range>(Anns...)) {
                constexpr auto r = ann::get_or<range>(range{0, 0}, Anns...);
                info[QStringLiteral("min")] = r.min;
                info[QStringLiteral("max")] = r.max;
            } else {
                info[QStringLiteral("min")] = 0.0;
                info[QStringLiteral("max")] = 0.0;
            }
            info[QStringLiteral("value")] = qml_detail::to_variant<F>(target->[:Fld:]);
            obj->appendField(info);

            T *tgt = target;
            obj->registerGetter(qname, [tgt]() -> QVariant {
                return qml_detail::to_variant<F>(tgt->[:Fld:]);
            });

            if constexpr (ann::has<readonly>(Anns...)) {
                obj->registerSetter(qname, [qname](const QVariant &) -> QString {
                    return qname + QStringLiteral(" is read-only");
                });
            } else if constexpr (ann::has<range>(Anns...) && std::is_arithmetic_v<F>) {
                constexpr auto r = ann::get_or<range>(range{0, 0}, Anns...);
                obj->registerSetter(qname, [tgt, qname](const QVariant &v) -> QString {
                    F      val = qml_detail::from_variant<F>(v);
                    double d   = static_cast<double>(val);
                    if (d < r.min || d > r.max) {
                        return qname + QStringLiteral(" out of range [") +
                               QString::number(r.min) + QStringLiteral(", ") +
                               QString::number(r.max) + QStringLiteral("]");
                    }
                    tgt->[:Fld:] = val;
                    return {};
                });
            } else {
                obj->registerSetter(qname, [tgt](const QVariant &v) -> QString {
                    tgt->[:Fld:] = qml_detail::from_variant<F>(v);
                    return {};
                });
            }
        }

        template <std::meta::info Fn, auto... Anns> void method_instance(const char *name) {
            constexpr auto dann  = ann::get_or<doc>(doc{""}, Anns...);
            constexpr auto arity = std::define_static_array(std::meta::parameters_of(Fn)).size();
            const QString  qname = QString::fromUtf8(name);

            QVariantList paramTypes;
            template for (constexpr auto p :
                          std::define_static_array(std::meta::parameters_of(Fn))) {
                using PT = std::remove_cvref_t<typename [:std::meta::type_of(p):]>;
                paramTypes.append(qml_detail::type_tag<PT>());
            }

            QVariantMap info;
            info[QStringLiteral("name")]       = qname;
            info[QStringLiteral("doc")]        = QString::fromUtf8(dann.text);
            info[QStringLiteral("arity")]      = static_cast<int>(arity);
            info[QStringLiteral("isStatic")]   = false;
            info[QStringLiteral("paramTypes")] = paramTypes;
            obj->appendMethod(info);

            T *tgt = target;
            obj->registerInvoker(qname, [tgt](const QVariantList &args) -> QVariant {
                return qml_detail::invoke_method_impl<Fn>(*tgt, args,
                                                         std::make_index_sequence<arity>{});
            });
        }

        template <std::meta::info Fn, auto... Anns> void method_static(const char *name) {
            constexpr auto dann  = ann::get_or<doc>(doc{""}, Anns...);
            constexpr auto arity = std::define_static_array(std::meta::parameters_of(Fn)).size();
            const QString  qname = QString::fromUtf8(name);

            QVariantList paramTypes;
            template for (constexpr auto p :
                          std::define_static_array(std::meta::parameters_of(Fn))) {
                using PT = std::remove_cvref_t<typename [:std::meta::type_of(p):]>;
                paramTypes.append(qml_detail::type_tag<PT>());
            }

            QVariantMap info;
            info[QStringLiteral("name")]       = qname;
            info[QStringLiteral("doc")]        = QString::fromUtf8(dann.text);
            info[QStringLiteral("arity")]      = static_cast<int>(arity);
            info[QStringLiteral("isStatic")]   = true;
            info[QStringLiteral("paramTypes")] = paramTypes;
            obj->appendMethod(info);

            obj->registerInvoker(qname, [](const QVariantList &args) -> QVariant {
                return qml_detail::invoke_static_impl<Fn>(args,
                                                         std::make_index_sequence<arity>{});
            });
        }
    };

    template <typename T> void bind_qml(ReflectedObject *obj, T &target) {
        QmlVisitor<T> v{obj, &target};
        walk<T>(v);
    }

} // namespace rosetta
