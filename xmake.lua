add_rules("mode.debug", "mode.release")

target("doorstop")
    set_kind("shared")
    set_optimize("smallest")

    if is_plat("windows") then
        add_files("src/windows/*.c")

        add_cxflags("-GS-", "-Ob2", "-MT", "-FS", "-GL-")
        add_shflags("-nodefaultlib",
                    "-entry:DllEntry",
                    "-dynamicbase kernel32.lib",
                    { force = true })
    end

    add_files("src/*.c")