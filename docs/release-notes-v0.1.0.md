# Chess v0.1.0

Chess v0.1.0 modernizes the original desktop game as a portable C++20 core
with a GTKmm 4 interface.

This release is published by Lucas Song as an individual.

## Highlights

- Complete local chess play with check, checkmate, stalemate, castling,
  en passant, four promotion choices, threefold repetition, the fifty-move
  rule, and insufficient-material draws.
- Clear gold selected-square treatment, translucent green legal destinations,
  and green capture destinations retained with a red border.
- Human-versus-human, human-versus-computer, computer-versus-human, and
  computer-versus-computer modes.
- Undo plus atomic XML Save, Save As, and Load with legacy-save compatibility.
- Project-owned chess-piece artwork and a high-resolution application icon.
- Third-party license notices included with packaged artifacts.

## Platforms and installation

The macOS artifact targets Apple Silicon and macOS 26.0 or newer. Drag
`Chess.app` from the DMG to Applications, then open it normally. The public
artifact must be Developer ID signed, notarized, and stapled before these
instructions are considered final.

An optional unsigned `.deb` may be provided for Ubuntu 24.04 and elementary
OS 8.1. Install it with:

```sh
sudo apt install ./chess-game_0.1.0-1_amd64.deb
```

See [known limitations](known-limitations.md) and [privacy](privacy.md).
