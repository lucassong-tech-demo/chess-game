# chess-game

This phase builds and tests the portable C++ chess core only. The legacy
GTK/Glade GUI is intentionally excluded from the default build and GTKmm is not
required.

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

## Sanitizers

AddressSanitizer and UndefinedBehaviorSanitizer can be enabled in a separate
build directory:

```sh
meson setup build-sanitize -Db_sanitize=address,undefined -Db_lundef=false
meson compile -C build-sanitize
meson test -C build-sanitize
```

To recreate either build directory, pass `--wipe` to `meson setup`.
