#!/bin/bash

set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "error: the LZO-free Cairo build is only supported on macOS" >&2
  exit 1
fi

script_dir="$(cd "$(dirname "$0")" && pwd)"
source_dir="$(cd "${script_dir}/.." && pwd)"
build_root="${source_dir}/build-macos-cairo-no-lzo"
cairo_version="1.18.4"
cairo_sha256="445ed8208a6e4823de1226a74ca319d3600e83f6369f99b14265006599c32ccb"
source_archive=""

usage() {
  cat <<'EOF'
Usage: scripts/build-macos-cairo-no-lzo.sh [options]

Build the three Cairo dylibs used by the macOS bundle from the official Cairo
1.18.4 source with LZO explicitly disabled. Nothing is installed system-wide.

Options:
  --source-archive PATH  Use an existing cairo-1.18.4.tar.xz archive
  -h, --help             Show this help

Without --source-archive, the script uses Homebrew's already-downloaded Cairo
source cache. It does not fetch or install software.
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --source-archive)
      [[ $# -ge 2 ]] ||
        { echo "error: --source-archive needs a path" >&2; exit 2; }
      source_archive="$2"
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

for tool in brew meson ninja pkg-config tar shasum otool nm file ditto; do
  command -v "${tool}" >/dev/null ||
    { echo "error: required tool not found: ${tool}" >&2; exit 1; }
done

if [[ -z "${source_archive}" ]]; then
  source_archive="$(brew --cache --build-from-source cairo)"
elif [[ "${source_archive}" != /* ]]; then
  source_archive="${source_dir}/${source_archive}"
fi

[[ -f "${source_archive}" ]] || {
  echo "error: Cairo source archive is not available: ${source_archive}" >&2
  echo "Provide the audited cairo-${cairo_version}.tar.xz with --source-archive." >&2
  exit 1
}

actual_sha256="$(shasum -a 256 "${source_archive}" | awk '{ print $1 }')"
[[ "${actual_sha256}" == "${cairo_sha256}" ]] || {
  echo "error: unexpected Cairo source SHA-256" >&2
  echo "expected: ${cairo_sha256}" >&2
  echo "actual:   ${actual_sha256}" >&2
  exit 1
}

source_tree="${build_root}/cairo-${cairo_version}"
meson_build="${build_root}/meson"
overrides="${build_root}/overrides"

rm -rf "${source_tree}" "${meson_build}" "${overrides}"
mkdir -p "${build_root}" "${overrides}"
tar -xf "${source_archive}" -C "${build_root}"
[[ -f "${source_tree}/meson.build" ]] ||
  { echo "error: Cairo source tree was not extracted as expected" >&2; exit 1; }

meson setup "${meson_build}" "${source_tree}" \
  --buildtype=release \
  --default-library=shared \
  -Dfontconfig=enabled \
  -Dfreetype=enabled \
  -Dpng=enabled \
  -Dglib=enabled \
  -Dxcb=enabled \
  -Dxlib=enabled \
  -Dzlib=enabled \
  -Dquartz=enabled \
  -Dlzo=disabled \
  -Dtests=disabled \
  -Dgtk_doc=false
meson compile -C "${meson_build}"

ditto "${meson_build}/src/libcairo.2.dylib" \
  "${overrides}/libcairo.2.dylib"
ditto "${meson_build}/util/cairo-gobject/libcairo-gobject.2.dylib" \
  "${overrides}/libcairo-gobject.2.dylib"
ditto \
  "${meson_build}/util/cairo-script/libcairo-script-interpreter.2.dylib" \
  "${overrides}/libcairo-script-interpreter.2.dylib"

for dylib in "${overrides}"/*.dylib; do
  file "${dylib}" | grep -q 'arm64' ||
    { echo "error: non-arm64 Cairo override: ${dylib}" >&2; exit 1; }
  if otool -L "${dylib}" | grep -qi 'liblzo'; then
    echo "error: LZO remains in Cairo override: ${dylib}" >&2
    exit 1
  fi
done

exports_file="$(mktemp "${TMPDIR:-/tmp}/chess-cairo-exports.XXXXXX")"
trap 'rm -f "${exports_file}"' EXIT
nm -gU "${overrides}"/*.dylib >"${exports_file}"
for symbol in \
  _cairo_script_create_for_stream \
  _cairo_script_from_recording_surface \
  _cairo_script_interpreter_create \
  _cairo_script_interpreter_destroy \
  _cairo_script_interpreter_feed_string \
  _cairo_script_interpreter_install_hooks; do
  grep -Fq " ${symbol}" "${exports_file}" ||
    { echo "error: required GTK symbol is missing: ${symbol}" >&2; exit 1; }
done

echo
echo "Created LZO-free Cairo overrides:"
for dylib in "${overrides}"/*.dylib; do
  echo "  ${dylib}"
done
echo "Cairo source SHA-256: ${actual_sha256}"
