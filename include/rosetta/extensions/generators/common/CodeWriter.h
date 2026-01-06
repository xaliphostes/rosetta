#pragma once
#include "GeneratorConfig.h"
#include "TypeMapper.h"
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include <regex>

// ============================================================================
// Base Code Writer - Modernized with string literals and RAII support
// ============================================================================

/**
 * @class CodeWriter
 * @brief Helper base class for generating structured text with indentation.
 *
 * CodeWriter is intended for "code generation" style output where indentation
 * matters (CMake, C++, Python, config files, etc.). It wraps an output stream
 * and provides:
 *
 * - Indentation management (indent/dedent)
 * - Line-oriented output (line, blank, comment, section)
 * - Multi-line raw-string emission with indentation normalization (emit)
 * - Simple ${VAR} placeholder substitution (emit / format)
 * - Block helpers to reduce boilerplate (block, cmake_set, cmake_if, ...)
 * - RAII indentation via IndentGuard (indented())
 *
 * Typical usage:
 *   - Create a subclass that implements generate()
 *   - Call line()/emit()/block() to write structured content
 *
 * Output model
 * ------------
 * CodeWriter writes to a provided std::ostream (e.g. std::ofstream,
 * std::ostringstream, std::cout). Indentation is applied only in the
 * line-oriented methods (line, comment, section, emit lines).
 *
 * Indentation
 * -----------
 * Indentation is controlled by an integer indent level (indent_) and a
 * configurable number of spaces per indent (spaces_per_indent_).
 *
 * Prefer the RAII helper indented() to avoid mismatched indent/dedent calls:
 *
 * @code
 * line("if(x) {");
 * {
 *   auto _ = indented();
 *   line("do_something();");
 * }
 * line("}");
 * @endcode
 *
 * Multi-line emission
 * -------------------
 * emit(text, vars) is designed for use with raw string literals R"( ... )".
 *
 * It:
 *  - splits the provided text into lines
 *  - strips an optional empty first/last line (common with R"( ... )")
 *  - computes the minimum common leading whitespace across non-empty lines
 *    and removes it (dedenting the literal)
 *  - re-emits each line with the current indentation level
 *  - supports ${VAR} substitution via the provided map
 *
 * Variable substitution
 * ---------------------
 * emit() and format() support a simple placeholder scheme:
 *
 *   "${KEY}"  -> replaced by vars.at("KEY") if present
 *
 * This is NOT a full template language: it is a straightforward string replace.
 *
 * Safety notes / conventions
 * --------------------------
 * - dedent() never makes indentation negative; it stops at 0.
 * - raw() writes verbatim text without indentation or newline.
 * - emit() treats empty lines specially: it produces blank() output.
 *
 * Extending
 * ---------
 * Subclasses implement:
 *
 *   virtual void generate() = 0;
 *
 * and may add domain-specific helpers (e.g. Python blocks, C++ namespaces,
 * CMake target helpers, etc.).
 */
class CodeWriter {
protected:
    std::ostream& out_;
    int indent_ = 0;
    TypeMapper type_mapper_;
    GeneratorConfig config_;
    int spaces_per_indent_ = 4;

    // ========================================================================
    // RAII Indent Guard
    // ========================================================================
public:
    class IndentGuard {
        CodeWriter& writer_;
    public:
        explicit IndentGuard(CodeWriter& w) : writer_(w) { writer_.indent(); }
        ~IndentGuard() { writer_.dedent(); }
        
        // Non-copyable, non-movable
        IndentGuard(const IndentGuard&) = delete;
        IndentGuard& operator=(const IndentGuard&) = delete;
    };
    
    // Create an indent guard for RAII-style indentation
    [[nodiscard]] IndentGuard indented() { return IndentGuard(*this); }

protected:
    // ========================================================================
    // Core Output Methods
    // ========================================================================
    
    // Output a line with current indentation
    void line(const std::string& s = "") {
        out_ << std::string(indent_ * spaces_per_indent_, ' ') << s << "\n";
    }

    // Output raw text without indentation or newline
    void raw(const std::string& s) { 
        out_ << s; 
    }

    // Output a blank line
    void blank() {
        out_ << "\n";
    }

    // Increase indentation
    void indent() { 
        indent_++; 
    }

    // Decrease indentation
    void dedent() {
        if (indent_ > 0) indent_--;
    }

    // Output a comment
    void comment(const std::string& text) {
        line("// " + text);
    }

    // Output a section header
    void section(const std::string& title) {
        line("// " + std::string(76, '='));
        line("// " + title);
        line("// " + std::string(76, '='));
    }

    // ========================================================================
    // Multi-line Raw String Support
    // ========================================================================
    
    /**
     * @brief Emit a multi-line raw string with automatic indentation handling
     * 
     * Features:
     * - Strips common leading whitespace from all lines
     * - Applies current indentation level
     * - Supports ${VAR} variable substitution
     * - Skips empty first/last lines (common with R"(...)")
     * 
     * @param text The raw string literal
     * @param vars Optional variable substitutions
     */
    void emit(std::string_view text, 
              const std::unordered_map<std::string, std::string>& vars = {}) {
        std::string processed = substitute_vars(std::string(text), vars);
        std::vector<std::string> lines = split_lines(processed);
        
        // Remove empty first/last lines (artifact of R"( syntax)
        if (!lines.empty() && lines.front().empty()) {
            lines.erase(lines.begin());
        }
        if (!lines.empty() && lines.back().empty()) {
            lines.pop_back();
        }
        
        if (lines.empty()) return;
        
        // Find minimum indentation (ignoring empty lines)
        size_t min_indent = std::string::npos;
        for (const auto& l : lines) {
            if (l.empty()) continue;
            size_t first_non_space = l.find_first_not_of(" \t");
            if (first_non_space != std::string::npos) {
                min_indent = std::min(min_indent, first_non_space);
            }
        }
        if (min_indent == std::string::npos) min_indent = 0;
        
        // Output each line with adjusted indentation
        for (const auto& l : lines) {
            if (l.empty()) {
                blank();
            } else {
                std::string stripped = (l.size() > min_indent) 
                    ? l.substr(min_indent) 
                    : "";
                line(stripped);
            }
        }
    }

    /**
     * @brief Emit without variable substitution (slightly faster)
     */
    // void emit(std::string_view text) {
    //     emit(text, {});
    // }

    // ========================================================================
    // Block Helpers with Lambdas
    // ========================================================================
    
    /**
     * @brief Execute a block with automatic indent/dedent
     * 
     * Usage:
     *   with_indent([&]() {
     *       line("indented content");
     *   });
     */
    template<typename F>
    void with_indent(F&& body) {
        indent();
        body();
        dedent();
    }
    
    /**
     * @brief Output a block with open/close delimiters
     * 
     * Usage:
     *   block("if(condition) {", "}", [&]() {
     *       line("doSomething();");
     *   });
     */
    template<typename F>
    void block(const std::string& open, const std::string& close, F&& body) {
        line(open);
        indent();
        body();
        dedent();
        line(close);
    }
    
    /**
     * @brief Output a block with only opening delimiter (close is just "}")
     */
    template<typename F>
    void block(const std::string& open, F&& body) {
        block(open, "}", std::forward<F>(body));
    }

    /**
     * @brief CMake-style block: set(VAR ...)
     */
    template<typename F>
    void cmake_set(const std::string& var_name, F&& body) {
        line("set(" + var_name);
        indent();
        body();
        dedent();
        line(")");
    }

    /**
     * @brief CMake-style conditional block
     */
    template<typename F>
    void cmake_if(const std::string& condition, F&& body) {
        block("if(" + condition + ")", "endif()", std::forward<F>(body));
    }

    /**
     * @brief CMake-style target property block
     */
    template<typename F>
    void cmake_target_block(const std::string& command, 
                            const std::string& target, 
                            const std::string& visibility,
                            F&& body) {
        line(command + "(" + target + " " + visibility);
        indent();
        body();
        dedent();
        line(")");
    }

    // ========================================================================
    // List/Collection Helpers
    // ========================================================================
    
    /**
     * @brief Output a list of items with proper formatting
     */
    void lines(const std::vector<std::string>& items) {
        for (const auto& item : items) {
            line(item);
        }
    }
    
    /**
     * @brief Output items with a transform function
     */
    template<typename Container, typename Transform>
    void lines(const Container& items, Transform&& transform) {
        for (const auto& item : items) {
            line(transform(item));
        }
    }
    
    /**
     * @brief Output quoted strings (useful for paths)
     */
    void quoted_lines(const std::vector<std::string>& items) {
        for (const auto& item : items) {
            line("\"" + item + "\"");
        }
    }

    // ========================================================================
    // String Utilities
    // ========================================================================
    
    // Join strings with separator
    static std::string join(const std::vector<std::string>& items, 
                            const std::string& sep) {
        std::ostringstream oss;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) oss << sep;
            oss << items[i];
        }
        return oss.str();
    }
    
    // Format with simple ${VAR} substitution
    static std::string format(const std::string& templ,
                              const std::unordered_map<std::string, std::string>& vars) {
        return substitute_vars(templ, vars);
    }

private:
    static std::vector<std::string> split_lines(const std::string& text) {
        std::vector<std::string> result;
        std::istringstream iss(text);
        std::string line;
        while (std::getline(iss, line)) {
            // Remove trailing \r if present (Windows line endings)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            result.push_back(line);
        }
        return result;
    }
    
    static std::string substitute_vars(const std::string& text,
                                       const std::unordered_map<std::string, std::string>& vars) {
        if (vars.empty()) return text;
        
        std::string result = text;
        for (const auto& [key, value] : vars) {
            std::string pattern = "${" + key + "}";
            size_t pos = 0;
            while ((pos = result.find(pattern, pos)) != std::string::npos) {
                result.replace(pos, pattern.length(), value);
                pos += value.length();
            }
        }
        return result;
    }

public:
    explicit CodeWriter(std::ostream& out, const GeneratorConfig& config = {})
        : out_(out), config_(config) {
        // Register types with the configured namespace
        type_mapper_.register_namespaced_types(config.types_namespace);
    }
    
    virtual ~CodeWriter() = default;
    
    // Main generation method - must be implemented by subclasses
    virtual void generate() = 0;

    // Access to configuration
    const GeneratorConfig& config() const { return config_; }
    const TypeMapper& type_mapper() const { return type_mapper_; }
    
    // Allow changing indent size
    void set_indent_size(int spaces) { spaces_per_indent_ = spaces; }
};

// ============================================================================
// Convenience Macros (Optional)
// ============================================================================

// RAII-style indent block
#define CODE_INDENT auto _indent_guard_##__LINE__ = indented()

// Quick block with braces
#define CODE_BLOCK(header) \
    line(header " {"); \
    auto _block_guard_##__LINE__ = indented(); \
    /* user code follows, dedent on scope exit */