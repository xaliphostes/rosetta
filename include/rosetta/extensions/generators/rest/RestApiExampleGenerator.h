#pragma once
#include "../common/CodeWriter.h"

// ============================================================================
// REST API Example Generator (index.html)
// Generates a web-based REST API explorer for testing the generated REST API
// ============================================================================

class RestApiExampleGenerator : public CodeWriter {
public:
    using CodeWriter::CodeWriter;

    void generate() override {
        write_doctype_and_head();
        write_styles();
        write_body_header();
        write_main_content();
        write_modals();
        write_javascript();
        write_closing();
    }

private:
    void write_doctype_and_head() {
        line("<!DOCTYPE html>");
        line("<html lang=\"en\">");
        line();
        line("<head>");
        indent();
        line("<meta charset=\"UTF-8\">");
        line("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
        line("<title>" + config_.module_name + " REST API Explorer</title>");
    }

    void write_styles() {
        line("<style>");
        indent();

        // CSS Variables
        line(":root {");
        indent();
        line("--bg-primary: #1a1a2e;");
        line("--bg-secondary: #16213e;");
        line("--bg-tertiary: #0f3460;");
        line("--accent: #e94560;");
        line("--accent-hover: #ff6b6b;");
        line("--text-primary: #eaeaea;");
        line("--text-secondary: #a0a0a0;");
        line("--success: #4ecca3;");
        line("--error: #ff6b6b;");
        line("--border: #2a2a4a;");
        dedent();
        line("}");
        line();

        // Reset
        line("* {");
        indent();
        line("margin: 0;");
        line("padding: 0;");
        line("box-sizing: border-box;");
        dedent();
        line("}");
        line();

        // Body
        line("body {");
        indent();
        line("font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;");
        line("background: var(--bg-primary);");
        line("color: var(--text-primary);");
        line("min-height: 100vh;");
        dedent();
        line("}");
        line();

        // Container
        line(".container {");
        indent();
        line("max-width: 1400px;");
        line("margin: 0 auto;");
        line("padding: 20px;");
        dedent();
        line("}");
        line();

        // Header
        line("header {");
        indent();
        line("background: var(--bg-secondary);");
        line("padding: 20px;");
        line("border-bottom: 2px solid var(--accent);");
        line("margin-bottom: 20px;");
        dedent();
        line("}");
        line();

        line("header h1 {");
        indent();
        line("font-size: 1.8rem;");
        line("font-weight: 600;");
        line("display: flex;");
        line("align-items: center;");
        line("gap: 10px;");
        dedent();
        line("}");
        line();

        line("header h1::before {");
        indent();
        line("content: \"\\1F50C\";");
        dedent();
        line("}");
        line();

        // Connection bar
        line(".connection-bar {");
        indent();
        line("display: flex;");
        line("gap: 10px;");
        line("align-items: center;");
        line("margin-top: 15px;");
        dedent();
        line("}");
        line();

        line(".connection-bar input {");
        indent();
        line("flex: 1;");
        line("max-width: 400px;");
        line("padding: 10px 15px;");
        line("border: 1px solid var(--border);");
        line("border-radius: 6px;");
        line("background: var(--bg-primary);");
        line("color: var(--text-primary);");
        line("font-size: 14px;");
        dedent();
        line("}");
        line();

        line(".connection-bar input:focus {");
        indent();
        line("outline: none;");
        line("border-color: var(--accent);");
        dedent();
        line("}");
        line();

        // Buttons
        line("button {");
        indent();
        line("padding: 10px 20px;");
        line("border: none;");
        line("border-radius: 6px;");
        line("cursor: pointer;");
        line("font-size: 14px;");
        line("font-weight: 500;");
        line("transition: all 0.2s;");
        dedent();
        line("}");
        line();

        line(".btn-primary {");
        indent();
        line("background: var(--accent);");
        line("color: white;");
        dedent();
        line("}");
        line();

        line(".btn-primary:hover {");
        indent();
        line("background: var(--accent-hover);");
        dedent();
        line("}");
        line();

        line(".btn-secondary {");
        indent();
        line("background: var(--bg-tertiary);");
        line("color: var(--text-primary);");
        dedent();
        line("}");
        line();

        line(".btn-secondary:hover {");
        indent();
        line("background: #1a4a7a;");
        dedent();
        line("}");
        line();

        line(".btn-danger {");
        indent();
        line("background: #c0392b;");
        line("color: white;");
        dedent();
        line("}");
        line();

        line(".btn-danger:hover {");
        indent();
        line("background: #e74c3c;");
        dedent();
        line("}");
        line();

        line(".btn-small {");
        indent();
        line("padding: 6px 12px;");
        line("font-size: 12px;");
        dedent();
        line("}");
        line();

        // Status indicator
        line(".status-indicator {");
        indent();
        line("display: inline-flex;");
        line("align-items: center;");
        line("gap: 6px;");
        line("padding: 6px 12px;");
        line("border-radius: 20px;");
        line("font-size: 13px;");
        line("background: var(--bg-primary);");
        dedent();
        line("}");
        line();

        line(".status-indicator.connected {");
        indent();
        line("color: var(--success);");
        dedent();
        line("}");
        line();

        line(".status-indicator.disconnected {");
        indent();
        line("color: var(--error);");
        dedent();
        line("}");
        line();

        line(".status-dot {");
        indent();
        line("width: 8px;");
        line("height: 8px;");
        line("border-radius: 50%;");
        line("background: currentColor;");
        dedent();
        line("}");
        line();

        // Main grid
        line(".main-grid {");
        indent();
        line("display: grid;");
        line("grid-template-columns: 300px 1fr 400px;");
        line("gap: 20px;");
        line("min-height: calc(100vh - 180px);");
        dedent();
        line("}");
        line();

        // Panel
        line(".panel {");
        indent();
        line("background: var(--bg-secondary);");
        line("border-radius: 10px;");
        line("overflow: hidden;");
        dedent();
        line("}");
        line();

        line(".panel-header {");
        indent();
        line("background: var(--bg-tertiary);");
        line("padding: 15px;");
        line("font-weight: 600;");
        line("display: flex;");
        line("justify-content: space-between;");
        line("align-items: center;");
        dedent();
        line("}");
        line();

        line(".panel-content {");
        indent();
        line("padding: 15px;");
        line("max-height: calc(100vh - 280px);");
        line("overflow-y: auto;");
        dedent();
        line("}");
        line();

        // Classes Panel
        line(".class-list {");
        indent();
        line("list-style: none;");
        dedent();
        line("}");
        line();

        line(".class-item {");
        indent();
        line("padding: 12px 15px;");
        line("border-radius: 6px;");
        line("cursor: pointer;");
        line("transition: background 0.2s;");
        line("display: flex;");
        line("justify-content: space-between;");
        line("align-items: center;");
        dedent();
        line("}");
        line();

        line(".class-item:hover {");
        indent();
        line("background: var(--bg-tertiary);");
        dedent();
        line("}");
        line();

        line(".class-item.active {");
        indent();
        line("background: var(--accent);");
        dedent();
        line("}");
        line();

        line(".class-badge {");
        indent();
        line("font-size: 11px;");
        line("padding: 3px 8px;");
        line("border-radius: 10px;");
        line("background: var(--bg-primary);");
        dedent();
        line("}");
        line();

        // Section
        line(".section {");
        indent();
        line("margin-bottom: 20px;");
        dedent();
        line("}");
        line();

        line(".section-title {");
        indent();
        line("font-size: 13px;");
        line("text-transform: uppercase;");
        line("color: var(--text-secondary);");
        line("margin-bottom: 10px;");
        line("letter-spacing: 0.5px;");
        dedent();
        line("}");
        line();

        // Constructor and method items
        line(".constructor-item,");
        line(".method-item {");
        indent();
        line("background: var(--bg-primary);");
        line("padding: 12px;");
        line("border-radius: 6px;");
        line("margin-bottom: 8px;");
        line("font-family: 'Consolas', 'Monaco', monospace;");
        line("font-size: 13px;");
        dedent();
        line("}");
        line();

        line(".method-item {");
        indent();
        line("cursor: pointer;");
        line("transition: border 0.2s;");
        line("border: 1px solid transparent;");
        dedent();
        line("}");
        line();

        line(".method-item:hover {");
        indent();
        line("border-color: var(--accent);");
        dedent();
        line("}");
        line();

        line(".method-item.not-callable {");
        indent();
        line("opacity: 0.5;");
        line("cursor: not-allowed;");
        dedent();
        line("}");
        line();

        line(".method-item.not-callable::after {");
        indent();
        line("content: \" (callback required)\";");
        line("color: var(--text-secondary);");
        line("font-size: 11px;");
        dedent();
        line("}");
        line();

        line(".method-name {");
        indent();
        line("color: var(--accent);");
        dedent();
        line("}");
        line();

        line(".method-params {");
        indent();
        line("color: var(--text-secondary);");
        dedent();
        line("}");
        line();

        line(".method-return {");
        indent();
        line("color: var(--success);");
        line("float: right;");
        dedent();
        line("}");
        line();

        // Objects Panel
        line(".object-item {");
        indent();
        line("background: var(--bg-primary);");
        line("padding: 12px;");
        line("border-radius: 6px;");
        line("margin-bottom: 8px;");
        line("display: flex;");
        line("justify-content: space-between;");
        line("align-items: center;");
        dedent();
        line("}");
        line();

        line(".object-info {");
        indent();
        line("display: flex;");
        line("flex-direction: column;");
        line("gap: 4px;");
        dedent();
        line("}");
        line();

        line(".object-id {");
        indent();
        line("font-family: monospace;");
        line("color: var(--accent);");
        dedent();
        line("}");
        line();

        line(".object-class {");
        indent();
        line("font-size: 12px;");
        line("color: var(--text-secondary);");
        dedent();
        line("}");
        line();

        line(".object-actions {");
        indent();
        line("display: flex;");
        line("gap: 6px;");
        dedent();
        line("}");
        line();

        // Modal
        line(".modal-overlay {");
        indent();
        line("display: none;");
        line("position: fixed;");
        line("top: 0;");
        line("left: 0;");
        line("right: 0;");
        line("bottom: 0;");
        line("background: rgba(0, 0, 0, 0.7);");
        line("z-index: 1000;");
        line("align-items: center;");
        line("justify-content: center;");
        dedent();
        line("}");
        line();

        line(".modal-overlay.active {");
        indent();
        line("display: flex;");
        dedent();
        line("}");
        line();

        line(".modal {");
        indent();
        line("background: var(--bg-secondary);");
        line("border-radius: 10px;");
        line("width: 500px;");
        line("max-width: 90vw;");
        line("max-height: 80vh;");
        line("overflow: hidden;");
        dedent();
        line("}");
        line();

        line(".modal-header {");
        indent();
        line("background: var(--bg-tertiary);");
        line("padding: 15px 20px;");
        line("display: flex;");
        line("justify-content: space-between;");
        line("align-items: center;");
        dedent();
        line("}");
        line();

        line(".modal-close {");
        indent();
        line("background: none;");
        line("border: none;");
        line("color: var(--text-secondary);");
        line("font-size: 24px;");
        line("cursor: pointer;");
        line("padding: 0;");
        line("line-height: 1;");
        dedent();
        line("}");
        line();

        line(".modal-close:hover {");
        indent();
        line("color: var(--text-primary);");
        dedent();
        line("}");
        line();

        line(".modal-body {");
        indent();
        line("padding: 20px;");
        line("max-height: 60vh;");
        line("overflow-y: auto;");
        dedent();
        line("}");
        line();

        // Form
        line(".form-group {");
        indent();
        line("margin-bottom: 15px;");
        dedent();
        line("}");
        line();

        line(".form-group label {");
        indent();
        line("display: block;");
        line("margin-bottom: 6px;");
        line("font-size: 13px;");
        line("color: var(--text-secondary);");
        dedent();
        line("}");
        line();

        line(".form-group input,");
        line(".form-group textarea,");
        line(".form-group select {");
        indent();
        line("width: 100%;");
        line("padding: 10px;");
        line("border: 1px solid var(--border);");
        line("border-radius: 6px;");
        line("background: var(--bg-primary);");
        line("color: var(--text-primary);");
        line("font-family: inherit;");
        line("font-size: 14px;");
        dedent();
        line("}");
        line();

        line(".form-group textarea {");
        indent();
        line("min-height: 80px;");
        line("resize: vertical;");
        line("font-family: monospace;");
        dedent();
        line("}");
        line();

        line(".form-group input:focus,");
        line(".form-group textarea:focus,");
        line(".form-group select:focus {");
        indent();
        line("outline: none;");
        line("border-color: var(--accent);");
        dedent();
        line("}");
        line();

        line(".form-hint {");
        indent();
        line("font-size: 11px;");
        line("color: var(--text-secondary);");
        line("margin-top: 4px;");
        dedent();
        line("}");
        line();

        // Console
        line(".console {");
        indent();
        line("background: #0d0d0d;");
        line("border-radius: 6px;");
        line("font-family: 'Consolas', 'Monaco', monospace;");
        line("font-size: 12px;");
        line("height: 100%;");
        line("display: flex;");
        line("flex-direction: column;");
        dedent();
        line("}");
        line();

        line(".console-header {");
        indent();
        line("padding: 10px 15px;");
        line("background: #1a1a1a;");
        line("display: flex;");
        line("justify-content: space-between;");
        line("align-items: center;");
        line("border-bottom: 1px solid #333;");
        dedent();
        line("}");
        line();

        line(".console-output {");
        indent();
        line("flex: 1;");
        line("padding: 15px;");
        line("overflow-y: auto;");
        line("max-height: calc(100vh - 320px);");
        dedent();
        line("}");
        line();

        line(".console-entry {");
        indent();
        line("margin-bottom: 15px;");
        line("padding-bottom: 15px;");
        line("border-bottom: 1px solid #222;");
        dedent();
        line("}");
        line();

        line(".console-entry:last-child {");
        indent();
        line("border-bottom: none;");
        dedent();
        line("}");
        line();

        line(".console-request {");
        indent();
        line("color: #6ab7ff;");
        line("margin-bottom: 5px;");
        dedent();
        line("}");
        line();

        line(".console-response {");
        indent();
        line("color: #98c379;");
        line("white-space: pre-wrap;");
        line("word-break: break-all;");
        dedent();
        line("}");
        line();

        line(".console-error {");
        indent();
        line("color: var(--error);");
        dedent();
        line("}");
        line();

        line(".console-timestamp {");
        indent();
        line("color: #666;");
        line("font-size: 11px;");
        dedent();
        line("}");
        line();

        // Empty state
        line(".empty-state {");
        indent();
        line("text-align: center;");
        line("padding: 40px;");
        line("color: var(--text-secondary);");
        dedent();
        line("}");
        line();

        line(".empty-state-icon {");
        indent();
        line("font-size: 48px;");
        line("margin-bottom: 15px;");
        dedent();
        line("}");
        line();

        // Scrollbar
        line("::-webkit-scrollbar {");
        indent();
        line("width: 8px;");
        dedent();
        line("}");
        line();

        line("::-webkit-scrollbar-track {");
        indent();
        line("background: var(--bg-primary);");
        dedent();
        line("}");
        line();

        line("::-webkit-scrollbar-thumb {");
        indent();
        line("background: var(--border);");
        line("border-radius: 4px;");
        dedent();
        line("}");
        line();

        line("::-webkit-scrollbar-thumb:hover {");
        indent();
        line("background: var(--bg-tertiary);");
        dedent();
        line("}");
        line();

        // Loading
        line(".loading {");
        indent();
        line("display: inline-block;");
        line("width: 16px;");
        line("height: 16px;");
        line("border: 2px solid var(--text-secondary);");
        line("border-top-color: var(--accent);");
        line("border-radius: 50%;");
        line("animation: spin 1s linear infinite;");
        dedent();
        line("}");
        line();

        line("@keyframes spin {");
        indent();
        line("to {");
        indent();
        line("transform: rotate(360deg);");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        // Responsive
        line("@media (max-width: 1200px) {");
        indent();
        line(".main-grid {");
        indent();
        line("grid-template-columns: 250px 1fr;");
        dedent();
        line("}");
        line();
        line(".console-panel {");
        indent();
        line("grid-column: 1 / -1;");
        line("max-height: 300px;");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        line("@media (max-width: 768px) {");
        indent();
        line(".main-grid {");
        indent();
        line("grid-template-columns: 1fr;");
        dedent();
        line("}");
        dedent();
        line("}");

        dedent();
        line("</style>");
        dedent();
        line("</head>");
        line();
    }

    void write_body_header() {
        line("<body>");
        indent();
        line("<header>");
        indent();
        line("<div class=\"container\">");
        indent();
        line("<h1>" + config_.module_name + " REST API Explorer</h1>");
        line("<div class=\"connection-bar\">");
        indent();
        line("<input type=\"text\" id=\"apiUrl\" value=\"http://localhost:8080\" placeholder=\"API "
             "URL\">");
        line("<button class=\"btn-primary\" onclick=\"connect()\">Connect</button>");
        line("<span id=\"statusIndicator\" class=\"status-indicator disconnected\">");
        indent();
        line("<span class=\"status-dot\"></span>");
        line("<span id=\"statusText\">Disconnected</span>");
        dedent();
        line("</span>");
        dedent();
        line("</div>");
        dedent();
        line("</div>");
        dedent();
        line("</header>");
        line();
    }

    void write_main_content() {
        line("<div class=\"container\">");
        indent();
        line("<div class=\"main-grid\">");
        indent();

        // Classes Panel
        line("<!-- Classes Panel -->");
        line("<div class=\"panel\">");
        indent();
        line("<div class=\"panel-header\">");
        indent();
        line("<span>&#128230; Classes</span>");
        line("<button class=\"btn-secondary btn-small\" "
             "onclick=\"refreshClasses()\">&#8635;</button>");
        dedent();
        line("</div>");
        line("<div class=\"panel-content\">");
        indent();
        line("<ul id=\"classList\" class=\"class-list\">");
        indent();
        line("<li class=\"empty-state\">");
        indent();
        line("<div class=\"empty-state-icon\">&#128237;</div>");
        line("<div>Connect to load classes</div>");
        dedent();
        line("</li>");
        dedent();
        line("</ul>");
        dedent();
        line("</div>");
        dedent();
        line("</div>");
        line();

        // Details Panel
        line("<!-- Details Panel -->");
        line("<div class=\"panel\">");
        indent();
        line("<div class=\"panel-header\">");
        indent();
        line("<span id=\"detailsTitle\">&#128203; Select a class</span>");
        line("<button class=\"btn-primary btn-small\" id=\"createBtn\" "
             "onclick=\"showCreateModal()\"");
        line("    style=\"display:none\">+ Create</button>");
        dedent();
        line("</div>");
        line("<div class=\"panel-content\" id=\"detailsContent\">");
        indent();
        line("<div class=\"empty-state\">");
        indent();
        line("<div class=\"empty-state-icon\">&#128072;</div>");
        line("<div>Select a class to view details</div>");
        dedent();
        line("</div>");
        dedent();
        line("</div>");
        dedent();
        line("</div>");
        line();

        // Objects & Console Panel
        line("<!-- Objects & Console Panel -->");
        line("<div class=\"panel console-panel\">");
        indent();
        line("<div class=\"panel-header\">");
        indent();
        line("<span>&#127919; Objects & Console</span>");
        line("<button class=\"btn-secondary btn-small\" "
             "onclick=\"refreshObjects()\">&#8635;</button>");
        dedent();
        line("</div>");
        line("<div class=\"panel-content\" style=\"padding: 0;\">");
        indent();

        // Objects Section
        line("<!-- Objects Section -->");
        line("<div style=\"padding: 15px; border-bottom: 1px solid var(--border);\">");
        indent();
        line("<div class=\"section-title\">Active Objects</div>");
        line("<div id=\"objectsList\">");
        indent();
        line("<div class=\"empty-state\" style=\"padding: 20px;\">");
        indent();
        line("<div>No objects created</div>");
        dedent();
        line("</div>");
        dedent();
        line("</div>");
        dedent();
        line("</div>");

        // Console Section
        line("<!-- Console Section -->");
        line("<div class=\"console\">");
        indent();
        line("<div class=\"console-header\">");
        indent();
        line("<span>Console Output</span>");
        line("<button class=\"btn-secondary btn-small\" onclick=\"clearConsole()\">Clear</button>");
        dedent();
        line("</div>");
        line("<div class=\"console-output\" id=\"consoleOutput\">");
        line("</div>");
        dedent();
        line("</div>");

        dedent();
        line("</div>");
        dedent();
        line("</div>");

        dedent();
        line("</div>");
        dedent();
        line("</div>");
        line();
    }

    void write_modals() {
        // Create Object Modal
        line("<!-- Create Object Modal -->");
        line("<div class=\"modal-overlay\" id=\"createModal\">");
        indent();
        line("<div class=\"modal\">");
        indent();
        line("<div class=\"modal-header\">");
        indent();
        line("<span id=\"createModalTitle\">Create Object</span>");
        line(
            "<button class=\"modal-close\" onclick=\"closeModal('createModal')\">&times;</button>");
        dedent();
        line("</div>");
        line("<div class=\"modal-body\">");
        indent();
        line("<div class=\"form-group\">");
        indent();
        line("<label>Constructor</label>");
        line("<select id=\"constructorSelect\" onchange=\"updateConstructorParams()\"></select>");
        dedent();
        line("</div>");
        line("<div id=\"constructorParams\"></div>");
        line("<button class=\"btn-primary\" onclick=\"createObject()\" style=\"width: "
             "100%;\">Create</button>");
        dedent();
        line("</div>");
        dedent();
        line("</div>");
        dedent();
        line("</div>");
        line();

        // Call Method Modal
        line("<!-- Call Method Modal -->");
        line("<div class=\"modal-overlay\" id=\"methodModal\">");
        indent();
        line("<div class=\"modal\">");
        indent();
        line("<div class=\"modal-header\">");
        indent();
        line("<span id=\"methodModalTitle\">Call Method</span>");
        line(
            "<button class=\"modal-close\" onclick=\"closeModal('methodModal')\">&times;</button>");
        dedent();
        line("</div>");
        line("<div class=\"modal-body\">");
        indent();
        line("<div class=\"form-group\">");
        indent();
        line("<label>Object</label>");
        line("<select id=\"methodObjectSelect\"></select>");
        dedent();
        line("</div>");
        line("<div id=\"methodParams\"></div>");
        line("<button class=\"btn-primary\" onclick=\"callMethod()\" style=\"width: "
             "100%;\">Execute</button>");
        dedent();
        line("</div>");
        dedent();
        line("</div>");
        dedent();
        line("</div>");
        line();
    }

    void write_javascript() {
        line("<script>");
        indent();

        // State variables
        line("// State");
        line("let apiUrl = 'http://localhost:8080';");
        line("let classes = [];");
        line("let objects = [];");
        line("let selectedClass = null;");
        line("let selectedClassInfo = null;");
        line("let currentMethod = null;");
        line();

        // API Functions
        line("// API Functions");
        line("async function apiCall(method, endpoint, body = null) {");
        indent();
        line("const url = `${apiUrl}${endpoint}`;");
        line("const options = {");
        indent();
        line("method,");
        line("headers: { 'Content-Type': 'application/json' }");
        dedent();
        line("};");
        line("if (body !== null) {");
        indent();
        line("options.body = JSON.stringify(body);");
        dedent();
        line("}");
        line();
        line("const startTime = Date.now();");
        line("try {");
        indent();
        line("const response = await fetch(url, options);");
        line("const data = await response.json();");
        line("const duration = Date.now() - startTime;");
        line();
        line("logToConsole(method, endpoint, body, data, duration);");
        line("return data;");
        dedent();
        line("} catch (error) {");
        indent();
        line("logToConsole(method, endpoint, body, { error: true, message: error.message }, "
             "Date.now() - startTime, true);");
        line("throw error;");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        // Console logging
        line("function logToConsole(method, endpoint, request, response, duration, isError = "
             "false) {");
        indent();
        line("const output = document.getElementById('consoleOutput');");
        line("const entry = document.createElement('div');");
        line("entry.className = 'console-entry';");
        line();
        line("const timestamp = new Date().toLocaleTimeString();");
        line("const requestStr = request ? `\\n${JSON.stringify(request, null, 2)}` : '';");
        line();
        line("entry.innerHTML = `");
        line("    <div class=\"console-timestamp\">${timestamp} (${duration}ms)</div>");
        line("    <div class=\"console-request\">${method} ${endpoint}${requestStr}</div>");
        line("    <div class=\"${isError ? 'console-error' : "
             "'console-response'}\">${JSON.stringify(response, null, 2)}</div>");
        line("`;");
        line();
        line("output.insertBefore(entry, output.firstChild);");
        dedent();
        line("}");
        line();

        line("function clearConsole() {");
        indent();
        line("document.getElementById('consoleOutput').innerHTML = '';");
        dedent();
        line("}");
        line();

        // Connection
        line("// Connection");
        line("async function connect() {");
        indent();
        line("apiUrl = document.getElementById('apiUrl').value.replace(/\\/$/, '');");
        line();
        line("try {");
        indent();
        line("const result = await apiCall('GET', '/health');");
        line("updateStatus(true, result.module || 'Connected');");
        line("await refreshClasses();");
        line("await refreshObjects();");
        dedent();
        line("} catch (error) {");
        indent();
        line("updateStatus(false, 'Connection failed');");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        line("function updateStatus(connected, text) {");
        indent();
        line("const indicator = document.getElementById('statusIndicator');");
        line("const statusText = document.getElementById('statusText');");
        line();
        line("indicator.className = `status-indicator ${connected ? 'connected' : "
             "'disconnected'}`;");
        line("statusText.textContent = text;");
        dedent();
        line("}");
        line();

        // Classes
        line("// Classes");
        line("async function refreshClasses() {");
        indent();
        line("try {");
        indent();
        line("const result = await apiCall('GET', '/api/classes');");
        line("if (!result.error) {");
        indent();
        line("classes = result.data;");
        line("renderClassList();");
        dedent();
        line("}");
        dedent();
        line("} catch (error) {");
        indent();
        line("console.error('Failed to load classes:', error);");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        line("function renderClassList() {");
        indent();
        line("const list = document.getElementById('classList');");
        line();
        line("if (classes.length === 0) {");
        indent();
        line("list.innerHTML = `");
        line("    <li class=\"empty-state\">");
        line("        <div class=\"empty-state-icon\">&#128237;</div>");
        line("        <div>No classes registered</div>");
        line("    </li>");
        line("`;");
        line("return;");
        dedent();
        line("}");
        line();
        line("list.innerHTML = classes.map(cls => `");
        line("    <li class=\"class-item ${selectedClass === cls ? 'active' : ''}\" "
             "onclick=\"selectClass('${cls}')\">");
        line("        <span>${cls}</span>");
        line("    </li>");
        line("`).join('');");
        dedent();
        line("}");
        line();

        line("async function selectClass(className) {");
        indent();
        line("selectedClass = className;");
        line("renderClassList();");
        line();
        line("try {");
        indent();
        line("const result = await apiCall('GET', `/api/classes/${className}`);");
        line("if (!result.error) {");
        indent();
        line("selectedClassInfo = result.data;");
        line("renderClassDetails();");
        dedent();
        line("}");
        dedent();
        line("} catch (error) {");
        indent();
        line("console.error('Failed to load class info:', error);");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        line("function renderClassDetails() {");
        indent();
        line("if (!selectedClassInfo) return;");
        line();
        line("document.getElementById('detailsTitle').textContent = `\\u{1F4CB} "
             "${selectedClassInfo.name}`;");
        line("document.getElementById('createBtn').style.display = 'inline-block';");
        line();
        line("const content = document.getElementById('detailsContent');");
        line();
        line("// Constructors");
        line("let constructorsHtml = '<div class=\"section\"><div "
             "class=\"section-title\">Constructors</div>';");
        line("if (selectedClassInfo.constructors.length === 0) {");
        indent();
        line("constructorsHtml += '<div class=\"constructor-item\">Default constructor</div>';");
        dedent();
        line("} else {");
        indent();
        line("selectedClassInfo.constructors.forEach((ctor, i) => {");
        indent();
        line("const params = ctor.params.length ? ctor.params.join(', ') : 'void';");
        line("const callable = ctor.rest_callable !== false;");
        line("constructorsHtml += `");
        line("    <div class=\"constructor-item ${callable ? '' : 'method-item not-callable'}\">");
        line("        <span class=\"method-name\">${selectedClassInfo.name}</span>(<span "
             "class=\"method-params\">${params}</span>)");
        line("    </div>");
        line("`;");
        dedent();
        line("});");
        dedent();
        line("}");
        line("constructorsHtml += '</div>';");
        line();
        line("// Methods");
        line("let methodsHtml = '<div class=\"section\"><div "
             "class=\"section-title\">Methods</div>';");
        line("if (selectedClassInfo.methods.length === 0) {");
        indent();
        line("methodsHtml += '<div class=\"empty-state\" style=\"padding: 20px;\">No "
             "methods</div>';");
        dedent();
        line("} else {");
        indent();
        line("selectedClassInfo.methods.forEach(method => {");
        indent();
        line("const params = method.params.length ? method.params.join(', ') : 'void';");
        line("const callable = method.rest_callable !== false;");
        line("methodsHtml += `");
        line("    <div class=\"method-item ${callable ? '' : 'not-callable'}\" ");
        line("         ${callable ? `onclick=\"showMethodModal('${method.name}')\"` : ''}>");
        line("        <span class=\"method-return\">${method.return_type}</span>");
        line("        <span class=\"method-name\">${method.name}</span>(<span "
             "class=\"method-params\">${params}</span>)");
        line("    </div>");
        line("`;");
        dedent();
        line("});");
        dedent();
        line("}");
        line("methodsHtml += '</div>';");
        line();
        line("content.innerHTML = constructorsHtml + methodsHtml;");
        dedent();
        line("}");
        line();

        // Objects
        line("// Objects");
        line("async function refreshObjects() {");
        indent();
        line("try {");
        indent();
        line("const result = await apiCall('GET', '/api/objects');");
        line("if (!result.error) {");
        indent();
        line("objects = result.data;");
        line("renderObjectsList();");
        dedent();
        line("}");
        dedent();
        line("} catch (error) {");
        indent();
        line("console.error('Failed to load objects:', error);");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        line("function renderObjectsList() {");
        indent();
        line("const list = document.getElementById('objectsList');");
        line();
        line("if (objects.length === 0) {");
        indent();
        line("list.innerHTML = '<div class=\"empty-state\" style=\"padding: 20px;\">No objects "
             "created</div>';");
        line("return;");
        dedent();
        line("}");
        line();
        line("list.innerHTML = objects.map(obj => `");
        line("    <div class=\"object-item\">");
        line("        <div class=\"object-info\">");
        line("            <span class=\"object-id\">${obj.id}</span>");
        line("            <span class=\"object-class\">${obj.class}</span>");
        line("        </div>");
        line("        <div class=\"object-actions\">");
        line("            <button class=\"btn-danger btn-small\" "
             "onclick=\"deleteObject('${obj.id}')\">&#128465;</button>");
        line("        </div>");
        line("    </div>");
        line("`).join('');");
        dedent();
        line("}");
        line();

        line("async function deleteObject(id) {");
        indent();
        line("if (!confirm(`Delete object ${id}?`)) return;");
        line();
        line("try {");
        indent();
        line("await apiCall('DELETE', `/api/objects/${id}`);");
        line("await refreshObjects();");
        dedent();
        line("} catch (error) {");
        indent();
        line("console.error('Failed to delete object:', error);");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        // Create Object Modal
        line("// Create Object Modal");
        line("function showCreateModal() {");
        indent();
        line("if (!selectedClassInfo) return;");
        line();
        line("document.getElementById('createModalTitle').textContent = `Create "
             "${selectedClassInfo.name}`;");
        line();
        line("const select = document.getElementById('constructorSelect');");
        line("const callableCtors = selectedClassInfo.constructors.filter(c => c.rest_callable !== "
             "false);");
        line();
        line("if (callableCtors.length === 0) {");
        indent();
        line("// Default constructor");
        line("select.innerHTML = '<option value=\"-1\">Default constructor</option>';");
        dedent();
        line("} else {");
        indent();
        line("select.innerHTML = callableCtors.map((ctor, i) => {");
        indent();
        line("const params = ctor.params.length ? ctor.params.join(', ') : 'void';");
        line("return `<option value=\"${i}\">${selectedClassInfo.name}(${params})</option>`;");
        dedent();
        line("}).join('');");
        dedent();
        line("}");
        line();
        line("updateConstructorParams();");
        line("document.getElementById('createModal').classList.add('active');");
        dedent();
        line("}");
        line();

        line("function updateConstructorParams() {");
        indent();
        line("const select = document.getElementById('constructorSelect');");
        line("const paramsDiv = document.getElementById('constructorParams');");
        line("const index = parseInt(select.value);");
        line();
        line("if (index < 0 || !selectedClassInfo.constructors[index]) {");
        indent();
        line("paramsDiv.innerHTML = '';");
        line("return;");
        dedent();
        line("}");
        line();
        line("const ctor = selectedClassInfo.constructors.filter(c => c.rest_callable !== "
             "false)[index];");
        line("if (!ctor || ctor.params.length === 0) {");
        indent();
        line("paramsDiv.innerHTML = '';");
        line("return;");
        dedent();
        line("}");
        line();
        line("paramsDiv.innerHTML = ctor.params.map((param, i) => `");
        line("    <div class=\"form-group\">");
        line("        <label>Parameter ${i + 1} (${param})</label>");
        line("        <input type=\"text\" id=\"ctorParam${i}\" "
             "placeholder=\"${getPlaceholder(param)}\">");
        line("        <div class=\"form-hint\">${getParamHint(param)}</div>");
        line("    </div>");
        line("`).join('');");
        dedent();
        line("}");
        line();

        line("function getPlaceholder(type) {");
        indent();
        line("if (type.includes('string')) return '\"value\"';");
        line("if (type.includes('vector<double>')) return '[1.0, 2.0, 3.0]';");
        line("if (type.includes('vector<int>')) return '[1, 2, 3]';");
        line("if (type.includes('Vector3')) return '[x, y, z]';");
        line("if (type.includes('double') || type.includes('float')) return '0.0';");
        line("if (type.includes('int')) return '0';");
        line("if (type.includes('bool')) return 'true';");
        line("return 'Object_ID';");
        dedent();
        line("}");
        line();

        line("function getParamHint(type) {");
        indent();
        line("if (type.includes('vector<')) return 'JSON array of values';");
        line("if (type.includes('string')) return 'String value';");
        line("if (type.includes('double') || type.includes('float') || type.includes('int')) "
             "return 'Numeric value';");
        line("if (type.includes('bool')) return 'true or false';");
        line("return 'For objects, use the object ID (e.g., \"Surface_1\")';");
        dedent();
        line("}");
        line();

        line("async function createObject() {");
        indent();
        line("if (!selectedClassInfo) return;");
        line();
        line("const select = document.getElementById('constructorSelect');");
        line("const index = parseInt(select.value);");
        line();
        line("let params = [];");
        line();
        line("if (index >= 0) {");
        indent();
        line("const callableCtors = selectedClassInfo.constructors.filter(c => c.rest_callable !== "
             "false);");
        line("const ctor = callableCtors[index];");
        line("if (ctor && ctor.params.length > 0) {");
        indent();
        line("for (let i = 0; i < ctor.params.length; i++) {");
        indent();
        line("const input = document.getElementById(`ctorParam${i}`);");
        line("try {");
        indent();
        line("params.push(JSON.parse(input.value));");
        dedent();
        line("} catch {");
        indent();
        line("params.push(input.value);");
        dedent();
        line("}");
        dedent();
        line("}");
        dedent();
        line("}");
        dedent();
        line("}");
        line();
        line("try {");
        indent();
        line("const result = await apiCall('POST', `/api/objects/${selectedClassInfo.name}`, "
             "params);");
        line("if (!result.error) {");
        indent();
        line("closeModal('createModal');");
        line("await refreshObjects();");
        dedent();
        line("}");
        dedent();
        line("} catch (error) {");
        indent();
        line("console.error('Failed to create object:', error);");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        // Method Call Modal
        line("// Method Call Modal");
        line("function showMethodModal(methodName) {");
        indent();
        line("if (!selectedClassInfo) return;");
        line();
        line("currentMethod = selectedClassInfo.methods.find(m => m.name === methodName);");
        line("if (!currentMethod || currentMethod.rest_callable === false) return;");
        line();
        line("document.getElementById('methodModalTitle').textContent = `Call ${methodName}()`;");
        line();
        line("// Object select");
        line("const objectSelect = document.getElementById('methodObjectSelect');");
        line("const classObjects = objects.filter(o => o.class === selectedClassInfo.name);");
        line();
        line("if (classObjects.length === 0) {");
        indent();
        line("objectSelect.innerHTML = '<option value=\"\">No objects available - create one "
             "first</option>';");
        dedent();
        line("} else {");
        indent();
        line("objectSelect.innerHTML = classObjects.map(obj =>");
        line("    `<option value=\"${obj.id}\">${obj.id}</option>`");
        line(").join('');");
        dedent();
        line("}");
        line();
        line("// Method params");
        line("const paramsDiv = document.getElementById('methodParams');");
        line("if (currentMethod.params.length === 0) {");
        indent();
        line("paramsDiv.innerHTML = '<div class=\"form-hint\">No parameters required</div>';");
        dedent();
        line("} else {");
        indent();
        line("paramsDiv.innerHTML = currentMethod.params.map((param, i) => `");
        line("    <div class=\"form-group\">");
        line("        <label>Parameter ${i + 1} (${param})</label>");
        line("        <input type=\"text\" id=\"methodParam${i}\" "
             "placeholder=\"${getPlaceholder(param)}\">");
        line("        <div class=\"form-hint\">${getParamHint(param)}</div>");
        line("    </div>");
        line("`).join('');");
        dedent();
        line("}");
        line();
        line("document.getElementById('methodModal').classList.add('active');");
        dedent();
        line("}");
        line();

        line("async function callMethod() {");
        indent();
        line("if (!currentMethod) return;");
        line();
        line("const objectId = document.getElementById('methodObjectSelect').value;");
        line("if (!objectId) {");
        indent();
        line("alert('Please select an object');");
        line("return;");
        dedent();
        line("}");
        line();
        line("let params = [];");
        line("for (let i = 0; i < currentMethod.params.length; i++) {");
        indent();
        line("const input = document.getElementById(`methodParam${i}`);");
        line("try {");
        indent();
        line("params.push(JSON.parse(input.value));");
        dedent();
        line("} catch {");
        indent();
        line("params.push(input.value);");
        dedent();
        line("}");
        dedent();
        line("}");
        line();
        line("try {");
        indent();
        line("await apiCall('POST', `/api/objects/${objectId}/${currentMethod.name}`, params);");
        line("closeModal('methodModal');");
        dedent();
        line("} catch (error) {");
        indent();
        line("console.error('Failed to call method:', error);");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        // Modal helpers
        line("// Modal helpers");
        line("function closeModal(id) {");
        indent();
        line("document.getElementById(id).classList.remove('active');");
        dedent();
        line("}");
        line();

        line("// Close modal on overlay click");
        line("document.querySelectorAll('.modal-overlay').forEach(overlay => {");
        indent();
        line("overlay.addEventListener('click', (e) => {");
        indent();
        line("if (e.target === overlay) {");
        indent();
        line("overlay.classList.remove('active');");
        dedent();
        line("}");
        dedent();
        line("});");
        dedent();
        line("});");
        line();

        line("// Keyboard shortcuts");
        line("document.addEventListener('keydown', (e) => {");
        indent();
        line("if (e.key === 'Escape') {");
        indent();
        line("document.querySelectorAll('.modal-overlay').forEach(m => "
             "m.classList.remove('active'));");
        dedent();
        line("}");
        dedent();
        line("});");
        line();

        line("// Auto-connect on load");
        line("window.onload = () => {");
        indent();
        line("connect();");
        dedent();
        line("};");

        dedent();
        line("</script>");
    }

    void write_closing() {
        dedent();
        line("</body>");
        line();
        line("</html>");
    }
};