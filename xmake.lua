add_rules("mode.debug", "mode.release")
includes("proxygen.lua")

option("include_logging")
    set_showmenu(true)
    set_description("Include verbose logging on run")
    add_defines("VERBOSE")


target("doorstop")
    set_kind("shared")
    set_optimize("smallest")
    add_options("include_logging")

    if is_os("windows") then
        add_proxydef("src/windows/proxy/proxylist.txt")
        add_files("src/windows/*.c")
        add_defines("UNICODE")
    end

    add_links("shell32", "kernel32", "user32")

    if is_plat("windows") then
        add_cxflags("-GS-", "-Ob2", "-MT", "-GL-", "-FS")
        add_shflags("-nodefaultlib",
                    "-entry:DllEntry",
                    "-dynamicbase:no",
                    {force=true})
    end

    if is_plat("mingw") then
        add_shflags("-nodefaultlibs", "-nostdlib", "-nolibc", "-e DllEntry", {force=true})
    end

    add_files("src/*.c")
    add_files("src/config/*.c")
    add_files("src/util/*.c")
    add_files("src/runtimes/*.c")