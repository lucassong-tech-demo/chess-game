# chess-game

This repository contains a portable C++20 chess core and a GTKmm 4 desktop
application. The current GUI is built directly in C++ with `Gtk::Application`,
`Gtk::ApplicationWindow`, and a dynamic 8×8 `Gtk::Grid`; it does not load
`cs240chess.glade` or use GTK2/libglade APIs.

## Architecture

- `model/` and `utils/` form `chess_core` and have no GTK dependency.
- `gui/src/ChessSession.cpp` is a display-independent interaction layer. It
  owns `GameFacade` and coordinates selection, legal moves, turns, captures,
  undo, new game, check, checkmate, and stalemate.
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
its board, valid-move set, undo result, and move history with RAII types.
History entries contain piece snapshots and coordinates rather than pointers
to live pieces, so undo can safely reconstruct a captured piece.

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

The application supports click-to-select, legal-move and capture highlighting,
move/capture, undo, new game, turn and terminal-state status, save, quit, and
an about dialog. Common shortcuts are:

- New game: `Command-N` on macOS, `Ctrl-N` on Linux
- Undo: `Command-Z` / `Ctrl-Z`
- Save: `Command-S` / `Ctrl-S`
- Load: `Command-O` / `Ctrl-O`
- Quit: `Command-Q` / `Ctrl-Q`

The Load action deliberately reports that loading is unavailable because the
current core's `GameFacade::LoadGame()` is still a stub. The legacy computer
player/controller is not connected to the new application, so the current GUI
is human-versus-human only. Castling, promotion, and en passant remain subject
to the existing core's legacy rule set; the GTK layer does not invent them.

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
`werror=true`. The tests cover core ownership and movement behavior plus the
display-independent click, highlight, move, capture, undo, and new-game flow.
