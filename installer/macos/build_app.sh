#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
#  cFd macOS .app bundle builder
#  Run from the repository root:  bash installer/macos/build_app.sh
#
#  Requirements: gcc, make (Xcode CLT)
#    xcode-select --install
#
#  Output: build/cFd.app
#          build/cFd-1.0.0-macos.tar.gz   (archive for distribution)
#          build/cFd-1.0.0-macos.dmg      (disk image, requires hdiutil)
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
VERSION="1.0.0"
APP_NAME="cFd"
BUNDLE="$REPO_ROOT/build/$APP_NAME.app"
BINARY="$REPO_ROOT/cfd"

# ── 1. Build the binary ──────────────────────────────────────────────────────
echo "→ Building cFd..."
cd "$REPO_ROOT"
make clean
make
echo "✓ Build complete: $BINARY"

# ── 2. Create .app bundle structure ──────────────────────────────────────────
echo "→ Creating .app bundle..."
rm -rf "$BUNDLE"
mkdir -p "$BUNDLE/Contents/MacOS"
mkdir -p "$BUNDLE/Contents/Resources"

# Copy binary
cp "$BINARY" "$BUNDLE/Contents/MacOS/$APP_NAME"
chmod +x "$BUNDLE/Contents/MacOS/$APP_NAME"

# Info.plist
cp "$REPO_ROOT/installer/macos/Info.plist" "$BUNDLE/Contents/Info.plist"

# Icon (generate a simple one if iconutil / png2icns is available)
if command -v iconutil &>/dev/null 2>&1; then
    ICONSET="$REPO_ROOT/build/cFd.iconset"
    mkdir -p "$ICONSET"
    # Create simple colored square icon using sips if available
    if command -v python3 &>/dev/null; then
        python3 "$REPO_ROOT/installer/macos/gen_icon.py" "$ICONSET" 2>/dev/null || true
    fi
    if ls "$ICONSET"/*.png &>/dev/null 2>&1; then
        iconutil -c icns "$ICONSET" -o "$BUNDLE/Contents/Resources/$APP_NAME.icns" 2>/dev/null || true
    fi
    rm -rf "$ICONSET"
fi

echo "✓ Bundle created: $BUNDLE"

# ── 3. Code sign (ad-hoc, no Apple Developer account needed) ─────────────────
if command -v codesign &>/dev/null; then
    echo "→ Signing (ad-hoc)..."
    codesign --force --deep --sign - "$BUNDLE" 2>/dev/null && echo "✓ Signed (ad-hoc)" || echo "⚠ Sign skipped"
fi

# ── 4. Create distributable archive ──────────────────────────────────────────
mkdir -p "$REPO_ROOT/build"
ARCHIVE="$REPO_ROOT/build/${APP_NAME}-${VERSION}-macos.tar.gz"
echo "→ Creating archive: $ARCHIVE"
cd "$REPO_ROOT/build"
tar -czf "$ARCHIVE" "$APP_NAME.app"
echo "✓ Archive: $ARCHIVE"

# ── 5. Create DMG (drag-and-drop installer) ───────────────────────────────────
DMG="$REPO_ROOT/build/${APP_NAME}-${VERSION}-macos.dmg"
if command -v hdiutil &>/dev/null; then
    echo "→ Creating DMG..."
    STAGING="$REPO_ROOT/build/dmg_staging"
    rm -rf "$STAGING"
    mkdir -p "$STAGING"
    cp -R "$BUNDLE" "$STAGING/"
    # Symlink to /Applications for drag-to-install
    ln -s /Applications "$STAGING/Applications"
    hdiutil create \
        -volname "cFd Terminal $VERSION" \
        -srcfolder "$STAGING" \
        -ov -format UDZO \
        "$DMG" >/dev/null
    rm -rf "$STAGING"
    echo "✓ DMG: $DMG"
else
    echo "⚠ hdiutil not found — skipping DMG creation"
fi

# ── Summary ──────────────────────────────────────────────────────────────────
echo ""
echo "┌─────────────────────────────────────────────────────┐"
echo "│  cFd macOS build complete                           │"
echo "├─────────────────────────────────────────────────────┤"
printf "│  App bundle : %-38s│\n" "$APP_NAME.app"
printf "│  Archive    : %-38s│\n" "${APP_NAME}-${VERSION}-macos.tar.gz"
if [ -f "$DMG" ]; then
printf "│  DMG        : %-38s│\n" "${APP_NAME}-${VERSION}-macos.dmg"
fi
echo "└─────────────────────────────────────────────────────┘"
echo ""
echo "Install: drag cFd.app to /Applications"
echo "Or run:  open build/cFd.app"
