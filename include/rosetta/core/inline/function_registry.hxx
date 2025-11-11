namespace rosetta::core {

    inline FunctionRegistry& FunctionRegistry::instance() {
        static FunctionRegistry registry;
        return registry;
    }
    
    template <typename Ret, typename... Args>
    inline FunctionMetadata& FunctionRegistry::register_function(const std::string& name, 
                                                                 Ret (*ptr)(Args...)) {
        auto meta = std::make_unique<FunctionMetadata>(name);
        meta->register_function(ptr);
        
        auto* ptr_meta = meta.get();
        functions_[name] = std::move(meta);
        
        return *ptr_meta;
    }
    
    inline FunctionMetadata& FunctionRegistry::get(const std::string& name) {
        auto it = functions_.find(name);
        if (it == functions_.end()) {
            throw std::runtime_error("Function not registered: " + name);
        }
        return *it->second;
    }
    
    inline const FunctionMetadata& FunctionRegistry::get(const std::string& name) const {
        auto it = functions_.find(name);
        if (it == functions_.end()) {
            throw std::runtime_error("Function not registered: " + name);
        }
        return *it->second;
    }
    
    inline bool FunctionRegistry::has_function(const std::string& name) const {
        return functions_.find(name) != functions_.end();
    }
    
    inline std::vector<std::string> FunctionRegistry::list_functions() const {
        std::vector<std::string> names;
        names.reserve(functions_.size());
        for (const auto& [name, _] : functions_) {
            names.push_back(name);
        }
        return names;
    }
    
    inline size_t FunctionRegistry::size() const {
        return functions_.size();
    }
    
    inline void FunctionRegistry::clear() {
        functions_.clear();
    }
    
    inline Any FunctionRegistry::invoke(const std::string& name, std::vector<Any> args) const {
        return get(name).invoke(std::move(args));
    }

} // namespace rosetta::core
