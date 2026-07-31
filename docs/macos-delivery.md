# macOS delivery

The repository's `scripts/build-macos-cairo-no-lzo.sh` first builds the three
Cairo dylibs required by GTK from the audited Cairo 1.18.4 source with LZO
explicitly disabled. It checks the official source SHA-256, arm64 architecture,
absence of `liblzo` load commands, and the GTK-required Cairo symbols. The
build remains under the Git-ignored source tree and does not install or replace
Homebrew libraries.

`scripts/package-macos.sh` then creates an arm64 Release `Chess.app`, runs both
Meson tests, recursively bundles non-system Mach-O dependencies, substitutes
the verified LZO-free Cairo dylibs, removes Homebrew/build-tree load paths,
applies an ad-hoc signature, verifies it, and creates a ZIP. Pass `--dmg` to
also create and verify a compressed DMG with an Applications shortcut.

The default outputs are under `dist/macos/`, which is ignored by Git. The
script only accepts an output directory inside this repository and only
replaces its exact `Chess.app` and versioned Chess archive paths. It exits
nonzero on a missing tool, build/test failure, non-arm64 input, missing
dependency, invalid signature, or non-relocatable Mach-O reference. It rejects
non-macOS hosts before configuring a build, so Linux builds and both existing
CI workflows are unaffected.

```sh
scripts/build-macos-cairo-no-lzo.sh
scripts/package-macos.sh --dmg
```

The Cairo build uses Homebrew's already-downloaded official source archive by
default and does not initiate a download. An audited local archive can instead
be supplied with `--source-archive PATH`. Packaging fails if the three
overrides are absent, if any override links LZO, if `liblzo2` enters the
recursive application closure, or if an LZO-named library is present.

The generated icon is derived solely from the tracked, project-owned
`gui/resources/icons/app-icon-1024.png` master. The script derives and
validates a 512×512 `Chess.icns` representation while retaining the 1024×1024
master for future packaging formats and artwork revisions.

The app identifier is `io.github.lucassong-tech-demo.chess-game`. The current
binary and its bundled libraries are arm64 and encode macOS 26.0 as their
minimum system version, so v0.1.0 must not claim compatibility with an older
macOS release.

`Contents/Resources` includes the project's MIT license,
`THIRD_PARTY_NOTICES.md`, the exact third-party source manifest,
`LGPL_RELINKING.md`, and the corresponding upstream license texts. A release
fails packaging if those materials are absent.

## Third-party sources and LGPL relinking

Validate that the source manifest covers the exact App dylib closure:

```sh
scripts/prepare-third-party-sources.sh \
  --verify-manifest-only \
  --app dist/macos/Chess.app
```

Create the corresponding-source archive for the GitHub Release:

```sh
scripts/prepare-third-party-sources.sh \
  --app dist/macos/Chess.app
```

This downloads and verifies source archives and formula snapshots but installs
nothing. Publish the resulting
`Chess-0.1.0-third-party-sources.tar.gz` beside the binary artifacts.

The official Developer ID build keeps Library Validation enabled and does not
use a Disable Library Validation entitlement. Recipients can test an
interface-compatible modified LGPL dylib in a separate ad-hoc-signed App copy
using `scripts/relink-macos-lgpl.sh`. See
[`lgpl-relinking.md`](lgpl-relinking.md) for the security model and acceptance
steps.

## Developer ID and notarization

The default bundle uses an ad-hoc signature (`-`) for local validation. It is
not Developer ID signed, notarized, or Gatekeeper-approved for distribution.
Formal signing support will be added only after the release contents are
frozen and the release operator explicitly authorizes use of a locally
installed Developer ID Application identity.

No entitlements are currently required by the application. If future features
need them, review a minimal property list before signing rather than adding
broad exceptions. Store notarization credentials in a local Keychain profile;
never put certificate passwords, API keys, or profiles in this repository or
command output. Apple submission and identity signing are intentionally not
performed by the packaging script.

The `scripts/lgpl-local-modification.entitlements` file is solely for a
recipient's locally modified, ad-hoc-signed copy. It must never be passed to
the formal Developer ID signing path.

Formal signing must proceed from inner dylibs to the outer app using hardened
runtime and a secure timestamp. Verification must confirm the signing
authority, Team ID, runtime flag, absence of `get-task-allow`, arm64
architecture, minimum system version, Mach-O closure, and packaged notices.

Apple notarization is a separate, explicitly authorized network operation.
Submit with `xcrun notarytool`, preserve the submission ID, and inspect the
log even when the result is Accepted. Staple and validate the accepted app
before rebuilding a ZIP; ZIP files themselves cannot be stapled. If
distributing a DMG, submit and staple the final DMG as well.

## Acceptance on another compatible Mac

1. Copy the ZIP to a new directory and verify its published SHA-256 with
   `shasum -a 256`.
2. Expand it with `ditto -x -k Chess-0.1.0-macOS-arm64.zip .`.
3. Confirm `file Chess.app/Contents/MacOS/Chess` reports arm64.
4. Confirm `find Chess.app -iname '*lzo*'` produces no output, and that no
   Mach-O file reports a `liblzo` dependency.
5. Open `Chess.app`. An ad-hoc build may require the local-development
   Gatekeeper exception appropriate to that Mac; it has not been notarized.
6. Confirm the board and all piece images appear. Exercise New, Undo, Save,
   Save As, and Load, using a temporary XML file and confirming the loaded
   position and undo history.
7. Quit and reopen the copied app without Homebrew or a source-tree working
   directory in the launch command.
