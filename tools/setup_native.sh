#!/bin/sh
# Install the optional EDS analysis tools natively (WSL or Linux): a Python venv with capstone and
# pyelftools, decomp-permuter, m2c and objdiff-cli. The build itself needs none of these; it only needs
# make, cpp, python3, binutils-arm-none-eabi and agbcc in tools/agbcc (from ../agbcc, see README.md).
# Never needs or touches a ROM.
#
#   tools/setup_native.sh [PREFIX]        # default PREFIX: $HOME/.eds-tools
#                                          # tools/dr sources $PREFIX/env.sh automatically
#
# On Debian/Ubuntu the script installs missing system packages with apt-get (via sudo when not root).
# The permuter, objdiff-cli and m2c are skipped when EDS_MINIMAL=1.
set -eu
PREFIX="${1:-$HOME/.eds-tools}"
mkdir -p "$PREFIX"

need_pkgs=""
command -v arm-none-eabi-as >/dev/null 2>&1 || need_pkgs="$need_pkgs binutils-arm-none-eabi"
command -v gcc >/dev/null 2>&1 || need_pkgs="$need_pkgs build-essential"
command -v git >/dev/null 2>&1 || need_pkgs="$need_pkgs git ca-certificates"
command -v make >/dev/null 2>&1 || need_pkgs="$need_pkgs make"
python3 -c 'import venv' >/dev/null 2>&1 || need_pkgs="$need_pkgs python3 python3-venv python3-pip"
[ -e /usr/include/png.h ] || need_pkgs="$need_pkgs libpng-dev pkg-config"
if [ -n "$need_pkgs" ]; then
    if command -v apt-get >/dev/null 2>&1; then
        SUDO=""; [ "$(id -u)" = 0 ] || SUDO="sudo"
        $SUDO apt-get update && $SUDO apt-get install -y --no-install-recommends $need_pkgs
    else
        echo "please install:$need_pkgs" >&2; exit 1
    fi
fi

# Python tools.
[ -x "$PREFIX/venv/bin/python3" ] || python3 -m venv "$PREFIX/venv"
"$PREFIX/venv/bin/pip" install -q capstone pyelftools

if [ "${EDS_MINIMAL:-0}" != "1" ]; then
    [ -d "$PREFIX/permuter" ] || git clone --depth 1 https://github.com/simonlindholm/decomp-permuter "$PREFIX/permuter"
    "$PREFIX/venv/bin/pip" install -q pycparser toml Levenshtein
    [ -d "$PREFIX/m2c" ] || git clone --depth 1 https://github.com/matt-kempster/m2c "$PREFIX/m2c"
    "$PREFIX/venv/bin/pip" install -q graphviz
    if [ ! -x "$PREFIX/bin/objdiff-cli" ]; then
        mkdir -p "$PREFIX/bin"
        case "$(uname -m)" in aarch64|arm64) a=linux-aarch64;; *) a=linux-x86_64;; esac
        "$PREFIX/venv/bin/python3" -c "import urllib.request,sys; urllib.request.urlretrieve(sys.argv[1], sys.argv[2])" \
            "https://github.com/encounter/objdiff/releases/download/v3.8.2/objdiff-cli-$a" "$PREFIX/bin/objdiff-cli"
        chmod +x "$PREFIX/bin/objdiff-cli"
    fi
fi

cat > "$PREFIX/env.sh" <<ENV
# Optional EDS analysis tools; tools/dr sources this file.
export M2C_DIR="$PREFIX/m2c"
export PERMUTER_DIR="$PREFIX/permuter"
export PATH="$PREFIX/venv/bin:$PREFIX/bin:\$PATH"
ENV
echo "Extras ready in $PREFIX; tools/dr picks them up (or: . \"$PREFIX/env.sh\")."
