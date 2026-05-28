// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Qt Widgets backend: implements the rosetta::walk visitor concept.
//
// Provides:
//   - rosetta::WidgetVisitor<T> — emits a row of widgets per field/method
//     into two QFormLayouts; wires GET/PUT/Call handlers directly via
//     QObject::connect (no QML, no moc on this side — Qt's own widget
//     signals are enough).
//   - rosetta::build_inspector<T>(target, typeName) — entry point: builds
//     the window, runs the walk, returns a QWidget you can show().
//
// Annotation behaviour mirrors the qml / python / rest backends:
//   readonly      -> editor disabled, PUT button replaced with "ro"
//   range{lo,hi}  -> bounds set on spinbox; PUT also re-checks
//   doc           -> tooltip on the field/method label

#pragma once

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QTime>
#include <QVBoxLayout>
#include <QVariant>
#include <QVariantList>
#include <QWidget>
#include <climits>
#include <experimental/meta>
#include <functional>
#include <rosetta/walk.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace rosetta {

    namespace widget_detail {

        template <typename F> QVariant to_variant(const F &v) {
            if constexpr (std::is_same_v<F, std::string>)
                return QString::fromStdString(v);
            else
                return QVariant::fromValue(v);
        }

        template <typename F> F from_variant(const QVariant &v) {
            if constexpr (std::is_same_v<F, std::string>)
                return v.toString().toStdString();
            else
                return v.value<F>();
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

        // Free helper so the visitor doesn't need to outlive the walk —
        // handlers capture `log` directly instead of `this`.
        inline void log_line(QTextEdit *log, const QString &msg, bool ok) {
            const QString ts    = QTime::currentTime().toString(QStringLiteral("HH:mm:ss"));
            const QString color = ok ? QStringLiteral("#4cb050") : QStringLiteral("#e57373");
            log->append(QStringLiteral("<span style=\"color:#555\">") + ts +
                        QStringLiteral("</span> <span style=\"color:") + color +
                        QStringLiteral("\">") + msg.toHtmlEscaped() + QStringLiteral("</span>"));
        }

    } // namespace widget_detail

    template <typename T> struct WidgetVisitor {
        QFormLayout *fields_layout;
        QFormLayout *methods_layout;
        QTextEdit   *log;
        T           *target;

        template <std::meta::info Fld, auto... Anns> void field(const char *name) {
            using F                 = [:std::meta::type_of(Fld):];
            constexpr auto dann     = ann::get_or<doc>(doc{""}, Anns...);
            constexpr bool ro       = ann::has<readonly>(Anns...);
            constexpr bool has_rng  = ann::has<range>(Anns...);
            constexpr auto rng      = ann::get_or<range>(range{0, 0}, Anns...);
            const QString  qname    = QString::fromUtf8(name);
            T             *tgt      = target;

            // -------- editor widget + typed get/set helpers --------
            QWidget                          *editor = nullptr;
            std::function<QVariant()>         read_editor;
            std::function<void(const F &)>    write_editor;

            if constexpr (std::is_same_v<F, std::string>) {
                auto *le = new QLineEdit(QString::fromStdString(tgt->[:Fld:]));
                le->setReadOnly(ro);
                editor       = le;
                read_editor  = [le] { return QVariant(le->text()); };
                write_editor = [le](const F &v) { le->setText(QString::fromStdString(v)); };
            } else if constexpr (std::is_same_v<F, bool>) {
                auto *cb = new QCheckBox();
                cb->setChecked(tgt->[:Fld:]);
                cb->setEnabled(!ro);
                editor       = cb;
                read_editor  = [cb] { return QVariant(cb->isChecked()); };
                write_editor = [cb](const F &v) { cb->setChecked(v); };
            } else if constexpr (std::is_integral_v<F>) {
                auto *sb = new QSpinBox();
                if constexpr (has_rng)
                    sb->setRange(static_cast<int>(rng.min), static_cast<int>(rng.max));
                else
                    sb->setRange(INT_MIN, INT_MAX);
                sb->setValue(static_cast<int>(tgt->[:Fld:]));
                sb->setReadOnly(ro);
                editor       = sb;
                read_editor  = [sb] { return QVariant(sb->value()); };
                write_editor = [sb](const F &v) { sb->setValue(static_cast<int>(v)); };
            } else if constexpr (std::is_floating_point_v<F>) {
                auto *sb = new QDoubleSpinBox();
                if constexpr (has_rng)
                    sb->setRange(rng.min, rng.max);
                else
                    sb->setRange(-1e12, 1e12);
                sb->setDecimals(6);
                sb->setValue(static_cast<double>(tgt->[:Fld:]));
                sb->setReadOnly(ro);
                editor       = sb;
                read_editor  = [sb] { return QVariant(sb->value()); };
                write_editor = [sb](const F &v) { sb->setValue(static_cast<double>(v)); };
            } else {
                auto *le = new QLineEdit();
                le->setReadOnly(true);
                le->setPlaceholderText(QStringLiteral("(unsupported type)"));
                editor      = le;
                read_editor = [] { return QVariant{}; };
                write_editor = [](const F &) {};
            }

            // -------- row layout: [editor] [GET] [PUT or ro] --------
            auto *row     = new QWidget();
            auto *hbox    = new QHBoxLayout(row);
            hbox->setContentsMargins(0, 0, 0, 0);
            hbox->addWidget(editor, /*stretch*/ 1);

            QTextEdit *log_ptr = log;
            auto      *get_btn = new QPushButton(QStringLiteral("GET"));
            hbox->addWidget(get_btn);
            QObject::connect(get_btn, &QPushButton::clicked, row,
                             [log_ptr, tgt, write_editor, qname] {
                                 F v = tgt->[:Fld:];
                                 write_editor(v);
                                 widget_detail::log_line(
                                     log_ptr,
                                     QStringLiteral("GET ") + qname + QStringLiteral(" = ") +
                                         widget_detail::to_variant<F>(v).toString(),
                                     true);
                             });

            if constexpr (ro) {
                auto *badge = new QPushButton(QStringLiteral("ro"));
                badge->setEnabled(false);
                hbox->addWidget(badge);
            } else {
                auto *put_btn = new QPushButton(QStringLiteral("PUT"));
                hbox->addWidget(put_btn);
                QObject::connect(put_btn, &QPushButton::clicked, row,
                                 [log_ptr, tgt, read_editor, qname] {
                                     F val = widget_detail::from_variant<F>(read_editor());
                                     if constexpr (has_rng && std::is_arithmetic_v<F>) {
                                         double d = static_cast<double>(val);
                                         if (d < rng.min || d > rng.max) {
                                             widget_detail::log_line(
                                                 log_ptr,
                                                 qname + QStringLiteral(" out of range"), false);
                                             return;
                                         }
                                     }
                                     tgt->[:Fld:] = val;
                                     widget_detail::log_line(
                                         log_ptr,
                                         QStringLiteral("PUT ") + qname + QStringLiteral(" ← ") +
                                             widget_detail::to_variant<F>(val).toString(),
                                         true);
                                 });
            }

            // -------- label with [range] hint + doc tooltip --------
            QString label_text = qname;
            if constexpr (has_rng)
                label_text += QStringLiteral(" [%1..%2]")
                                  .arg(rng.min)
                                  .arg(rng.max);
            auto *label = new QLabel(label_text);
            if (dann.text[0] != '\0')
                label->setToolTip(QString::fromUtf8(dann.text));

            fields_layout->addRow(label, row);
        }

        template <std::meta::info Fn, auto... Anns> void method_instance(const char *name) {
            constexpr auto dann  = ann::get_or<doc>(doc{""}, Anns...);
            constexpr auto arity = std::define_static_array(std::meta::parameters_of(Fn)).size();
            const QString  qname = QString::fromUtf8(name);
            T             *tgt   = target;

            auto *row  = new QWidget();
            auto *hbox = new QHBoxLayout(row);
            hbox->setContentsMargins(0, 0, 0, 0);

            std::vector<QLineEdit *> inputs;
            template for (constexpr auto p :
                          std::define_static_array(std::meta::parameters_of(Fn))) {
                auto *le = new QLineEdit();
                le->setPlaceholderText(QString::fromUtf8(std::meta::identifier_of(p)));
                le->setMaximumWidth(140);
                hbox->addWidget(le);
                inputs.push_back(le);
            }
            hbox->addStretch(1);

            QTextEdit *log_ptr  = log;
            auto      *call_btn = new QPushButton(QStringLiteral("Call"));
            hbox->addWidget(call_btn);
            QObject::connect(call_btn, &QPushButton::clicked, row,
                             [log_ptr, tgt, inputs, qname] {
                                 QVariantList args;
                                 for (auto *le : inputs)
                                     args.append(QVariant(le->text()));
                                 QVariant result = widget_detail::invoke_method_impl<Fn>(
                                     *tgt, args, std::make_index_sequence<arity>{});
                                 widget_detail::log_line(
                                     log_ptr,
                                     QStringLiteral("CALL ") + qname + QStringLiteral(" → ") +
                                         result.toString(),
                                     true);
                             });

            auto *label = new QLabel(qname + QStringLiteral("(") + QString::number(arity) +
                                     QStringLiteral(")"));
            if (dann.text[0] != '\0')
                label->setToolTip(QString::fromUtf8(dann.text));
            methods_layout->addRow(label, row);
        }

        template <std::meta::info Fn, auto... Anns> void method_static(const char *name) {
            constexpr auto dann  = ann::get_or<doc>(doc{""}, Anns...);
            constexpr auto arity = std::define_static_array(std::meta::parameters_of(Fn)).size();
            const QString  qname = QString::fromUtf8(name);

            auto *row  = new QWidget();
            auto *hbox = new QHBoxLayout(row);
            hbox->setContentsMargins(0, 0, 0, 0);

            std::vector<QLineEdit *> inputs;
            template for (constexpr auto p :
                          std::define_static_array(std::meta::parameters_of(Fn))) {
                auto *le = new QLineEdit();
                le->setPlaceholderText(QString::fromUtf8(std::meta::identifier_of(p)));
                le->setMaximumWidth(140);
                hbox->addWidget(le);
                inputs.push_back(le);
            }
            hbox->addStretch(1);

            QTextEdit *log_ptr  = log;
            auto      *call_btn = new QPushButton(QStringLiteral("Call"));
            hbox->addWidget(call_btn);
            QObject::connect(call_btn, &QPushButton::clicked, row,
                             [log_ptr, inputs, qname] {
                                 QVariantList args;
                                 for (auto *le : inputs)
                                     args.append(QVariant(le->text()));
                                 QVariant result = widget_detail::invoke_static_impl<Fn>(
                                     args, std::make_index_sequence<arity>{});
                                 widget_detail::log_line(
                                     log_ptr,
                                     QStringLiteral("CALL ") + qname + QStringLiteral(" → ") +
                                         result.toString(),
                                     true);
                             });

            auto *label = new QLabel(qname + QStringLiteral("(") + QString::number(arity) +
                                     QStringLiteral(") [static]"));
            if (dann.text[0] != '\0')
                label->setToolTip(QString::fromUtf8(dann.text));
            methods_layout->addRow(label, row);
        }
    };

    template <typename T> QWidget *build_inspector(T &target, const QString &type_name) {
        auto *root = new QWidget();
        root->setWindowTitle(type_name + QStringLiteral(" — Qt Widgets inspector"));

        auto *vbox = new QVBoxLayout(root);

        auto *heading = new QLabel(type_name +
                                   QStringLiteral(" — generated from one "
                                                  "C++26 reflection walk"));
        heading->setStyleSheet(QStringLiteral("color:#888;"));
        vbox->addWidget(heading);

        auto *fields_group  = new QGroupBox(QStringLiteral("Fields"));
        auto *fields_layout = new QFormLayout(fields_group);
        vbox->addWidget(fields_group);

        auto *methods_group  = new QGroupBox(QStringLiteral("Methods"));
        auto *methods_layout = new QFormLayout(methods_group);
        vbox->addWidget(methods_group);

        auto *log = new QTextEdit();
        log->setReadOnly(true);
        log->setFontFamily(QStringLiteral("Menlo"));
        log->setStyleSheet(QStringLiteral("background:#0e0e0e; color:#e8e8e8;"));
        vbox->addWidget(log, /*stretch*/ 1);

        WidgetVisitor<T> v{fields_layout, methods_layout, log, &target};
        walk<T>(v);
        return root;
    }

} // namespace rosetta
