// ============================================================================
// rosetta/extensions/validation/constraint_validator.hpp
//
// Système de validation avec contraintes
// ============================================================================
#pragma once
#include "../../core/registry.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace rosetta::extensions {

    /**
     * @brief Interface de base pour une contrainte
     * @tparam T Type de la valeur à valider
     */
    template <typename T> class Constraint {
    public:
        virtual ~Constraint() = default;

        /**
         * @brief Valide une valeur
         * @param value Valeur à valider
         * @return true si valide
         */
        virtual bool validate(const T &value) const = 0;

        /**
         * @brief Message d'erreur en cas d'échec
         * @return Message d'erreur
         */
        virtual std::string get_error_message() const = 0;
    };

    /**
     * @brief Contrainte de plage (min/max)
     */
    template <typename T> class RangeConstraint : public Constraint<T> {
        T min_;
        T max_;

    public:
        RangeConstraint(T min, T max) : min_(min), max_(max) {}

        bool validate(const T &value) const override { return value >= min_ && value <= max_; }

        std::string get_error_message() const override {
            return "Value must be between " + std::to_string(min_) + " and " + std::to_string(max_);
        }
    };

    /**
     * @brief Contrainte de non-nullité
     */
    template <typename T> class NotNullConstraint : public Constraint<T *> {
    public:
        bool validate(T *const &value) const override { return value != nullptr; }

        std::string get_error_message() const override { return "Value must not be null"; }
    };

    /**
     * @brief Contrainte de taille pour conteneurs
     */
    template <typename Container> class SizeConstraint : public Constraint<Container> {
        size_t min_size_;
        size_t max_size_;

    public:
        SizeConstraint(size_t min_size, size_t max_size = SIZE_MAX)
            : min_size_(min_size), max_size_(max_size) {}

        bool validate(const Container &value) const override {
            return value.size() >= min_size_ && value.size() <= max_size_;
        }

        std::string get_error_message() const override {
            return "Container size must be between " + std::to_string(min_size_) + " and " +
                   std::to_string(max_size_);
        }
    };

    /**
     * @brief Contrainte personnalisée avec lambda
     */
    template <typename T> class CustomConstraint : public Constraint<T> {
        std::function<bool(const T &)> validator_;
        std::string                    error_message_;

    public:
        CustomConstraint(std::function<bool(const T &)> validator, std::string error_message)
            : validator_(std::move(validator)), error_message_(std::move(error_message)) {}

        bool validate(const T &value) const override { return validator_(value); }

        std::string get_error_message() const override { return error_message_; }
    };

    /**
     * @brief Validateur de contraintes pour les champs de classe
     */
    class ConstraintValidator {
        struct FieldConstraints {
            std::vector<std::function<bool(const core::Any &)>> validators;
            std::vector<std::string>                            error_messages;
        };

        std::unordered_map<std::string, std::unordered_map<std::string, FieldConstraints>>
            class_constraints_;

        ConstraintValidator() = default;

        // Non-copiable
        ConstraintValidator(const ConstraintValidator &)            = delete;
        ConstraintValidator &operator=(const ConstraintValidator &) = delete;

    public:
        /**
         * @brief Obtient l'instance singleton
         */
        static ConstraintValidator &instance() {
            static ConstraintValidator validator;
            return validator;
        }

        /**
         * @brief Ajoute une contrainte sur un champ
         * @tparam Class Type de la classe
         * @tparam T Type du champ
         * @param field_name Nom du champ
         * @param constraint Contrainte à ajouter
         */
        template <typename Class, typename T>
        void add_field_constraint(const std::string             &field_name,
                                  std::unique_ptr<Constraint<T>> constraint) {
            std::string class_name = typeid(Class).name();

            auto &field_constraints = class_constraints_[class_name][field_name];

            // Capturer la contrainte dans un shared_ptr pour permettre la copie
            auto shared_constraint = std::shared_ptr<Constraint<T>>(std::move(constraint));

            // Wrapper pour Any
            auto validator = [shared_constraint](const core::Any &value) -> bool {
                return shared_constraint->validate(value.as<T>());
            };

            field_constraints.validators.push_back(std::move(validator));
            field_constraints.error_messages.push_back(shared_constraint->get_error_message());
        }

        /**
         * @brief Valide un objet complet
         * @tparam Class Type de la classe
         * @param obj Objet à valider
         * @param errors Vecteur pour collecter les erreurs
         * @return true si valide
         */
        template <typename Class>
        bool validate(const Class &obj, std::vector<std::string> &errors) {
            std::string class_name = typeid(Class).name();

            auto class_it = class_constraints_.find(class_name);
            if (class_it == class_constraints_.end()) {
                return true; // Pas de contraintes
            }

            auto &registry = core::Registry::instance();
            if (!registry.has_class<Class>()) {
                return true;
            }

            const auto &meta  = registry.get<Class>();
            bool        valid = true;

            for (const auto &[field_name, field_constraints] : class_it->second) {
                auto value = meta.get_field(const_cast<Class &>(obj), field_name);

                for (size_t i = 0; i < field_constraints.validators.size(); ++i) {
                    if (!field_constraints.validators[i](value)) {
                        errors.push_back(field_name + ": " + field_constraints.error_messages[i]);
                        valid = false;
                    }
                }
            }

            return valid;
        }

        /**
         * @brief Efface toutes les contraintes
         */
        void clear() { class_constraints_.clear(); }
    };

    /**
     * @brief Helper pour créer des contraintes facilement
     */
    template <typename T> std::unique_ptr<Constraint<T>> make_range(T min, T max) {
        return std::make_unique<RangeConstraint<T>>(min, max);
    }

    template <typename T> std::unique_ptr<Constraint<T *>> make_not_null() {
        return std::make_unique<NotNullConstraint<T>>();
    }

    template <typename Container>
    std::unique_ptr<Constraint<Container>> make_size(size_t min, size_t max = SIZE_MAX) {
        return std::make_unique<SizeConstraint<Container>>(min, max);
    }

    template <typename T>
    std::unique_ptr<Constraint<T>> make_custom(std::function<bool(const T &)> validator,
                                               std::string                    error_message) {
        return std::make_unique<CustomConstraint<T>>(std::move(validator),
                                                     std::move(error_message));
    }

} // namespace rosetta::extensions