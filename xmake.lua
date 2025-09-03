includes("info.lua")
local info = build_info(info_lua)

add_rules("mode.debug", "mode.release")

option("include_logging")
    set_showmenu(true)
    set_description("Include verbose logging on run")
    add_defines("VERBOSE")


target("doorstop")
    set_kind("shared")
    set_optimize("smallest")
    add_options("include_logging")
    local load_events = {}

    if is_os("windows") then
        includes("src/windows/build_tools/proxygen.lua")
        add_proxydef(load_events)

        includes("src/windows/build_tools/rcgen.lua")
        add_rc(load_events, info)

        add_files("src/windows/*.c")
        add_defines("UNICODE")
        add_links("shell32", "kernel32", "user32")
    end

    if is_os("linux") or is_os("macosx") then
        add_files("src/nix/*.c")
        add_files("src/nix/plthook/*.c")
        add_links("dl")
        if is_mode("debug") then
            set_symbols("debug")
            set_optimize("none")
        end
    end

    if is_plat("windows") then
        add_cxflags("-GS-", "-Ob2", "-MT", "-GL-", "-FS")
        add_shflags("-nodefaultlib",
                    "-entry:DllEntry",
                    "-dynamicbase:no",
                    {force=true})
    end

    if is_plat("mingw") then
        add_shflags("-nostdlib", "-nolibc", {force=true})

        if is_arch("i386") then
            add_shflags("-e _DllEntry", "-Wl,--enable-stdcall-fixup", {force=true})
        elseif is_arch("x86_64") then
            add_shflags("-e DllEntry", {force=true})
        end
    end

    add_files("src/*.c")
    add_files("src/config/*.c")
    add_files("src/util/*.c")
    add_files("src/runtimes/*.c")

    on_load(function(target)
        for i, event in ipairs(load_events) do
            event(target, import, io)
        end
    end)

    after_build(function(target)
        io.writefile(path.join(target:targetdir(), ".doorstop_version"),
            info.version.major.."."..info.version.minor.."."..info.version.patch..info.version.release)
    end)

    if is_os("macosx") then
        -- Build x86_64 binary
        target("doorstop_x86_64")
            add_options("include_logging")
            set_kind("shared")
            set_arch("x86_64")
            set_optimize("smallest")
            add_files("src/*.c")
            add_files("src/config/*.c")
            add_files("src/util/*.c")
            add_files("src/runtimes/*.c")
            add_files("src/nix/*.c")
            add_files("src/nix/plthook/*.c")
            add_links("dl")
            if is_mode("debug") then
                set_symbols("debug")
                set_optimize("none")
            end

        -- Build arm64 binary
        target("doorstop_arm64")
            add_options("include_logging")
            set_kind("shared")
            set_arch("arm64")
            set_optimize("smallest")
            add_files("src/*.c")
            add_files("src/config/*.c")
            add_files("src/util/*.c")
            add_files("src/runtimes/*.c")
            add_files("src/nix/*.c")
            add_files("src/nix/plthook/*.c")
            add_links("dl")
            if is_mode("debug") then
                set_symbols("debug")
                set_optimize("none")
            end

        -- Combine the binaries into a Universal Binary
        after_build(function (target)
            os.execv("sleep", {"5"}) -- Give time for both builds to finish (workaround)
            local build_mode = is_mode("debug") and "debug" or "release"
            local targetdir = target:targetdir()
            os.mkdir(path.join(targetdir, "..", "..", "universal", build_mode))
            os.execv("lipo", {"-create", "-output", path.join(targetdir, "..", "..", "universal", build_mode, "libdoorstop.dylib"), path.join(targetdir, "..", "..", "x86_64", build_mode, "libdoorstop_x86_64.dylib"), path.join(targetdir, "libdoorstop_arm64.dylib")})
        end)
    end
