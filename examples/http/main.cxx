#include <iostream>
#include <unordered_map>
#include "../person.h"
#include <httplib.h> // cpp-httplib library

void launchGui(introspection::Introspectable &obj);

// Example usage
int main()
{
    std::cerr << "Starting HTTP server at http://localhost:8080\n";
    std::cerr << "Press Ctrl+C to stop the server.\n";

    Person person("Alice", 30, 1.65);
    launchGui(person);

    return 0;
}

// ===================================================

std::string generateHtmlForm(const introspection::Introspectable &obj)
{
    std::stringstream html;
    const auto &type_info = obj.getTypeInfo();

    html << "<!DOCTYPE html>\n<html>\n<head>\n";
    html << "<title>" << type_info.class_name << " Editor</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; max-width: 600px; margin: 50px auto; }\n";
    html << ".field { margin: 15px 0; }\n";
    html << "label { display: block; margin-bottom: 5px; font-weight: bold; }\n";
    html << "input, button { padding: 8px; border: 1px solid #ddd; border-radius: 4px; }\n";
    html << "input[type='text'], input[type='number'] { width: 300px; }\n";
    html << "button { background: #007cba; color: white; margin: 5px; cursor: pointer; }\n";
    html << "button:hover { background: #005a8a; }\n";
    html << ".methods { margin-top: 30px; }\n";
    html << "</style>\n</head>\n<body>\n";

    html << "<h1>" << type_info.class_name << " Editor</h1>\n";
    html << "<form id='objectForm'>\n";

    // Generate form fields for members
    auto member_names = type_info.getMemberNames();
    for (const auto &name : member_names)
    {
        const auto *member = type_info.getMember(name);
        html << "<div class='field'>\n";
        html << "<label for='" << name << "'>" << name << " (" << member->type_name << "):</label>\n";

        auto value = member->getter(&obj);

        if (member->type_name == "string")
        {
            html << "<input type='text' id='" << name << "' name='" << name
                 << "' value='" << std::any_cast<std::string>(value) << "'>\n";
        }
        else if (member->type_name == "int")
        {
            html << "<input type='number' id='" << name << "' name='" << name
                 << "' value='" << std::any_cast<int>(value) << "'>\n";
        }
        else if (member->type_name == "double")
        {
            html << "<input type='number' step='0.01' id='" << name << "' name='" << name
                 << "' value='" << std::any_cast<double>(value) << "'>\n";
        }
        else if (member->type_name == "bool")
        {
            html << "<input type='checkbox' id='" << name << "' name='" << name
                 << (std::any_cast<bool>(value) ? " checked" : "") << "'>\n";
        }

        html << "</div>\n";
    }

    html << "<button type='button' onclick='updateObject()'>Update Object</button>\n";
    html << "</form>\n";

    // Generate method buttons
    html << "<div class='methods'>\n<h2>Methods</h2>\n";
    auto method_names = type_info.getMethodNames();
    for (const auto &name : method_names)
    {
        html << "<button onclick='callMethod(\"" << name << "\")'>" << name << "()</button>\n";
    }
    html << "</div>\n";

    // Add JavaScript for interactivity
    html << "<script>\n";
    html << "function updateObject() {\n";
    html << "  const formData = new FormData(document.getElementById('objectForm'));\n";
    html << "  const data = Object.fromEntries(formData);\n";
    html << "  console.log('Update object with:', data);\n";
    html << "  // Here we would send data back to C++ via WebSocket/HTTP\n";
    html << "}\n\n";
    html << "function callMethod(methodName) {\n";
    html << "  console.log('Call method:', methodName);\n";
    html << "  // Here we can call the C++ method via WebSocket/HTTP\n";
    html << "}\n";
    html << "</script>\n";

    html << "</body>\n</html>";

    return html.str();
}

void launchGui(Introspectable &obj)
{
    httplib::Server server;

    server.Get("/", [&](const httplib::Request &, httplib::Response &res)
               { res.set_content(generateHtmlForm(obj), "text/html"); });

    server.Post("/update", [&](const httplib::Request &req, httplib::Response &res)
                {

        // Parse JSON and update object
        // obj.setMemberValue(name, value);

        res.set_content("OK", "text/plain"); });

    server.listen("0.0.0.0", 8080);
}