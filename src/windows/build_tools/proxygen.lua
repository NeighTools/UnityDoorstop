
function add_proxydef(load_events)
    filename = "src/windows/proxy/proxylist.txt"

    table.insert(load_events, function(target, import, io)
        import("util", {rootdir="src/windows/build_tools"})

        local tmpl_dir = path.join(path.directory(path.directory(filename)), "build_tools")

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

        util.write_template(
            path.join(tmpl_dir, "proxy.c.in"),
            path.join("build", "proxy.c"),
            {
                PROXY_DEFS = proxy_defs,
                PROXY_ADDS = proxy_adds,
                PROXY_FUNCS = proxy_funcs
            }
        )
        util.write_template(
            path.join(tmpl_dir, "dll.def.in"),
            path.join("build", "dll.def"),
            {
                EXPORT_FUNCS = exports_funcs
            }
        )
    end)

    add_files("build/proxy.c")

    if is_plat("windows") then add_files("build/dll.def") end
    if is_plat("mingw") then add_shflags("build/dll.def", {force = true}) end
end
