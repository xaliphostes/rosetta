#pragma once
#include "GeneratorConfig.h"
#include "TypeMapper.h"
#include <fstream>
#include <sstream>
#include <string>

// ============================================================================
// Base Code Writer
// Provides utilities for generating formatted source code
// ============================================================================

class CodeWriter {
protected:
    std::ostream& out_;
    int indent_ = 0;
    TypeMapper type_mapper_;
    GeneratorConfig config_;

    // Output a line with current indentation
    void line(const std::string& s = "") {
        out_ << std::string(indent_ * 4, ' ') << s << "\n";
    }

    // Output raw text without indentation
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
};
