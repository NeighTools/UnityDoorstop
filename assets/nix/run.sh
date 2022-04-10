#!/bin/sh
# Doorstop start script
# 
# Run the script to start the game with Doorstop enabled
#
# There are two ways to use this script
#
# 1. Via CLI: Run ./run.sh <path to game> [doorstop arguments] [game arguments]
# 2. Via config: edit the options below and run ./run.sh without any arguments

# 0 is false, 1 is true

# LINUX: name of Unity executable
# MACOS: name of the .app directory
executable_name=""

# All of the below can be overriden with command line args

# General Config Options

enabled="1"

target_assembly=""

ignore_disable_switch="0"

# Mono Options

search_path_override=""

debug_enable="0"

debug_start_server="0"

debug_address="127.0.0.1:10000"

debug_suspend="0"

# CoreCLR options (IL2CPP)

runtime_path=""

corelib_path=""


# Everything past this point is the actual script

# Special case: program is launched via Steam
# In that case rerun the script via their bootstrapper to ensure Steam overlay works
if [ "$2" = "SteamLaunch" ]; then
    steam="$1 $2 $3 $4 $0 $5"
    shift 5
    $steam $@
    exit
fi

# Handle first param being executable name
if [ -x "$1" ] ; then
    executable_name="$1"
    echo "$1"
    shift
fi

if [ -z "${executable_name}" -o ! -x "${executable_name}" ]; then
    echo "Please set executable_name to a valid name in a text editor or as the first command line parameter"
    exit 1
fi

# This is just copied from the BepInEx run script, it *somehow* gets the base directory
a="/$0"; a=${a%/*}; a=${a#/}; a=${a:-.}; BASEDIR=$(cd "$a"; pwd -P)

arch=""
executable_path=""
lib_extension=""

# Set executable path and the extension to use for the libdoorstop shared object
os_type="$(uname -s)"
case ${os_type} in
    Linux*)
        executable_path="${executable_name}"
        # Handle relative paths
        if [ executable_path != /* ] ; then
            executable_path="${BASEDIR}/${executable_path}"
        fi
        lib_extension="so"
    ;;
    Darwin*)
        # macOS man, what are they doing over there
        real_executable_name="${executable_name}"

        # Handle relative directories
        if [ real_executable_name != /* ] ; then
            real_executable_name="${BASEDIR}/${real_executable_name}"
        fi

        # If we're not even an actual executable, check .app Info for actual executable
        if [ "${real_executable_name}" != *.app/Contents/MacOS/* ] ; then
            # Add .app to the end if not given
            if [ "${real_executable_name}" != *.app ] ; then
                real_executable_name="${real_executable_name}.app"
            fi
            inner_executable_name=$(defaults read "${real_executable_name}/Contents/Info" CFBundleExecutable)
            executable_path="${real_executable_name}/Contents/MacOS/${inner_executable_name}"
        else
            executable_path="${executable_name}"
        fi
        lib_extension="dylib"
    ;;
    *)
        # alright whos running games on freebsd
        echo "Unknown operating system ($(uname -s))"
        echo "Make an issue at https://github.com/NeighTools/UnityDoorstop"
        exit 1
    ;;
esac

abs_path() {
    echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
}

_readlink() {
    # relative links with readlink (without -f) do not preserve the path info 
    ab_path="$(abs_path "$1")"
    link="$(readlink "${ab_path}")"
    case $link in
        /*);;
        *) link="$(dirname "$ab_path")/$link";;
    esac
    echo "$link"
}


resolve_executable_path () {
    e_path="$(abs_path "$1")"
    
    while [ -L "${e_path}" ]; do 
        e_path=$(_readlink "${e_path}");
    done
    echo "${e_path}"
}

# Get absolute path of executable and show to user
executable_path=$(resolve_executable_path "${executable_path}")
echo "${executable_path}"

# Figure out the arch of the executable with file
file_out="$(LD_PRELOAD="" file -b ${executable_path})"
case "${file_out}" in
    *64-bit*)
        arch="x64"
    ;;
    *32-bit*)
        arch="x86"
    ;;
    *)
        echo "The executable \"${executable_path}\" is not compiled for x86 or x64 (might be ARM?)"
        echo "If you think this is a mistake (or would like to encourage support for other architectures)"
        echo "Please make an issue at https://github.com/NeighTools/UnityDoorstop"
        echo "Got: ${file_out}"
        exit 1
    ;;
esac

# Helper to convert common boolean strings into just 0 and 1
doorstop_bool() {
    case "$1" in
        TRUE|true|t|T|1|Y|y|yes)
            echo "1"
        ;;
        FALSE|false|f|F|0|N|n|no)
            echo "0"
        ;;
    esac
}

# Read from command line
executable_final="${executable_path}"
while [ "$#" != "0" ]; do
    case "$1" in
        --doorstop_enabled)
            shift
            enabled="$(doorstop_bool $1)"
        ;;
        --doorstop_target_assembly)
            shift
            target_assembly="$1"
        ;;
        --doorstop-mono-dll-search-path-override)
            shift
            search_path_override="$1"
        ;;
        --doorstop-mono-debug-enabled)
            shift
            debug_enable="$(doorstop_bool $1)"
        ;;
        --doorstop-mono-debug-start-server)
            shift
            debug_start_server="$(doorstop_bool $1)"
        ;;
        --doorstop-mono-debug-suspend)
            shift
            debug_suspend="$(doorstop_bool $1)"
        ;;
        --doorstop-mono-debug-address)
            shift
            debug_address="$1"
        ;;
        --doorstop-clr-runtime-coreclr-path)
            shift
            runtime_path="$1"
        ;;
        --doorstop-clr-corlib-dir)
            shift
            corelib_path="$1"
        ;;
        *)
            executable_final="${executable_final} \"$1\""
        ;;
    esac
    shift
done

# Move variables to environment
export DOORSTOP_ENABLED="${enabled}"
export DOORSTOP_TARGET_ASSEMBLY="${target_assembly}"
export DOORSTOP_IGNORE_DISABLED_ENV="${ignore_disable_switch}"
if [ -n "${search_path_override}" ] ; then
    export DOORSTOP_MONO_DLL_SEARCH_PATH_OVERRIDE="${search_path_override}"
fi
export DOORSTOP_MONO_DEBUG_ENABLED="${debug_enable}"
export DOORSTOP_MONO_DEBUG_START_SERVER="${debug_start_server}"
export DOORSTOP_MONO_DEBUG_ADDRESS="${debug_address}"
export DOORSTOP_MONO_DEBUG_SUSPEND="${debug_suspend}"
if [ -n "${runtime_path}" ] ; then
    export DOORSTOP_CLR_RUNTIME_CORECLR_PATH="${runtime_path}"
fi
if [ -n "${corelib_path}" ] ; then
    export DOORSTOP_CLR_CORLIB_DIR="${DOORSTOP_CLR_CORLIB_DIR}"
fi

# Final setup
doorstop_directory="${BASEDIR}/doorstop_libs/"
doorstop_name="libdoorstop_${arch}.${lib_extension}"

export LD_LIBRARY_PATH="${doorstop_directory}:${LD_LIBRARY_PATH}"
export LD_PRELOAD="${doorstop_name}:${LD_PRELOAD}"
export DYLD_LIBRARY_PATH="${doorstop_directory}:${DYLD_LIBRARY_PATH}"
export DYLD_INSERT_LIBRARIES="${doorstop_name}:${DYLD_INSERT_LIBRARIES}"

${executable_final}
