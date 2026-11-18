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

# Ondertekenen en notariseren, als er een Developer ID op deze Mac staat.
# Zonder (bijvoorbeeld in GitHub Actions) blijft het pakket ongesigneerd.
# Notariseren met een App Store Connect API-sleutel (NOTARY_KEY, NOTARY_KEY_ID, NOTARY_ISSUER;
# zo doet GitHub Actions het), of anders met het sleutelhangerprofiel NOTARY_PROFILE
# (standaard "roughstart", aangemaakt met: xcrun notarytool store-credentials roughstart ...).
# Zie ook: TvdK Apps/02 Naslag/apple-ondertekenen.md
SIGN_IDENTITY="${SIGN_IDENTITY:-$(security find-identity -v -p codesigning 2>/dev/null | sed -n 's/.*"\(Developer ID Application:[^"]*\)".*/\1/p' | head -1)}"
NOTARY_PROFILE="${NOTARY_PROFILE:-roughstart}"
if [[ -n "${NOTARY_KEY:-}" ]]; then
    NOTARY_AUTH=(--key "$NOTARY_KEY" --key-id "$NOTARY_KEY_ID" --issuer "$NOTARY_ISSUER")
else
    NOTARY_AUTH=(--keychain-profile "$NOTARY_PROFILE")
fi
if [[ -n "$SIGN_IDENTITY" ]]; then
    echo "Ondertekenen met: $SIGN_IDENTITY"
    for bundle in "$OUT/RoughStart.component" "$OUT/RoughStart.vst3"; do
        codesign --force --deep --options runtime --timestamp --sign "$SIGN_IDENTITY" "$bundle"
        codesign --verify --strict --verbose=1 "$bundle"
    done

    echo "Notariseren bij Apple (duurt meestal een paar minuten)..."
    NOTARY_ZIP="dist/notarize-$VERSION.zip"
    ditto -c -k --keepParent "$OUT/RoughStart.component" "$NOTARY_ZIP"
    ditto -c -k --keepParent "$OUT/RoughStart.vst3" "dist/notarize-vst3.zip"
    for z in "$NOTARY_ZIP" "dist/notarize-vst3.zip"; do
        xcrun notarytool submit "$z" "${NOTARY_AUTH[@]}" --wait | tee dist/notary.log
        grep -q "status: Accepted" dist/notary.log || { echo "Notariseren mislukt, zie dist/notary.log"; exit 1; }
    done
    rm -f "$NOTARY_ZIP" "dist/notarize-vst3.zip" dist/notary.log

    xcrun stapler staple "$OUT/RoughStart.component"
    xcrun stapler staple "$OUT/RoughStart.vst3"
    SIGNED=1
else
    echo "Geen Developer ID gevonden: pakket wordt NIET ondertekend."
    SIGNED=0
fi
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

echo "Ondertekend en genotariseerd: $([[ $SIGNED == 1 ]] && echo ja || echo nee)"
echo "Klaar: dist/$NAME.zip ($(du -h "dist/$NAME.zip" | cut -f1))"
