// ============================================================================
// rosetta/core/any.hpp
//
// Type erasure pour stocker n'importe quel type de manière type-safe
// ============================================================================
#pragma once
#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace rosetta::core {

    /**
     * @brief Conteneur type-erased pouvant stocker n'importe quel type
     *
     * Similaire à std::any mais avec une interface simplifiée pour Rosetta
     */
    class Any {
        struct Holder {
            virtual ~Holder()                              = default;
            virtual Holder         *clone() const          = 0;
            virtual std::string     type_name() const      = 0;
            virtual const void     *get_void_ptr() const   = 0;
            virtual std::type_index get_type_index() const = 0; // ADD THIS
        };

        template <typename T> struct HolderImpl : Holder {
            T value;
            HolderImpl(T v) : value(std::move(v)) {}
            Holder         *clone() const override { return new HolderImpl(value); }
            std::string     type_name() const override { return typeid(T).name(); }
            const void     *get_void_ptr() const override { return &value; }
            std::type_index get_type_index() const override { return std::type_index(typeid(T)); }
        };

        std::unique_ptr<Holder> holder_;

    public:
        /**
         * @brief Constructeur par défaut (any vide)
         */
        Any() = default;

        /**
         * @brief Constructeur à partir d'une valeur
         * @tparam T Type de la valeur
         * @param value Valeur à stocker
         */
        template <typename T> Any(T value) : holder_(new HolderImpl<T>(std::move(value))) {}

        /**
         * @brief Constructeur de copie
         */
        Any(const Any &other) : holder_(other.holder_ ? other.holder_->clone() : nullptr) {}

        /**
         * @brief Constructeur de move
         */
        Any(Any &&) = default;

        /**
         * @brief Opérateur d'assignation par copie
         */
        Any &operator=(const Any &other) {
            if (this != &other) {
                holder_ = other.holder_ ? std::unique_ptr<Holder>(other.holder_->clone()) : nullptr;
            }
            return *this;
        }

        /**
         * @brief Opérateur d'assignation par move
         */
        Any &operator=(Any &&) = default;

        /**
         * @brief Récupère la valeur stockée
         * @tparam T Type attendu
         * @return Référence à la valeur
         * @throws std::bad_cast si le type ne correspond pas
         */
        template <typename T> T &as() { return static_cast<HolderImpl<T> *>(holder_.get())->value; }

        /**
         * @brief Récupère la valeur stockée (version const)
         * @tparam T Type attendu
         * @return Référence const à la valeur
         * @throws std::bad_cast si le type ne correspond pas
         */
        template <typename T> const T &as() const {
            return static_cast<const HolderImpl<T> *>(holder_.get())->value;
        }

        /**
         * @brief Obtient le nom du type stocké
         * @return Nom du type (mangled)
         */
        std::string type_name() const { return holder_ ? holder_->type_name() : "empty"; }

        /**
         * @brief Vérifie si l'any contient une valeur
         * @return true si non vide
         */
        bool has_value() const { return holder_ != nullptr; }

        /**
         * @brief Réinitialise l'any (le vide)
         */
        void reset() { holder_.reset(); }

        std::type_index get_type_index() const {
            if (!holder_) {
                return std::type_index(typeid(void));
            }
            return holder_->get_type_index();
        }

        const void *get_void_ptr() const {
            if (!holder_)
                return nullptr;
            return holder_->get_void_ptr();
        }
    };

} // namespace rosetta::core