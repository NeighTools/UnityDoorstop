function add_proxydef(filename)

    on_load(function(target)

        local tmpl_dir = path.directory(filename)

        -- Generate proxy files manually because xmake employs some weird cache that
        -- breaks initial builds.
        function interpolate(s, tab)
            return (s:gsub('($%b{})',
                           function(w) return tab[w:sub(3, -2)] or w end))
        end

        function write(name, replacements)
            local content = io.readfile(path.join(tmpl_dir, name .. ".in"))
            io.writefile(path.join("build", name),
                         interpolate(content, replacements))
        end

        local proxy_defs = ""
        local proxy_adds = ""
        local proxy_funcs = ""
        local exports_funcs = ""

        local funcs = io.readfile(filename):split("\n")
        for i, name in ipairs(funcs) do
            local func = string.trim(name)
            proxy_defs = proxy_defs ..
                             format("static FARPROC __%s__;\r\n", func)
            proxy_adds = proxy_adds ..
                             format(
                                 "__%s__ = GetProcAddress((HMODULE)dll, \"%s\");\r\n",
                                 func, func)
            proxy_funcs = proxy_funcs ..
                              format("void *exp_%s() { return __%s__(); }",
                                     func, func) .. "\r\n"
            exports_funcs =
                exports_funcs .. format("%s = exp_%s", func, func) .. "\r\n"
        end

        write("proxy.c", {
            PROXY_DEFS = proxy_defs,
            PROXY_ADDS = proxy_adds,
            PROXY_FUNCS = proxy_funcs
        })
        write("dll.def", {EXPORT_FUNCS = exports_funcs})
    end)

    add_files("build/proxy.c")

    if is_plat("windows") then add_files("build/dll.def") end
    if is_plat("mingw") then add_shflags("build/dll.def", {force = true}) end
end
