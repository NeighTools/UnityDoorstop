includes("info.lua")
local info = build_info(info_lua)
set_project(build_info().name)
set_version(build_info().version.major.."."..build_info().version.minor.."."..build_info().version.patch..build_info().version.release)

local buildarches
if is_os("windows") or is_os("linux") then
    buildarches = {"x86", "x64"}
elseif is_os("macosx") then
    buildarches = {"arm64", "arm64e", "x86_64"}
end

add_rules("mode.debug", "mode.release")

includes("@builtin/xpack")

for _, arch in ipairs(buildarches) do
    target("doorstop_"..arch)
        set_basename("doorstop")
        set_arch(arch)
        set_default(is_arch(arch))
        set_kind("shared")
        set_optimize("smallest")
        if is_mode("debug") then
            set_symbols("debug")
            set_optimize("none")
            add_defines("VERBOSE")
        end
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
            -- Add platform-specific plthook files
            if is_os("linux") then
                add_files("src/nix/plthook/plthook_elf.c")
            elseif is_os("macosx") then
                add_files("src/nix/plthook/plthook_osx.c")
            end
            add_links("dl")
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
            elseif is_arch("x64", "x86_64") then
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
            io.writefile(path.join(target:targetdir(), ".doorstop_version"), target:get("version"))
        end)
end

xpack("doorstop")
    set_formats("zip")
    set_basename("doorstop_$(plat)_$(mode)")

    for _, arch in ipairs(buildarches) do
        add_targets("doorstop_"..arch)
    end

    on_installcmd(function (package, batchcmds)
        local lipoargs = nil
        local targetfile
        for _, target in ipairs(package:targets()) do
            local installdir
            if package:is_plat("macosx") then
                installdir = package:installdir("universal")
            else
                installdir = package:installdir(target:get("arch"))
            end

            batchcmds:mkdir(installdir)
            batchcmds:cp(path.join(path.directory(target:targetfile()), ".doorstop_version"), installdir)

            if package:is_plat("windows") then
                batchcmds:cp("assets/windows/doorstop_config.ini", installdir)
            else
                batchcmds:cp("assets/nix/run.sh", installdir)
            end

            if package:is_plat("macosx") then
                lipoargs = format("%s -arch %s %s", lipoargs or "-create", target:get("arch"), target:targetfile())
                targetfile = target:targetfile()
            elseif package:is_plat("windows") then
                batchcmds:cp(target:targetfile(), path.join(installdir, "winhttp.dll"))
            else
                batchcmds:cp(target:targetfile(), installdir)
            end
        end
        if lipoargs then
            lipoargs = format("%s -output %s/%s", lipoargs, package:installdir("universal"), path.filename(targetfile))
            --batchcmds:vlua("lipo", lipoargs)
            batchcmds:execv(os.programfile(), {"l", "lipo", lipoargs})
        end
        batchcmds:cp("LICENSE", package:installdir())
    end)
