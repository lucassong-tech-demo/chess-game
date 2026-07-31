# v0.1.0 public release checklist

This record must describe the exact source and final downloadable artifacts.
Never record passwords, private keys, API keys, Keychain profile contents, or
GitHub tokens here.

## Release identity

| Item | Value |
|---|---|
| Release branch | `codex/p2-public-release` |
| Final commit | Pending |
| Final tag | `v0.1.0` (not created) |
| Bundle identifier | `io.github.lucassong-tech-demo.chess-game` |
| Display name / version | Chess / 0.1.0 |
| Publisher / account type | Lucas Song / Individual |
| Copyright notice | `Copyright © 2026 Lucas Song` |
| macOS target | Apple Silicon arm64, macOS 26.0 or newer |
| Linux target | Ubuntu 24.04 / elementary OS 8.1 |

## Quality gates

| Check | Status | Evidence |
|---|---:|---|
| Selected, legal-move, and capture highlights visually distinct | Pass | User acceptance on 2026-07-28 |
| New, move, capture, and repeated Undo | Pass | User acceptance on 2026-07-28 |
| Save, Save As, Load, and failed-load atomicity | Pass | User acceptance on 2026-07-28 |
| Four human/computer modes | Pass | User acceptance on 2026-07-28 |
| Castling tests, including rights and Undo | Pass | Local automated suite |
| En-passant tests, including expiry and Undo | Pass | Local automated suite |
| Four promotion choices and Undo | Pass | Local automated suite |
| Threefold repetition | Pass | Local automated suite |
| Fifty-move rule | Pass | Local automated suite |
| Insufficient-material draws | Pass | Local automated suite |
| macOS Release core/session/GUI build and tests | Pass | Local build |
| macOS ASan/UBSan | Pass | Local build; LeakSanitizer unavailable in Apple runtime |
| Ubuntu Release, GUI smoke, ASan/UBSan/LeakSanitizer | Pass | Branch GitHub Actions run observed successful by release owner |
| macOS ad-hoc ZIP/DMG package closure | Pass | Local package before metadata freeze |
| LZO absent from macOS Mach-O closure and notices | Pass | Cairo 1.18.4 rebuilt with `-Dlzo=disabled`; 46-dylib closure audited |
| Third-party source manifest covers macOS dylibs | Pass | Machine check exactly matched 34 components to the 46-dylib App closure |
| Corresponding-source release archive | Pass | 34 sources and 34 formula snapshots verified; archive SHA-256 `83a664c43bbf38cc5c39321438acd46768337458e161773becffc46e6f59c1df` |
| LGPL local relinking workflow | Partial | Compatible-dylib replacement, runtime ad-hoc signing, entitlement, and strict verification pass; rebuilt modified dylib launch/smoke remains |
| Final package after metadata/license freeze | Pending | Rebuild required |
| elementary OS `.deb` install, launch, and uninstall | Pending / optional | Decide before publication |

## Identity, signing, and notarization

| Check | Status / value |
|---|---|
| Apple account type and public name reviewed | Pass — Individual / Lucas Song |
| Developer ID Application identity installed | Pending |
| Signed nested code from inside out | Pending |
| Hardened runtime and secure timestamp | Pending |
| Library Validation enabled in official App | Pending |
| Disable Library Validation absent from official App | Pending |
| No `get-task-allow` entitlement | Pending |
| `codesign --verify --deep --strict` | Pending |
| Signing authority and Team ID | Pending |
| Notary submission ID | Pending |
| Notary result and log warnings | Pending |
| App stapled and validated | Pending |
| DMG stapled and validated | Pending |
| `spctl --assess` and `hdiutil verify` | Pending |

## Final artifacts

| Artifact | Build commit | SHA-256 | Status |
|---|---|---|---|
| `Chess-0.1.0-macOS-arm64.dmg` | Pending | Pending | Pending |
| `Chess-0.1.0-macOS-arm64.zip` (optional) | Pending | Pending | Pending |
| `Chess-0.1.0-third-party-sources.tar.gz` | Pending | Pending | Pending |
| `SHA256SUMS` | Pending | Pending | Pending |
| `chess-game_0.1.0-1_amd64.deb` (optional) | Pending | Pending | Pending |

## Clean-environment acceptance

- [ ] Download the final asset from its intended public URL so it receives a
      genuine quarantine attribute.
- [ ] Verify the published SHA-256 before opening it.
- [ ] Test on a compatible Apple Silicon Mac or clean user environment without
      Homebrew and without the source tree.
- [ ] Confirm Gatekeeper opens it normally on first launch.
- [ ] Confirm the icon, board, pieces, notices, and all resources are present.
- [ ] Repeat New, move, capture, Undo, Save, Save As, Load, failed Load, and all
      four player-mode checks.
- [ ] Record tester, date, hardware, macOS version, artifact, and result below.

| Field | Result |
|---|---|
| Tester | Pending |
| Date | Pending |
| Mac / architecture | Pending |
| macOS version | Pending |
| Downloaded artifact | Pending |
| Overall result | Pending |
| Notes | Pending |

## Publication

- [ ] P2 changes reviewed and merged into `main`.
- [ ] Final tag `v0.1.0` points to the exact build commit; RC tags are unchanged.
- [ ] Release notes, privacy, limitations, notices, licenses, and checksums are
      attached or linked.
- [ ] GitHub Release assets match the accepted hashes.
- [ ] Public GitHub Release created only after explicit authorization.
