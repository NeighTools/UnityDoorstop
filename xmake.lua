add_rules("mode.debug", "mode.release")

target("doorstop")
    set_kind("shared")
    add_files("src/*.c")