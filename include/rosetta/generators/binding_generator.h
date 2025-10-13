// ============================================================================
// rosetta/generators/binding_generator.hpp
//
// Interface de base pour tous les générateurs de bindings
// ============================================================================
#pragma once
#include <string>

namespace rosetta::generators {

    /**
     * @brief Interface abstraite pour les générateurs de bindings
     *
     * Tous les générateurs spécifiques (Python, JavaScript, etc.) doivent
     * hériter de cette classe et implémenter les méthodes virtuelles.
     */
    class BindingGenerator {
    public:
        virtual ~BindingGenerator() = default;

        /**
         * @brief Génère le code de binding complet
         * @return Code source généré
         */
        virtual std::string generate() const = 0;

        /**
         * @brief Retourne le nom du langage cible
         * @return Nom du langage (ex: "Python", "JavaScript")
         */
        virtual std::string get_language() const = 0;

        /**
         * @brief Retourne l'extension de fichier recommandée
         * @return Extension (ex: ".cpp", ".js")
         */
        virtual std::string get_file_extension() const { return ".cpp"; }

        /**
         * @brief Génère les instructions de compilation
         * @return Texte des instructions
         */
        virtual std::string generate_build_instructions() const {
            return "// No specific build instructions";
        }

        /**
         * @brief Génère un exemple d'utilisation
         * @return Code d'exemple
         */
        virtual std::string generate_usage_example() const {
            return "// No usage example available";
        }
    };

} // namespace rosetta::generators