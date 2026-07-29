# Known limitations for v0.1.0

- The macOS build is arm64-only and requires macOS 26.0 or newer. Intel Macs
  and older macOS releases are not supported by this artifact.
- Until a final artifact is Developer ID signed, notarized, and stapled, the
  ad-hoc package is for local testing only and does not provide the normal
  Gatekeeper experience expected for public distribution.
- Play is local only. There is no network multiplayer, matchmaking, account,
  cloud save, or synchronization feature.
- The computer opponent is deliberately simple and deterministic. It is not
  intended to provide engine-strength play or adjustable difficulty.
- The XML save format is application-specific. It is not PGN and does not
  import or export PGN games.
- A Linux `.deb`, if released, is an unsigned best-effort artifact for Ubuntu
  24.04 / elementary OS 8.1. It is not an official Debian archive package or
  signed apt repository.
