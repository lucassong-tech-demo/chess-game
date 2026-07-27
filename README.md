# chess-game

This repository contains a portable C++20 chess core and a GTKmm 4 desktop
application. The current GUI is built directly in C++ with `Gtk::Application`,
`Gtk::ApplicationWindow`, and a dynamic 8×8 `Gtk::Grid`; it does not load
`cs240chess.glade` or use GTK2/libglade APIs.

## Architecture

- `model/` and `utils/` form `chess_core` and have no GTK dependency.
- `gui/src/ChessSession.cpp` is a display-independent interaction layer. It
  owns `GameFacade` and coordinates selection, explicit promotion, player
  modes, and file-path behavior. Rules and persistent game state stay in the
  GTK-independent facade.
- `ChessBoardView` renders model state and reports cell clicks through
  `Gtk::GestureClick`; it does not implement chess rules or copy the board.
- `ChessWindow` owns application actions, menus, status presentation, and the
  save dialog.
- CSS and the existing piece PNG files are compiled into a `Gio::Resource`, so
  the executable can run from any working directory.

The historical `controller/`, `view/`, and `cs240chess.glade` files remain in
the repository for reference only. They are legacy GTK2-era code and are not
part of any Meson target.

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

The application supports legal-move/capture highlighting, explicit promotion
choice, castling and en passant, undo, new game, complete turn/check/draw
status, current-file status, four human/computer combinations, Save, Save As,
Load, quit, and an about dialog. The deliberately simple computer player
chooses a legal move deterministically and explicitly chooses a queen when it
promotes. Common shortcuts are:

- New game: `Command-N` on macOS, `Ctrl-N` on Linux
- Undo: `Command-Z` / `Ctrl-Z`
- Save: `Command-S` / `Ctrl-S`
- Save As: `Command-Shift-S` / `Ctrl-Shift-S`
- Load: `Command-O` / `Ctrl-O`
- Quit: `Command-Q` / `Ctrl-Q`

## macOS delivery

On Apple Silicon macOS, create a relocatable, ad-hoc-signed Release app and
ZIP with `scripts/package-macos.sh`; add `--dmg` to also create a DMG. See
[`docs/macos-delivery.md`](docs/macos-delivery.md) for artifact acceptance,
Developer ID signing, and notarization steps. Local artifacts are written
under the Git-ignored `dist/macos/` directory.

## Linux verification and delivery

On elementary OS 8.1 / Ubuntu 24.04, run `scripts/verify-linux.sh` for a
Release build, tests, ELF dependency checks, and a GUI startup probe. Run
`scripts/package-linux-deb.sh` to create a local Debian package under
`dist/linux/`. See [`docs/linux-delivery.md`](docs/linux-delivery.md) for
installation and manual acceptance steps.

## Sanitizers

AddressSanitizer and UndefinedBehaviorSanitizer can be enabled in a separate
core-only build:

```sh
meson setup build-sanitize \
  -Dgui=disabled \
  -Db_sanitize=address,undefined \
  -Db_lundef=false
meson compile -C build-sanitize
meson test -C build-sanitize
```

All targets use strict compiler warnings with `warning_level=3` and
`werror=true`. Tests cover piece boundaries, turn enforcement, check filtering,
mate/stalemate, draw rules, every special move and undo, terminal undo, new
game, XML round trips and legacy compatibility, invalid/unavailable paths,
atomic failed loads, Save/Save As path behavior, selection clearing, promotion,
and all player combinations.
