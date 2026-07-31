# Linux verification and Debian delivery

The supported Linux baseline is elementary OS 8.1 / Ubuntu 24.04 with
GTKmm 4.10 or newer on x86_64. The v0.1.0 release scope includes
`chess-game_0.1.0-1_amd64.deb` as an unsigned, best-effort GitHub Release
artifact. It is not an official Debian archive package or a signed apt
repository. Unlike the macOS app, it does not bundle GTKmm/GTK; apt resolves
the shared-library dependencies calculated from the executable by
`dpkg-shlibdeps`.

Install the build and local Debian-package tools:

```sh
sudo apt update
sudo apt install \
  build-essential \
  binutils \
  dbus-x11 \
  desktop-file-utils \
  dpkg-dev \
  file \
  libglib2.0-bin \
  libgtkmm-4.0-dev \
  meson \
  ninja-build \
  pkg-config \
  xvfb
```

## Release acceptance

From the repository root, run:

```sh
scripts/verify-linux.sh
```

Relative `--build-dir` and `--output-dir` arguments are resolved from the
repository root, so the GitHub Actions invocation can safely use
`--build-dir build-linux --output-dir dist/linux`.

This configures `build-linux-release` with `--buildtype=release` and
`-Dgui=enabled`, builds it, runs the core and session tests, checks that the
application is a supported 64-bit ELF, checks `ldd` for missing libraries,
rejects source/build RPATHs, uses `gresource` to verify the embedded CSS and
all 12 piece images, and verifies that the GTK process stays alive for eight
seconds. It uses the current X11/Wayland session, or `xvfb-run` when headless.
Headless probes select GTK's Cairo software renderer so they do not require a
physical DRI3/EGL device.

For normal development:

```sh
meson setup build-linux-dev --buildtype=debug -Dgui=enabled
meson compile -C build-linux-dev
meson test -C build-linux-dev --print-errorlogs
./build-linux-dev/chess-game
```

After the automated probe, manually confirm:

1. The complete 8×8 board, all pieces, and CSS styling appear.
2. New resets the board and Undo restores the preceding position.
3. Save As creates an XML file, Save updates it, and Load restores it.
4. The loaded game remains undoable.
5. Closing and reopening the application succeeds.

The special chess rules are covered by existing tests but are not part of the
Linux delivery acceptance click-through. Before v0.1.0 public release, the
automated suite must cover castling, en passant, every promotion choice,
threefold repetition, the fifty-move rule, and insufficient material.

## Build and inspect the v0.1.0 `.deb`

Run:

```sh
scripts/package-linux-deb.sh
```

The script repeats the Release build/test/ELF checks, strips a staged copy of
the executable, asks `dpkg-shlibdeps` to calculate its runtime package
dependencies, and creates:

```text
dist/linux/chess-game_0.1.0-1_amd64.deb
```

On an arm64 Linux build machine the architecture suffix is `arm64`. A `.deb`
must be built separately on each target architecture; the script does not
cross-compile.

Inspect and install the package:

```sh
dpkg-deb --info dist/linux/chess-game_0.1.0-1_amd64.deb
dpkg-deb --contents dist/linux/chess-game_0.1.0-1_amd64.deb
sha256sum dist/linux/chess-game_0.1.0-1_amd64.deb
sudo apt install ./dist/linux/chess-game_0.1.0-1_amd64.deb
chess-game
```

It installs:

```text
/usr/bin/chess-game
/usr/share/applications/io.github.chess_game.desktop
/usr/share/icons/hicolor/128x128/apps/io.github.chess_game.png
/usr/share/doc/chess-game/copyright
/usr/share/doc/chess-game/THIRD_PARTY_NOTICES.md
/usr/share/doc/chess-game/licenses/
```

Remove the locally installed package with:

```sh
sudo apt remove chess-game
```

The generated `.deb` is the unsigned, best-effort Linux artifact planned for
the v0.1.0 GitHub Release. It is not an official Debian archive package and
not a signed apt repository. Broader distribution would additionally require
maintained Debian source packaging, changelog and signing/repository
infrastructure.

## Gate 7: elementary OS 8.1 clean-machine acceptance

Do not record this acceptance against a moving development checkout. Run it
later on the elementary OS 8.1 x86_64 machine, after the release candidate
commit is frozen and pushed, and first confirm that `git rev-parse HEAD`
matches the commit recorded in `docs/release-v0.1.0.md`.

From the frozen repository checkout:

```sh
git status --short
git rev-parse HEAD
scripts/verify-linux.sh
scripts/package-linux-deb.sh
sha256sum dist/linux/chess-game_0.1.0-1_amd64.deb
dpkg-deb --info dist/linux/chess-game_0.1.0-1_amd64.deb
dpkg-deb --contents dist/linux/chess-game_0.1.0-1_amd64.deb
dpkg-deb -f dist/linux/chess-game_0.1.0-1_amd64.deb \
  Package Version Architecture Depends
sudo apt install ./dist/linux/chess-game_0.1.0-1_amd64.deb
ldd /usr/bin/chess-game
chess-game
```

Record the commit and SHA-256, then manually confirm:

1. `git status --short` was empty and the commit matched the frozen candidate.
2. The package reports `Package: chess-game`, `Version: 0.1.0-1`, and
   `Architecture: amd64`, and apt resolves all declared dependencies.
3. `ldd /usr/bin/chess-game` contains no `not found` entry.
4. Launch succeeds both from the application menu and with `chess-game`.
5. The complete board, project-owned pieces, selected square, green legal
   destinations, red-bordered captures, icon, and About information render
   correctly.
6. New, ordinary moves, captures, repeated Undo, Save, Save As, Load, and a
   malformed/missing XML load all behave correctly; a failed load preserves
   the current game.
7. Human/Human, Human/Computer, Computer/Human, and Computer/Computer modes
   all operate.
8. Closing and reopening the installed application succeeds.

After the functional checks, uninstall and verify that the executable is no
longer installed:

```sh
sudo apt remove chess-game
test ! -e /usr/bin/chess-game
```

Record the tester, date, distribution version, architecture, GTKmm version,
package SHA-256, uninstall result, and overall Pass/Fail in
`docs/release-v0.1.0.md`. These checks cannot be completed or claimed from a
Mac.

## Why `.deb` first

elementary OS and Ubuntu are Debian-family systems, so `.deb` integrates with
apt, declares shared-library dependencies, installs a desktop entry, and is
the smallest useful delivery step. Unlike the macOS bundle, it does not copy
GTKmm into the package; apt installs compatible runtime libraries.

AppImage would be useful for a broader range of distributions but needs an
additional AppImage packaging tool and careful GTK plugin/runtime bundling.
Flatpak would provide the strongest runtime isolation but requires a Flatpak
manifest, an SDK/runtime, and usually a repository such as Flathub. Neither is
generated by the current scripts, and no additional tools are installed
automatically.
