#!/bin/bash

# Пути
PLUGIN_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$HOME/far2l-dev/src/build"
INSTALL_PLUGIN_DIR="$HOME/far2l-dev/install/share/far2l/plugins"
FAR2L_BIN="$HOME/far2l-dev/install/bin/far2l"

echo "=== Building plugin ==="
cd "$BUILD_DIR" || exit 1
make alias_cd

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "=== Copying plugin ==="
mkdir -p "$INSTALL_PLUGIN_DIR"
cp "$BUILD_DIR/plugins/alias_cd/alias_cd.far-plug-wide" "$INSTALL_PLUGIN_DIR/"

echo "=== Running far2l ==="
"$FAR2L_BIN" --tty