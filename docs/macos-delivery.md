# macOS delivery

The repository's `scripts/package-macos.sh` creates an arm64 Release
`Chess.app`, runs both Meson tests, recursively bundles non-system Mach-O
dependencies, removes Homebrew/build-tree load paths, applies an ad-hoc
signature, verifies it, and creates a ZIP. Pass `--dmg` to also create a
compressed DMG with an Applications shortcut.

The default outputs are under `dist/macos/`, which is ignored by Git. The
script only accepts an output directory inside this repository and only
replaces its exact `Chess.app` and versioned Chess archive paths. It exits
nonzero on a missing tool, build/test failure, non-arm64 input, missing
dependency, invalid signature, or non-relocatable Mach-O reference. It rejects
non-macOS hosts before configuring a build, so Linux builds and both existing
CI workflows are unaffected.

```sh
scripts/package-macos.sh --dmg
```

The generated icon is derived solely from the tracked
`gui/resources/icons/app-icon.png` project asset. This avoids introducing an
untracked third-party icon. The 128-pixel source limits large-icon sharpness
and can be replaced later by a project-owned high-resolution master.

## Developer ID and notarization

The default bundle uses an ad-hoc signature (`-`) for local validation. It is
not Developer ID signed, notarized, or Gatekeeper-approved for distribution.
After obtaining an appropriate certificate and choosing its identity locally,
an authorized release operator can sign the already assembled bundle with:

```sh
SIGNING_IDENTITY="Developer ID Application: ORGANIZATION (TEAMID)"
find dist/macos/Chess.app/Contents/Frameworks -type f -name '*.dylib' \
  -print0 | while IFS= read -r -d '' library; do
    codesign --force --options runtime --timestamp \
      --sign "$SIGNING_IDENTITY" "$library"
  done
codesign --force --options runtime --timestamp \
  --sign "$SIGNING_IDENTITY" dist/macos/Chess.app
codesign --verify --deep --strict --verbose=2 dist/macos/Chess.app
ditto -c -k --sequesterRsrc --keepParent \
  dist/macos/Chess.app dist/macos/Chess-0.1.0-macOS-arm64.zip
xcrun notarytool submit dist/macos/Chess-0.1.0-macOS-arm64.zip \
  --keychain-profile "CHESS-NOTARY" --wait
xcrun stapler staple dist/macos/Chess.app
xcrun stapler validate dist/macos/Chess.app
spctl --assess --type execute --verbose=4 dist/macos/Chess.app
```

No entitlements are currently required by the application. If future features
need them, review a minimal property list before signing rather than adding
broad exceptions. Store notarization credentials in a local Keychain profile;
never put certificate passwords, API keys, or profiles in this repository or
command output. Apple submission and identity signing are intentionally not
performed by the packaging script.

To notarize a DMG instead, first Developer ID-sign the app, recreate the DMG,
submit that DMG with `notarytool`, and staple the accepted DMG.

## Acceptance on another compatible Mac

1. Copy the ZIP to a new directory and verify its published SHA-256 with
   `shasum -a 256`.
2. Expand it with `ditto -x -k Chess-0.1.0-macOS-arm64.zip .`.
3. Confirm `file Chess.app/Contents/MacOS/Chess` reports arm64.
4. Open `Chess.app`. An ad-hoc build may require the local-development
   Gatekeeper exception appropriate to that Mac; it has not been notarized.
5. Confirm the board and all piece images appear. Exercise New, Undo, Save,
   Save As, and Load, using a temporary XML file and confirming the loaded
   position and undo history.
6. Quit and reopen the copied app without Homebrew or a source-tree working
   directory in the launch command.
