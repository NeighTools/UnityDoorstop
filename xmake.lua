add_rules("mode.debug", "mode.release")
includes("proxygen.lua")

target("doorstop")
    set_kind("shared")
    set_optimize("smallest")

    if is_plat("windows") then
        add_proxydef("src/windows/proxy/proxylist.txt")
        add_files("src/windows/*.c")

        add_cxflags("-GS-", "-Ob2", "-MT", "-FS", "-GL-")
        add_shflags("-nodefaultlib",
                    "-entry:DllEntry",
                    "-dynamicbase kernel32.lib",
                    "-dynamicbase shell32.lib",
                    {force=true})
        add_defines("UNICODE")
    end

    add_files("src/*.c")    