# v0.1.0-rc1 acceptance record

This document records the release-candidate acceptance status for the
modernized chess game. It distinguishes automated evidence from manual
desktop checks that still require a person to operate the packaged
applications.

## Release identity

| Item | Value |
|---|---|
| Branch | `main` |
| Tag | `v0.1.0-rc1` |
| Commit | `fcc20f54ea79bef84f8078316edcf608d44b55e7` |
| Source tree | `fb34bac518e81a3ac7db07f3f28eee0ddad53703` |
| Target macOS | Apple Silicon, macOS 26.0 or newer |
| Target Linux | elementary OS 8.1 / Ubuntu 24.04, x86_64 |

The tagged merge commit and the final `codex-gtkmm3` commit have the same Git
tree. The merge therefore did not change the source that produced the
previously verified release artifacts.

## Automated acceptance

| Check | Status | Evidence |
|---|---:|---|
| Migration merged into `main` | Pass | Merge commit `fcc20f5` |
| Annotated RC tag points to the merge | Pass | `v0.1.0-rc1` |
| macOS arm64 and Ubuntu x86_64 core builds | Pass | [Core CI run 30314746406](https://github.com/lucassong-tech-demo/chess-game/actions/runs/30314746406) |
| Core and session tests | Pass | Core CI and Linux CI |
| ASan and UBSan baseline | Pass | Core CI |
| Ubuntu 24.04 GTKmm 4.10 Release build | Pass | [Linux run 30314746483](https://github.com/lucassong-tech-demo/chess-game/actions/runs/30314746483) |
| Linux ELF dependencies and embedded resources | Pass | Linux run |
| Headless Linux GUI startup probe | Pass | Linux run |
| Unsigned test `.deb` creation | Pass | Linux run artifact |
| macOS relocatable bundle dependency closure | Pass | 48 Mach-O files checked; 47 private dylibs |
| macOS ad-hoc signature | Pass | `codesign --verify --deep --strict` |
| macOS relocated startup probe | Pass | Application launched from a temporary directory without Homebrew access |

LeakSanitizer is not part of the current Linux CI baseline. ASan and UBSan
remain enabled.

## macOS artifacts

The following locally generated artifacts correspond to the tagged source
tree:

| Artifact | SHA-256 |
|---|---|
| `Chess-0.1.0-macOS-arm64.zip` | `b4ce8f218aa261971890c20b7a8f6bc720f4c41aecfc8705ab4a3b3d4a4f0e47` |
| `Chess-0.1.0-macOS-arm64.dmg` | `c20d5494cc010ccc02530e5da03638ac6b668e1d1c979fd26848d7510f6e5174` |

These artifacts are ad-hoc signed development deliverables. They are not
Developer ID signed, notarized, stapled, or approved for public Gatekeeper
distribution.

## Manual macOS acceptance

Test the packaged `Chess.app`, not the executable in a Meson build directory.
Record the tester, date, machine and result below.

- [ ] Verify the published ZIP or DMG SHA-256.
- [ ] Copy or extract `Chess.app` into a new directory and launch it.
- [ ] Confirm the board, all 32 pieces and CSS styling render correctly.
- [ ] Confirm White moves first and turns alternate.
- [ ] Move a piece and capture an opposing piece.
- [ ] Use Undo repeatedly and confirm pieces and turns are restored.
- [ ] Start New Game and confirm board, turn, history and file state reset.
- [ ] Use Save As to create an XML file.
- [ ] Make another move and use Save to update the same file.
- [ ] Start a new game, then Load the XML and verify board and history.
- [ ] Load malformed or missing XML and confirm the current game is preserved.
- [ ] Exercise Human/Human, Human/Computer, Computer/Human and
      Computer/Computer modes.
- [ ] Quit and reopen the application without a Homebrew or source-tree launch
      command.

| Field | Result |
|---|---|
| Tester | Pending |
| Date | Pending |
| Mac model / architecture | Pending |
| macOS version | Pending |
| ZIP or DMG tested | Pending |
| Overall result | Pending |
| Notes / defects | Pending |

## Manual elementary OS acceptance

Download the `.deb` artifact from the successful Linux workflow and test it on
the elementary OS 8.1 machine.

```sh
sha256sum chess-game_0.1.0-1_amd64.deb
dpkg-deb --info chess-game_0.1.0-1_amd64.deb
dpkg-deb --contents chess-game_0.1.0-1_amd64.deb
sudo apt install ./chess-game_0.1.0-1_amd64.deb
chess-game
```

- [ ] Confirm installation succeeds and dependencies resolve through apt.
- [ ] Launch from the application menu and with `chess-game`.
- [ ] Confirm the board, pieces and CSS styling render correctly.
- [ ] Complete move, capture, repeated Undo and New Game checks.
- [ ] Complete Save As, Save and Load checks.
- [ ] Confirm a loaded game remains undoable.
- [ ] Exercise all four player combinations.
- [ ] Close and reopen the installed application.
- [ ] Remove it with `sudo apt remove chess-game`.

| Field | Result |
|---|---|
| Tester | Pending |
| Date | Pending |
| Distribution | elementary OS 8.1 |
| Architecture | x86_64 |
| GTKmm version | Pending |
| `.deb` SHA-256 | Pending |
| Overall result | Pending |
| Notes / defects | Pending |

## Deferred or non-blocking items

The following items are not blockers for RC1 development acceptance, but must
remain visible:

- Linux LeakSanitizer is currently disabled.
- Legacy GTK2 controller/view and custom HTTP/HTML sources remain in the
  repository but are excluded from the modern build and packages.
- `ChessBoard::Test()` and `GameFacade::Test()` are still embedded in
  production model sources.
- Castling, en passant, promotion and additional draw rules are implemented
  and covered by automated tests, but their comprehensive post-migration
  acceptance was explicitly deferred.
- The macOS icon is generated from a 128×128 project image.
- The macOS package is arm64-only and requires macOS 26.0 or newer.
- Public macOS distribution still requires third-party license notices,
  Developer ID signing, notarization and stapling.

## RC1 decision

Automated acceptance is complete. Final two-platform release-candidate
acceptance remains **pending** until both manual sections are completed and
their overall results are recorded as Pass.
