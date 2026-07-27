#!/bin/bash

set -euo pipefail

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "error: Linux verification is only supported on Linux" >&2
  exit 1
fi

script_dir="$(cd "$(dirname "$0")" && pwd)"
source_dir="$(cd "${script_dir}/.." && pwd)"
build_dir="${source_dir}/build-linux-release"
build=true
gui_smoke=true
smoke_seconds=8

usage() {
  cat <<'EOF'
Usage: scripts/verify-linux.sh [options]

Build and test the Linux Release application, verify its ELF dependencies,
and probe that its GTK window stays running.

Options:
  --build-dir PATH       Meson build directory (default: build-linux-release)
  --skip-build           Verify an already-built chess-game executable
  --skip-gui-smoke       Skip the window startup/keep-alive probe
  --smoke-seconds N      Probe duration in seconds (default: 8)
  -h, --help             Show this help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)
      [[ $# -ge 2 ]] || { echo "error: --build-dir needs a path" >&2; exit 2; }
      build_dir="$2"
      shift 2
      ;;
    --skip-build)
      build=false
      shift
      ;;
    --skip-gui-smoke)
      gui_smoke=false
      shift
      ;;
    --smoke-seconds)
      [[ $# -ge 2 ]] || { echo "error: --smoke-seconds needs a value" >&2; exit 2; }
      smoke_seconds="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "error: unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

[[ "${smoke_seconds}" =~ ^[1-9][0-9]*$ ]] ||
  { echo "error: --smoke-seconds must be a positive integer" >&2; exit 2; }

for tool in meson pkg-config file readelf ldd strings timeout; do
  command -v "${tool}" >/dev/null ||
    { echo "error: required tool not found: ${tool}" >&2; exit 1; }
done

if ${build}; then
  if [[ -f "${build_dir}/meson-private/coredata.dat" ]]; then
    meson setup --reconfigure "${build_dir}" "${source_dir}" \
      --buildtype=release -Dgui=enabled
  else
    meson setup "${build_dir}" "${source_dir}" \
      --buildtype=release -Dgui=enabled
  fi
  meson compile -C "${build_dir}"
  meson test -C "${build_dir}" --print-errorlogs
fi

executable="${build_dir}/chess-game"
[[ -x "${executable}" ]] ||
  { echo "error: executable not found: ${executable}" >&2; exit 1; }

file_output="$(file "${executable}")"
echo "${file_output}"
grep -q 'ELF 64-bit' <<<"${file_output}" ||
  { echo "error: expected a 64-bit ELF executable" >&2; exit 1; }

elf_machine="$(readelf -h "${executable}" |
  sed -n 's/^[[:space:]]*Machine:[[:space:]]*//p')"
case "${elf_machine}" in
  *X86-64*|*AArch64*) ;;
  *)
    echo "error: unsupported delivery architecture: ${elf_machine}" >&2
    exit 1
    ;;
esac

if missing="$(ldd "${executable}" | grep 'not found')"; then
  echo "${missing}" >&2
  echo "error: unresolved ELF shared libraries" >&2
  exit 1
fi

dynamic_entries="$(readelf -d "${executable}")"
rpaths="$(grep -E 'RPATH|RUNPATH' <<<"${dynamic_entries}" || true)"
if grep -F -e "${source_dir}" -e "${build_dir}" <<<"${rpaths}" >/dev/null; then
  echo "error: source/build path present in ELF RPATH" >&2
  exit 1
fi

strings "${executable}" |
  grep -F '/io/github/chess_game/chess.css' >/dev/null ||
  { echo "error: embedded GTK resource was not found in the ELF" >&2; exit 1; }

gtkmm_version="$(pkg-config --modversion gtkmm-4.0)"
echo "GTKmm: ${gtkmm_version}"
echo "ELF machine: ${elf_machine}"
echo "ELF dependencies: resolved"

if ${gui_smoke}; then
  smoke_log="$(mktemp "${TMPDIR:-/tmp}/chess-linux-smoke.XXXXXX")"
  trap 'rm -f "${smoke_log}"' EXIT

  command_prefix=()
  if [[ -z "${DISPLAY:-}" && -z "${WAYLAND_DISPLAY:-}" ]]; then
    if command -v xvfb-run >/dev/null; then
      command_prefix+=(xvfb-run -a)
    else
      echo "error: no graphical session and xvfb-run is unavailable" >&2
      exit 1
    fi
  fi
  if command -v dbus-run-session >/dev/null &&
    [[ -z "${DBUS_SESSION_BUS_ADDRESS:-}" ]]; then
    command_prefix=(dbus-run-session -- "${command_prefix[@]}")
  fi

  set +e
  timeout --signal=TERM "${smoke_seconds}s" \
    "${command_prefix[@]}" env G_DEBUG=fatal-criticals \
    "${executable}" >"${smoke_log}" 2>&1
  smoke_status=$?
  set -e

  if [[ ${smoke_status} -ne 124 ]]; then
    echo "error: GUI exited before the ${smoke_seconds}-second probe completed" >&2
    sed -n '1,160p' "${smoke_log}" >&2
    exit 1
  fi
  if grep -Ei '(^|[[:space:]])(error|critical):|not found|cannot open display' \
    "${smoke_log}"; then
    echo "error: GUI smoke log contains a runtime error" >&2
    exit 1
  fi
  echo "GUI startup probe: passed (${smoke_seconds} seconds)"
else
  echo "GUI startup probe: skipped"
fi

echo "Linux Release verification passed: ${executable}"
