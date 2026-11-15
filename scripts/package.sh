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
# Documentatie als PDF: opent op elke Mac (Voorvertoning), ook zonder Markdown-app
python3 scripts/md2pdf.py docs/INSTALL.md "$OUT/LEESMIJ - Installeren.pdf"
python3 scripts/md2pdf.py docs/MANUAL.md "$OUT/RoughStart Manual.pdf"
cp LICENSE "$OUT/LICENSE.txt"
cp Resources/fonts/Inter-LICENSE.txt "$OUT/Inter-LICENSE.txt"

# ditto behoudt de bundelstructuur van de .component
ditto -c -k --norsrc --keepParent "$OUT" "dist/$NAME.zip"
rm -rf "$OUT"

echo "Klaar: dist/$NAME.zip ($(du -h "dist/$NAME.zip" | cut -f1))"
