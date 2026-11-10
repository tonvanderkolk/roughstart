#!/bin/zsh
# Maakt een testpakket: dist/RoughStart-<versie>.zip met de plugin,
# het installatiebriefje en de handleiding.
set -euo pipefail

cd "$(dirname "$0")/.."
VERSION=$(tr -d '[:space:]' < VERSION)
NAME="RoughStart-$VERSION"
OUT="dist/$NAME"

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64" > /dev/null
cmake --build build --target RoughStart_AU

COMPONENT="build/RoughStart_artefacts/Release/AU/RoughStart.component"
echo "Architecturen: $(lipo -archs "$COMPONENT/Contents/MacOS/RoughStart")"

rm -rf "$OUT" "dist/$NAME.zip"
mkdir -p "$OUT"
cp -R "$COMPONENT" "$OUT/"
cp docs/INSTALL.md "$OUT/LEESMIJ - Installeren.md"
cp docs/MANUAL.md "$OUT/Manual.md"

# ditto behoudt de bundelstructuur van de .component
ditto -c -k --norsrc --keepParent "$OUT" "dist/$NAME.zip"
rm -rf "$OUT"

echo "Klaar: dist/$NAME.zip ($(du -h "dist/$NAME.zip" | cut -f1))"
