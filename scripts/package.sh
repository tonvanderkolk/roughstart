#!/bin/zsh
# Maakt het Mac-pakket: dist/RoughStart-<versie>-macos.zip met de plugin (AU en VST3),
# het installatiebriefje en de handleiding als PDF en de licenties.
# De PDF's komen ook los in dist/docs/, voor het Windows-pakket in GitHub Actions.
set -euo pipefail

cd "$(dirname "$0")/.."
VERSION=$(tr -d '[:space:]' < VERSION)
NAME="RoughStart-$VERSION-macos"
OUT="dist/$NAME"

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64" > /dev/null
cmake --build build --target RoughStart_AU RoughStart_VST3

COMPONENT="build/RoughStart_artefacts/Release/AU/RoughStart.component"
echo "Architecturen: $(lipo -archs "$COMPONENT/Contents/MacOS/RoughStart")"

rm -rf "$OUT" "dist/$NAME.zip"
mkdir -p "$OUT"
cp -R "$COMPONENT" "$OUT/"
cp -R "build/RoughStart_artefacts/Release/VST3/RoughStart.vst3" "$OUT/"
# Documentatie als PDF: opent op elke Mac (Voorvertoning), ook zonder Markdown-app
python3 scripts/md2pdf.py docs/INSTALL.md "$OUT/LEESMIJ - Installeren.pdf"
python3 scripts/md2pdf.py docs/MANUAL.md "$OUT/RoughStart Manual.pdf"
cp LICENSE "$OUT/LICENSE.txt"
cp Resources/fonts/Inter-LICENSE.txt "$OUT/Inter-LICENSE.txt"

rm -rf dist/docs && mkdir -p dist/docs
cp "$OUT/LEESMIJ - Installeren.pdf" "$OUT/RoughStart Manual.pdf" "$OUT/LICENSE.txt" "$OUT/Inter-LICENSE.txt" dist/docs/

# ditto behoudt de bundelstructuur van de .component
ditto -c -k --norsrc --keepParent "$OUT" "dist/$NAME.zip"
rm -rf "$OUT"

echo "Klaar: dist/$NAME.zip ($(du -h "dist/$NAME.zip" | cut -f1))"
