add_rules("mode.debug", "mode.release")
includes("proxygen.lua")

option("include_logging", { default = false, showmenu = true, description = "Include verbose logging on run" })
    add_defines("VERBOSE")


target("doorstop")
    set_kind("shared")
    set_optimize("smallest")
    add_options("include_logging")

    if is_plat("windows") then
        add_proxydef("src/windows/proxy/proxylist.txt")
        add_files("src/windows/*.c")

        add_cxflags("-GS-", "-Ob2", "-MT", "-FS", "-GL-")
        add_shflags("-nodefaultlib",
                    "-entry:DllEntry",
                    "-dynamicbase:no",
                    {force=true})
		add_links("shell32", "kernel32", "user32")
        add_defines("UNICODE")
    end

    add_files("src/*.c")