# chess-game

This repository contains a portable C++20 chess core and a GTKmm 4 desktop
application. The current GUI is built directly in C++ with `Gtk::Application`,
`Gtk::ApplicationWindow`, and a dynamic 8×8 `Gtk::Grid`.

## Architecture

- `model/` forms `chess_core` and has no GTK dependency.
- `gui/src/ChessSession.cpp` is a display-independent interaction layer. It
  owns `GameFacade` and coordinates selection, explicit promotion, player
  modes, and file-path behavior. Rules and persistent game state stay in the
  GTK-independent facade.
- `ChessBoardView` renders model state and reports cell clicks through
  `Gtk::GestureClick`; it does not implement chess rules or copy the board.
- `ChessWindow` owns application actions, menus, status presentation, and the
  save dialog.
- CSS and the 12 project-owned piece PNGs under `gui/resources/pieces/` are
compiled into a `Gio::Resource`, so the executable can run from any working
directory. macOS packaging derives its `.icns` from the project-owned
1024×1024 master at `gui/resources/icons/app-icon-1024.png`.

The retired GTK2/libglade controller and view, Glade layout, and custom
HTTP/HTML utility stack have been removed. Active source and build paths use
only the C++20 model and GTKmm 4 GUI.

The MIT license covers the project-owned code and runtime artwork committed to
this repository. Unpublished artwork masters and candidates kept outside Git
are not part of this repository or its distributed license grant.

## Core ownership model

`ChessBoard` is the sole owner of live pieces. Its fixed 8×8 grid stores
`std::unique_ptr<Piece>` values; APIs outside the board receive only non-owning
pointers or references. Moves and temporary move simulations transfer
ownership explicitly, including ownership of captured pieces.

Move generation returns `std::set<BoardPosition>` by value. `GameFacade` owns
the board, turn, castling rights, en-passant target, halfmove/fullmove clocks,
repetition keys, terminal status, and move history with RAII types. History
entries contain value snapshots and the pre-move special state, so ordinary
moves, captures, castling, en passant, and all four promotion choices can be
undone exactly.

The core enforces check safety, checkmate, stalemate, threefold repetition, the
fifty-move rule, and the standard insufficient-material cases. Terminal games
reject further moves but remain undoable.

## Save format

New saves are UTF-8 XML with a `<chessgame version="2">` root. They include:

- current board, side to move, castling rights, en-passant target, clocks;
- complete undo history, including special-move and prior-state metadata;
- repetition keys required to preserve draw detection after loading.

Writes use a sibling temporary file followed by an atomic rename. Loads parse
and validate into a temporary state before replacing the active game. Failed
reads therefore leave the board, turn, history, and current file unchanged.
The reader also accepts the repository's original unversioned format,
including the historical `-<chessgame>` / `-<board>` prefixes in `try.xml`.

## Requirements

The project uses C++20, Meson, and Ninja. The optional GUI requires
GTKmm **4.10 or newer**.

macOS (Apple Silicon, Homebrew):

```sh
brew install meson ninja pkgconf gtkmm4
```

elementary OS 8.1 / Ubuntu 24.04 Noble:

```sh
sudo apt install build-essential meson ninja-build pkg-config libgtkmm-4.0-dev
```

Ubuntu 24.04 provides GTKmm 4.10, which is the minimum supported API baseline.

## Core-only build and test

The core and both non-GUI test executables remain buildable without GTKmm:

```sh
meson setup build-core -Dgui=disabled
meson compile -C build-core
meson test -C build-core
```

If `gui=auto` (the default), Meson builds the GUI when `gtkmm-4.0 >= 4.10` is
available and otherwise builds only the core and display-independent tests.
Use `-Dgui=enabled` to require GTKmm and fail configuration when it is absent.

## GUI build and run

```sh
meson setup build-gui -Dgui=enabled
meson compile -C build-gui
meson test -C build-gui
./build-gui/chess-game
```

The application marks the selected piece with a gold square, ordinary legal
moves with green squares, and captures with red borders. It also supports explicit
promotion choice, castling and en passant, undo, new game, complete
turn/check/draw status, current-file status, four human/computer combinations,
Save, Save As, Load, quit, and an about dialog. The deliberately simple computer
player chooses a legal move deterministically and explicitly chooses a queen
when it promotes. Common shortcuts are:

- New game: `Command-N` on macOS, `Ctrl-N` on Linux
- Undo: `Command-Z` / `Ctrl-Z`
- Save: `Command-S` / `Ctrl-S`
- Save As: `Command-Shift-S` / `Ctrl-Shift-S`
- Load: `Command-O` / `Ctrl-O`
- Quit: `Command-Q` / `Ctrl-Q`

## macOS delivery

On Apple Silicon macOS, first build the audited Cairo 1.18.4 dylibs with LZO
disabled, then create a relocatable, ad-hoc-signed Release app and ZIP:

```sh
scripts/build-macos-cairo-no-lzo.sh
scripts/package-macos.sh
```

Add `--dmg` to the packaging command to also create and verify a DMG. See
[`docs/macos-delivery.md`](docs/macos-delivery.md) for artifact acceptance,
Developer ID signing, and notarization steps. Local artifacts are written
under the Git-ignored `dist/macos/` directory. The dependency build is also
Git-ignored and does not install or replace Homebrew libraries.

The v0.1.0 macOS target is Apple Silicon with macOS 26.0 or newer. This
baseline follows the minimum version encoded in the executable and every
bundled Homebrew library; lowering it requires rebuilding and retesting the
complete dependency closure against an older SDK.

Before public distribution, validate the exact 34-component/46-dylib source
manifest and create the corresponding-source archive:

```sh
scripts/prepare-third-party-sources.sh \
  --app dist/macos/Chess.app
```

The official App keeps Hardened Runtime Library Validation enabled. See
[`docs/lgpl-relinking.md`](docs/lgpl-relinking.md) for the separate local
workflow that lets recipients test an interface-compatible modified LGPL
dylib without the release owner's Developer ID credentials.

## Linux verification and delivery

On elementary OS 8.1 / Ubuntu 24.04, run `scripts/verify-linux.sh` for a
Release build, tests, ELF dependency checks, and a GUI startup probe. Run
`scripts/package-linux-deb.sh` to create
`dist/linux/chess-game_0.1.0-1_amd64.deb`. The v0.1.0 GitHub Release scope
includes this unsigned, best-effort x86_64 package for Ubuntu 24.04 and
elementary OS 8.1. It is not an official Debian archive or signed apt package,
and it uses GTKmm/GTK and other shared libraries supplied by the target
system. See [`docs/linux-delivery.md`](docs/linux-delivery.md) for installation
and Gate 7 manual acceptance steps.

## Sanitizers

AddressSanitizer and UndefinedBehaviorSanitizer can be enabled in a separate
core-only build:

```sh
meson setup build-sanitize \
  -Dgui=disabled \
  -Db_sanitize=address,undefined \
  -Db_lundef=false
meson compile -C build-sanitize
# Linux:
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
  meson test -C build-sanitize
# macOS (Apple sanitizer has no LeakSanitizer support):
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
  meson test -C build-sanitize
```

Leak detection is enabled on the Ubuntu 24.04 CI job. The macOS job keeps
`detect_leaks=0` because Apple's bundled sanitizer runtime does not support
LeakSanitizer.

All targets use strict compiler warnings with `warning_level=3` and
`werror=true`. Tests cover piece boundaries, turn enforcement, check filtering,
mate/stalemate, draw rules, every special move and undo, terminal undo, new
game, XML round trips and legacy compatibility, invalid/unavailable paths,
atomic failed loads, Save/Save As path behavior, selection clearing, promotion,
and all player combinations.

## Release information

- [Privacy](docs/privacy.md)
- [Known limitations](docs/known-limitations.md)
- [v0.1.0 release notes](docs/release-notes-v0.1.0.md)
- [v0.1.0 release checklist](docs/release-v0.1.0.md)
- [Third-party notices](THIRD_PARTY_NOTICES.md)
- [LGPL sources and local relinking](docs/lgpl-relinking.md)
