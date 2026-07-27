#!/bin/bash

set -euo pipefail

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "error: Debian packaging is only supported on Linux" >&2
  exit 1
fi

script_dir="$(cd "$(dirname "$0")" && pwd)"
source_dir="$(cd "${script_dir}/.." && pwd)"
build_dir="${source_dir}/build-linux-release"
output_dir="${source_dir}/dist/linux"
build=true

usage() {
  cat <<'EOF'
Usage: scripts/package-linux-deb.sh [options]

Build, test, verify, and package chess-game as a local Debian/Ubuntu package.

Options:
  --build-dir PATH   Meson build directory (default: build-linux-release)
  --output-dir PATH  Artifact directory (default: dist/linux)
  --skip-build       Package an already-built, previously tested executable
  -h, --help         Show this help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)
      [[ $# -ge 2 ]] || { echo "error: --build-dir needs a path" >&2; exit 2; }
      build_dir="$2"
      shift 2
      ;;
    --output-dir)
      [[ $# -ge 2 ]] || { echo "error: --output-dir needs a path" >&2; exit 2; }
      output_dir="$2"
      shift 2
      ;;
    --skip-build)
      build=false
      shift
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

for tool in desktop-file-validate dpkg dpkg-deb dpkg-shlibdeps install \
  realpath sha256sum strip; do
  command -v "${tool}" >/dev/null ||
    { echo "error: required tool not found: ${tool}" >&2; exit 1; }
done

case "${build_dir}" in
  /*) ;;
  *) build_dir="${source_dir}/${build_dir}" ;;
esac
build_dir="$(realpath -m -- "${build_dir}")"

case "${output_dir}" in
  /*) ;;
  *) output_dir="${source_dir}/${output_dir}" ;;
esac
output_dir="$(realpath -m -- "${output_dir}")"

case "${output_dir}" in
  "${source_dir}"/*) ;;
  *)
    echo "error: output directory must be inside the source tree: ${source_dir}" >&2
    exit 1
    ;;
esac

verify_args=(--build-dir "${build_dir}" --skip-gui-smoke)
if ! ${build}; then
  verify_args+=(--skip-build)
fi
"${script_dir}/verify-linux.sh" "${verify_args[@]}"

version="$(sed -n "s/^[[:space:]]*version: '\\([^']*\\)',/\\1/p" \
  "${source_dir}/meson.build" | head -1)"
[[ -n "${version}" ]] ||
  { echo "error: could not read project version from meson.build" >&2; exit 1; }

architecture="$(dpkg --print-architecture)"
case "${architecture}" in
  amd64|arm64) ;;
  *)
    echo "error: unsupported Debian delivery architecture: ${architecture}" >&2
    exit 1
    ;;
esac

package_version="${version}-1"
package_name="chess-game"
deb="${output_dir}/${package_name}_${package_version}_${architecture}.deb"
staging="$(mktemp -d "${TMPDIR:-/tmp}/chess-deb.XXXXXX")"
extracted="$(mktemp -d "${TMPDIR:-/tmp}/chess-deb-check.XXXXXX")"
shlib_work="$(mktemp -d "${TMPDIR:-/tmp}/chess-shlibdeps.XXXXXX")"
cleanup() {
  rm -rf "${staging}" "${extracted}" "${shlib_work}"
}
trap cleanup EXIT

install -Dm755 "${build_dir}/chess-game" "${staging}/usr/bin/chess-game"
strip "${staging}/usr/bin/chess-game"
install -Dm644 "${source_dir}/view/images/wking.png" \
  "${staging}/usr/share/icons/hicolor/128x128/apps/io.github.chess_game.png"
install -Dm644 "${source_dir}/LICENSE" \
  "${staging}/usr/share/doc/${package_name}/copyright"

desktop_file="${staging}/usr/share/applications/io.github.chess_game.desktop"
mkdir -p "$(dirname "${desktop_file}")"
cat >"${desktop_file}" <<'EOF'
[Desktop Entry]
Type=Application
Name=Chess
Comment=GTKmm chess game
Exec=chess-game
Icon=io.github.chess_game
Terminal=false
Categories=Game;BoardGame;
StartupNotify=true
EOF
desktop-file-validate "${desktop_file}"

mkdir -p "${shlib_work}/debian"
cat >"${shlib_work}/debian/control" <<EOF
Source: ${package_name}
Section: games
Priority: optional
Maintainer: Chess Game contributors <noreply@example.com>
Standards-Version: 4.7.0

Package: ${package_name}
Architecture: any
Description: GTKmm chess game
 A portable C++20 chess game with a GTKmm 4 desktop interface.
EOF
shlib_output="$(
  cd "${shlib_work}"
  dpkg-shlibdeps -O -e"${staging}/usr/bin/chess-game"
)"
dependencies="$(sed -n 's/^shlibs:Depends=//p' <<<"${shlib_output}")"
[[ -n "${dependencies}" ]] ||
  { echo "error: dpkg-shlibdeps produced no runtime dependencies" >&2; exit 1; }

installed_size="$(du -sk "${staging}/usr" | awk '{ print $1 }')"
mkdir -p "${staging}/DEBIAN"
cat >"${staging}/DEBIAN/control" <<EOF
Package: ${package_name}
Version: ${package_version}
Section: games
Priority: optional
Architecture: ${architecture}
Depends: ${dependencies}
Installed-Size: ${installed_size}
Maintainer: Chess Game contributors <noreply@example.com>
Description: GTKmm chess game
 A portable C++20 chess game with a GTKmm 4 desktop interface.
EOF

mkdir -p "${output_dir}"
rm -f "${deb}"
dpkg-deb --root-owner-group --build "${staging}" "${deb}"
dpkg-deb --info "${deb}"
dpkg-deb --contents "${deb}"
dpkg-deb --extract "${deb}" "${extracted}"

file "${extracted}/usr/bin/chess-game" | grep -q 'ELF 64-bit' ||
  { echo "error: packaged executable is not 64-bit ELF" >&2; exit 1; }
if missing="$(ldd "${extracted}/usr/bin/chess-game" | grep 'not found')"; then
  echo "${missing}" >&2
  echo "error: packaged executable has unresolved libraries" >&2
  exit 1
fi

sha256sum "${deb}"
echo "Created Debian package: ${deb}"
