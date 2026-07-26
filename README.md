# chess-game

This phase builds and tests the portable C++ chess core only. The legacy
GTK/Glade GUI is intentionally excluded from the default build and GTKmm is not
required.

## Core ownership model

`ChessBoard` is the sole owner of live pieces. Its fixed 8×8 grid stores
`std::unique_ptr<Piece>` values; APIs outside the board receive only
non-owning pointers or references. Moving and temporarily simulating a move
transfer ownership explicitly, including ownership of a captured piece.

Piece move generation returns `std::set<BoardPosition>` by value, so repeated
queries do not share heap-backed scratch storage. `GameFacade` owns its board,
valid-move set, undo result, and move history with standard RAII types.
History entries are values containing piece snapshots and coordinates rather
than pointers to live pieces. Undo reconstructs a captured piece from its
snapshot, avoiding dependence on an object that was destroyed during capture.

All public board-coordinate queries accept rows and columns from 0 through 7.
Out-of-range coordinates throw `std::out_of_range`; attempts to move from an
empty cell or to the same cell throw `std::invalid_argument`.

## Requirements

The project uses C++20, Meson, and Ninja.

macOS (Apple Silicon):

```sh
brew install meson ninja pkgconf
```

elementary OS 8.1 / Ubuntu 24.04:

```sh
sudo apt install build-essential meson ninja-build pkg-config
```

## Build and test

From the repository root:

```sh
meson setup build
meson compile -C build
meson test -C build
```

Strict compiler warnings are enabled and treated as errors.
The automated tests cover initialization and clearing, repeated new games,
ordinary moves, captures and continuous captures, undo, history clearing,
coordinate validation, repeated move generation, destruction paths, and
compatibility checks for the original movement rules of all six piece types.

## Sanitizers

AddressSanitizer and UndefinedBehaviorSanitizer can be enabled in a separate
build directory:

```sh
meson setup build-sanitize -Db_sanitize=address,undefined -Db_lundef=false
meson compile -C build-sanitize
meson test -C build-sanitize
```

On platforms whose AddressSanitizer includes leak detection, the same test run
also verifies that the ownership and destruction paths have no leaks. To
recreate either build directory, pass `--wipe` to `meson setup`.
