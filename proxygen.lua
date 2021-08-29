function add_proxydef(filename)
    on_load(function(target)
        local ptr_type = ""
        if is_arch("x86") then
            ptr_type = "DWORD"
        elseif is_arch("x64") then
            ptr_type = "QWORD"
        end

        local asm_defs = ""
        local asm_vars = ""
        local asm_jmps = ""
        local proxy_defs = ""
        local proxy_adds = ""
        local exports_funcs = ""

        local funcs = io.readfile(filename):split("\n")
        for i, name in ipairs(funcs) do
            local func = string.trim(name)
            asm_defs = asm_defs .. format("public %s\r\n", func)
            asm_defs = asm_defs .. format("public __%s__\r\n", func)

            asm_vars = asm_vars .. format("  __%s__ %s 0\r\n", func, ptr_type)

            asm_jmps = asm_jmps .. format("%s:\r\n", func)
            asm_jmps = asm_jmps ..
                           format("  jmp %s ptr __%s__\r\n", ptr_type, func)

            proxy_defs = proxy_defs ..
                             format("extern FARPROC __%s__;\r\n", func)
            proxy_adds = proxy_adds ..
                             format(
                                 "__%s__ = GetProcAddress((HMODULE)dll, \"%s\");\r\n",
                                 func, func)

            exports_funcs = exports_funcs .. func .. "\r\n"
        end

        local tmpl_dir = path.directory(filename)

        -- Generate proxy files manually because xmake employs some weird cache that
        -- breaks initial builds.
        function interpolate(s, tab)
            return (s:gsub('($%b{})',
                           function(w) return tab[w:sub(3, -2)] or w end))
        end

        function write(name)
            local content = io.readfile(path.join(tmpl_dir, name .. ".in"))
            io.writefile(path.join("build", name), interpolate(content, {
                ASM_DEFS = asm_defs,
                ASM_VARS = asm_vars,
                ASM_JMPS = asm_jmps,
                PROXY_DEFS = proxy_defs,
                PROXY_ADDS = proxy_adds,
                EXPORT_FUNCS = exports_funcs
            }))
        end

        write("dllproxy.asm")
        write("proxy.c")
        write("dll.def")
    end)
    add_files("build/dllproxy.asm")
    add_files("build/proxy.c")
    add_shflags("-def:build/dll.def", {force = true})
end
