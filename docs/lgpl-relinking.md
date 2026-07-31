# LGPL libraries and local relinking on macOS

Chess itself is distributed under the license in `LICENSE`. Its macOS bundle
also contains separately built dynamic libraries under LGPL and other
licenses. `THIRD_PARTY_NOTICES.md` identifies every bundled library and
`third_party/macos-arm64-v0.1.0-sources.tsv` pins the corresponding source,
formula, and binary provenance for v0.1.0.

This document is compliance and reproducibility information, not legal advice.

## Public release security policy

The official Developer ID build must:

- enable Hardened Runtime and secure timestamping;
- keep Library Validation enabled;
- sign every nested dylib and the outer App with the same Developer ID team;
- use no `com.apple.security.cs.disable-library-validation` entitlement; and
- contain no `get-task-allow` entitlement.

Apple documents that Hardened Runtime enables Library Validation by default.
It normally permits libraries signed by Apple or by the same team as the main
executable:

https://developer.apple.com/documentation/bundleresources/entitlements/com.apple.security.cs.disable-library-validation

Keeping that protection in the official App prevents an unrelated,
unexpectedly signed library from being injected into the game.

## Exact corresponding sources

Create the source archive intended for the GitHub Release with:

```sh
scripts/prepare-third-party-sources.sh \
  --app dist/macos/Chess.app
```

The command downloads but does not install software. It verifies every
upstream archive and Homebrew formula against the SHA-256 values in the
manifest, then creates:

```text
dist/Chess-0.1.0-third-party-sources.tar.gz
```

The archive contains:

- the exact upstream source archive for all 34 bundled components;
- the Homebrew/core formula snapshot used to identify each bottle build;
- the custom LZO-free Cairo build script;
- this relinking guide and the local relinking helper; and
- a `SHA256SUMS` file covering the archive contents.

The source archive must be attached to the same public release as the macOS
binary artifacts. Do not substitute a moving branch or an unversioned project
homepage for this versioned archive.

## Testing a modified LGPL library

Modifying an official App invalidates its Developer ID signature and
notarization seal. Users do not need, and must never receive, the release
owner's Developer ID certificate or private key.

Build an interface-compatible arm64 dylib from the corresponding source, then
create a separate local copy:

```sh
scripts/relink-macos-lgpl.sh \
  --app dist/macos/Chess.app \
  --output-app dist/local-lgpl-test/Chess.app \
  --replace /absolute/path/to/libfribidi.0.dylib
```

Repeat `--replace` to replace more than one dylib. The helper:

1. refuses to modify the source App or overwrite an existing output;
2. requires an arm64 Mach-O dylib with a basename already present in the App;
3. rejects dependencies not supplied by macOS or the App;
4. rewrites resolvable dependencies to the App's `Frameworks` directory;
5. ad-hoc signs the copied dylibs and App with Hardened Runtime;
6. disables Library Validation only in that local modified copy; and
7. verifies the resulting code-signature closure.

The local entitlement is stored in
`scripts/lgpl-local-modification.entitlements`. It must not be used by the
official Developer ID release.

The modified copy is intentionally not Developer ID signed, notarized, or
Gatekeeper-equivalent to the official download. The user may run it only under
the local-development security decisions appropriate to their own Mac.

## Release acceptance

Before publication:

1. Verify manifest coverage against the final App:

   ```sh
   scripts/prepare-third-party-sources.sh \
     --verify-manifest-only \
     --app dist/macos/Chess.app
   ```

2. Build the source archive and verify its reported SHA-256.
3. Exercise the relink helper with at least one interface-compatible rebuilt
   LGPL dylib.
4. Launch the modified copy and exercise New, move, Undo, Save, and Load.
5. Confirm the official App has Hardened Runtime but does not contain Disable
   Library Validation or `get-task-allow`.
6. Confirm the modified test copy does contain Disable Library Validation and
   no Developer ID identity.

GNU publishes the LGPL license texts and its licensing FAQ at:

- https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
- https://www.gnu.org/licenses/lgpl-3.0.html
- https://www.gnu.org/licenses/gpl-faq.html
