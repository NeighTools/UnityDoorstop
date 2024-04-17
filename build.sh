#!/usr/bin/env bash

# Script that downloads xmake if it's missing and builds the project
# Parameters
# -with_logging : enable logging
# -arch=<arch> : comma-separated list of architectures to build for
# ... : additional parameters passed to xmake


# Bomb out if anything goes wrong
set -e

CBLACK="\033[0;30m"
CRED="\033[0;31m"
CGREEN="\033[0;32m"
CYELLOW="\033[0;33m"
CBLUE="\033[0;34m"
CPINK="\033[0;35m"
CCYAN="\033[0;36m"
CWHITE="\033[0;37m"
CDEFAULT="\033[0m"

PBLACK="\001\033[0;30m\002"
PRED="\001\033[0;31m\002"
PGREEN="\001\033[0;32m\002"
PYELLOW="\001\033[0;33m\002"
PBLUE="\001\033[0;34m\002"
PPINK="\001\033[0;35m\002"
PCYAN="\001\033[0;36m\002"
PWHITE="\001\033[0;37m\002"
PDEFAULT="\001\033[0m\002"

function echo-err { echo "$@" 1>&2; }

function ask { echo -ne "${CYELLOW} $1: ${CDEFAULT}"; read ASK; export ASK; }
function ask-default { echo -ne "${CYELLOW} $1 [$2]: ${CDEFAULT}"; read ASK; export ASK=${ASK:-$2}; }
function ask-yes { echo -ne "${CYELLOW} $1 [Y/n]: ${CDEFAULT}"; read ASK; ASK="$(echo $ASK | tr '[:upper:]' '[:lower:]' | head -c 1)"; export ASK=${ASK:-y}; }
function ask-no { echo -ne "${CYELLOW} $1 [y/N]: ${CDEFAULT}"; read ASK; ASK="$(echo $ASK | tr '[:upper:]' '[:lower:]' | head -c 1)"; export ASK=${ASK:-n}; }
function ask-enter { echo -e "${CYELLOW} $1: [Press enter to continue]${CDEFAULT}" ; read; }
function ask-password { echo -ne "${CYELLOW} $1: ${CDEFAULT}" ; read -s ASK; echo; }

function msg-error { echo -e "${CRED}${@}${CDEFAULT}"; }
function msg-info { echo -e "${CCYAN}${@}${CDEFAULT}"; }
function msg-success { echo -e "${CGREEN}${@}${CDEFAULT}"; }
function msg-dry { echo -e "${CPINK}${@}${CDEFAULT}"; }

function date-today { echo $(date +%F); }
function date-second { echo $(date +%F_%H-%M-%S); }
function date-8601 { date -u +%Y-%m-%dT%H:%M:%S%z; }
function date-8601-local { date +%Y-%m-%dT%H:%M:%S%z; }
function log-8601 { msg-info "[ $(date-8601) ] $@"; }
function log-8601-local { msg-info "[ $(date-8601-local) ] $@"; }
function log-err-8601 { echo-err "[ $(date-8601) ] $@"; }
function log-err-8601-local { echo-err "[ $(date-8601-local) ] $@"; }

commands_exist () {
  while [[ "$1" != "" ]]; do
    if ! command -v "$1" &> /dev/null; then
      return 1
    else
      shift
    fi
  done
}

help () {
  echo "
    ./build.sh [-with_logging] [-arch=<arch>] [--help|-h] [...]
    Script that downloads xmake if it's missing and builds the project
      Parameters
        -with_logging : enable logging
        -arch=<arch> : comma-separated list of architectures to build for
        -h, --help: show this help message
        ... : additional parameters passed to xmake
  "
}

# Parse parameters into variables
WITH_LOGGING="n"
PROFILE="release"
# Bash list of architectures to build for
ARCHS=("x86" "x64")
if [[ "$(uname)" == "Darwin" ]]; then
    ARCHS=("x86_64")
fi
while [[ $# > 0 ]]; do
    key="$1"
    case $key in
        -with_logging)
            WITH_LOGGING="y"
            shift
            ;;
        -debug)
            PROFILE="debug"
            shift
            ;;
        -arch=*)
            # Split the parameter into an array, remove commas
            IFS=',' read -r -a ARCHS <<< "${key#*=}"
            shift
            ;;
        *)
            break
            ;;
    esac
done

XMAKE_VERSION="2.8.9"

# Get current dir
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$CURRENT_DIR"

TOOLS_DIR="$CURRENT_DIR/tools"
XMAKE_DIR="$TOOLS_DIR/xmake"
XMAKE_BUILD_DIR="$TOOLS_DIR/xmake_build"
xmake="$XMAKE_DIR/bin/xmake"

if [[ ! -d "$TOOLS_DIR" ]]; then
    log-8601-local "Creating tools dir..."
    mkdir -p "$TOOLS_DIR"
fi

# get make
if commands_exist gmake; then
    if gmake --version >/dev/null 2>&1; then
        make=gmake
    fi
elif commands_exist make; then
    make=make
else
    msg-error "No make command found, install gmake or make to continue"
    exit 1
fi

test_tools()
{
    if commands_exist cc gcc clang; then
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
    else
        msg-error "Missing tools:"
        for TOOL in cc gcc clang; do
          commands_exist "$TOOL" || msg-error "- $TOOL"
        done
    fi
}

install_tools()
{
    log-8601-local "Installing tools..."
    if commands_exist sudo; then
        sudoprefix=sudo
    else
        msg-error "No valid sudo command found, install a sudo command to continue"
        exit 1
    fi
    if commands_exist apt; then
      { apt --version >/dev/null 2>&1 && $sudoprefix apt install -y git build-essential curl libreadline-dev ccache gcc-multilib g++-multilib; }
    elif commands_exist yum; then
      { yum --version >/dev/null 2>&1 && $sudoprefix yum install -y git readline-devel ccache bzip2 && $sudoprefix yum groupinstall -y 'Development Tools'; }
    elif commands_exist zypper; then
      { zypper --version >/dev/null 2>&1 && $sudoprefix zypper --non-interactive install git readline-devel ccache && $sudoprefix zypper --non-interactive install -t pattern devel_C_C++; }
    elif commands_exist pacman; then
      { pacman -V >/dev/null 2>&1 && $sudoprefix pacman -S --noconfirm --needed git base-devel ccache; }
    elif commands_exist emergy; then
      { emerge -V >/dev/null 2>&1 && $sudoprefix emerge -atv dev-vcs/git ccache; }
    elif commands_exist pkg; then
      { pkg list-installed >/dev/null 2>&1 && $sudoprefix pkg install -y git getconf build-essential readline ccache; } || # termux
      { pkg help >/dev/null 2>&1 && $sudoprefix pkg install -y git readline ccache ncurses; } # freebsd
    elif commands_exist nix-env; then
      { nix-env --version >/dev/null 2>&1 && nix-env -i git gcc readline ncurses; } # nixos
    elif commands_exist apk; then
      { apk --version >/dev/null 2>&1 && $sudoprefix apk add git gcc g++ make readline-dev ncurses-dev libc-dev linux-headers; }
    elif commands_exist xbps-install; then
      { xbps-install --version >/dev/null 2>&1 && $sudoprefix xbps-install -Sy git base-devel ccache; } #void
    elif commands_exist brew; then
      { brew install gmake readline xz; } # MacOS
    else
      msg-error "No supported package manager found, please install cc, gcc, and clang to continue"
      return 1
    fi
    msg-success "Tools installed successfully"
}

INSTALL_FAILED_MSG="
    Dependencies Installation Fail
    The getter currently only support these package managers
        * apt
        * yum
        * zypper
        * pacman
        * portage
        * xbps
    Please install following dependencies manually:
        * curl
        * gzip or xz
        * git
        * make
        * cc
        * gcc
        * clang
        * libreadline-dev (readline-devel)
        * ccache (optional)
"

if [[ ! -d "$XMAKE_DIR" ]] || [[ ! -x "$xmake" ]]; then
    log-8601-local "Checking for build tools..."
    if test_tools; then
        msg-success "Tools found, and are working properly"
    else
        log-8601-local "Installing tools"
        if install_tools && test_tools; then
            msg-success "Tools found, and are working properly"
        else
            msg-error "$INSTALL_FAILED_MSG"
        fi
    fi

    log-8601-local "Downloading and building xmake, this might take a while..."
    if commands_exist gzip; then
        pack=gz
    else
        msg-error "No decompression binary found, please install gzip to continue"
    fi

    log-8601-local "Downloading xmake runner..."
    curl -fSL "https://github.com/xmake-io/xmake/releases/download/v$XMAKE_VERSION/xmake-v$XMAKE_VERSION.$pack.run" > "$TOOLS_DIR/xmake.run"
    log-8601-local "Downloading xmake maybe..."
    sh "$TOOLS_DIR/xmake.run" --noexec --target "$XMAKE_BUILD_DIR"
    log-8601-local "Buinding and installing xmake..."
    if (cd "$XMAKE_BUILD_DIR" && ./configure && DESTDIR="$TOOLS_DIR" PREFIX="xmake" $make install); then
        msg-success "xmake installed successfully"
    else
        msg-error "Failed to install xmake"
        exit 1
    fi
fi

# Build projects for each arch
for arch in "${ARCHS[@]}"
do
    log-8601-local "Building for $arch..."
    "$xmake" f -a $arch -m $PROFILE --include_logging=$WITH_LOGGING
    "$xmake" "$@"
done
