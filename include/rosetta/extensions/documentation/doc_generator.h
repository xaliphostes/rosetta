#pragma once
#include "../../core/registry.h"
#include <sstream>
#include <string>
#include <vector>

namespace rosetta::extensions {

    enum class DocFormat { Markdown, HTML, PlainText };

    class DocGenerator {
        DocFormat format_;

    public:
        explicit DocGenerator(DocFormat format = DocFormat::Markdown);
        std::string generate() const;
        std::string generate_class_doc(const std::string &class_name) const;

    private:
        void generate_markdown(std::stringstream &ss) const;
        void generate_class_markdown(std::stringstream &ss, const std::string &class_name, const core::Registry::MetadataHolder &holder) const;
        void generate_html(std::stringstream &ss) const;
        void generate_class_html(std::stringstream &ss, const std::string &class_name, const core::Registry::MetadataHolder &holder) const;
        void generate_plaintext(std::stringstream &ss) const;
        void generate_class_plaintext(std::stringstream &ss, const std::string &class_name, const core::Registry::MetadataHolder &holder) const;
        static std::string to_anchor(const std::string &str);
    };

} // namespace rosetta::extensions

#include "inline/doc_generator.hxx"