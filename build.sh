#!/usr/bin/env bash

# Script that downloads xmake if it's missing and builds the project
# Parameters
# --with-logging : enable logging
# --arch=<arch> : comma-separated list of architectures to build for
# ... : additional parameters passed to xmake

# Parse parameters into variables
WITH_LOGGING="n"
# Bash list of architectures to build for
ARCHS=("x86" "x64")
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --with-logging)
            WITH_LOGGING="y"
            shift
            ;;
        --arch=*)
            # Split the parameter into an array, remove commas
            IFS=',' read -r -a ARCHS <<< "${key#*=}"
            shift
            ;;
        *)
            break
            ;;
    esac
done

XMAKE_VERSION="2.6.1"

# Get current dir
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

TOOLS_DIR="$CURRENT_DIR/tools"
XMAKE_DIR="$TOOLS_DIR/xmake"
XMAKE_BUILD_DIR="$TOOLS_DIR/xmake_build"
xmake="$XMAKE_DIR/bin/xmake"

if [ ! -d "$TOOLS_DIR" ]; then
    echo "Creating tools dir..."
    mkdir -p "$TOOLS_DIR"
fi

# get make
if gmake --version >/dev/null 2>&1
then
    make=gmake
else
    make=make
fi

test_tools()
{
    prog='#include <stdio.h>\nint main(){return 0;}'
    {
        git --version &&
        $make --version &&
        {
            echo -e "$prog" | cc -xc - -o /dev/null ||
            echo -e "$prog" | gcc -xc - -o /dev/null ||
            echo -e "$prog" | clang -xc - -o /dev/null ||
            echo -e "$prog" | cc -xc -c - -o /dev/null -I/usr/include -I/usr/local/include ||
            echo -e "$prog" | gcc -xc -c - -o /dev/null -I/usr/include -I/usr/local/include ||
            echo -e "$prog" | clang -xc -c - -o /dev/null -I/usr/include -I/usr/local/include
        }
    } >/dev/null 2>&1
}

install_tools()
{
    { apt --version >/dev/null 2>&1 && $sudoprefix apt install -y git build-essential libreadline-dev ccache gcc-multilib g++-multilib; } ||
    { yum --version >/dev/null 2>&1 && $sudoprefix yum install -y git readline-devel ccache bzip2 && $sudoprefix yum groupinstall -y 'Development Tools'; } ||
    { zypper --version >/dev/null 2>&1 && $sudoprefix zypper --non-interactive install git readline-devel ccache && $sudoprefix zypper --non-interactive install -t pattern devel_C_C++; } ||
    { pacman -V >/dev/null 2>&1 && $sudoprefix pacman -S --noconfirm --needed git base-devel ccache; } ||
    { emerge -V >/dev/null 2>&1 && $sudoprefix emerge -atv dev-vcs/git ccache; } ||
    { pkg list-installed >/dev/null 2>&1 && $sudoprefix pkg install -y git getconf build-essential readline ccache; } || # termux
    { pkg help >/dev/null 2>&1 && $sudoprefix pkg install -y git readline ccache ncurses; } || # freebsd
    { nix-env --version >/dev/null 2>&1 && nix-env -i git gcc readline ncurses; } || # nixos
    { apk --version >/dev/null 2>&1 && $sudoprefix apk add git gcc g++ make readline-dev ncurses-dev libc-dev linux-headers; } ||
    { xbps-install --version >/dev/null 2>&1 && $sudoprefix xbps-install -Sy git base-devel ccache; } #void
}

if [ ! -d "$XMAKE_DIR" ]; then
    echo "Checking for build tools..."
    test_tools || { install_tools && test_tools; } || my_exit "$(echo -e 'Dependencies Installation Fail\nThe getter currently only support these package managers\n\t* apt\n\t* yum\n\t* zypper\n\t* pacman\n\t* portage\n\t* xbps\n Please install following dependencies manually:\n\t* git\n\t* build essential like `make`, `gcc`, etc\n\t* libreadline-dev (readline-devel)\n\t* ccache (optional)')" 1

    echo "Downloading and building xmake, this might take a while..."
    if xz --version >/dev/null 2>&1
    then
        pack=xz
    else
        pack=gz
    fi

    curl -fSL "https://github.com/xmake-io/xmake/releases/download/v$XMAKE_VERSION/xmake-v$XMAKE_VERSION.$pack.run" > "$TOOLS_DIR/xmake.run"
    sh "$TOOLS_DIR/xmake.run" --noexec --target "$XMAKE_BUILD_DIR"

    cd "$XMAKE_BUILD_DIR" && $make build && DESTDIR="$TOOLS_DIR" PREFIX="xmake" $make install
fi

# Build projects for each arch
for arch in "${ARCHS[@]}"
do
    echo "Building for $arch..."
    $xmake f -a $arch --include_logging=$WITH_LOGGING
    $xmake $@
done