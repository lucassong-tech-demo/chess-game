#!/bin/bash

set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "error: macOS packaging is only supported on Darwin" >&2
  exit 1
fi

script_dir="$(cd "$(dirname "$0")" && pwd)"
source_dir="$(cd "${script_dir}/.." && pwd)"
build_dir="${source_dir}/build-macos-release"
output_dir="${source_dir}/dist/macos"
build=true
create_dmg=false

usage() {
  cat <<'EOF'
Usage: scripts/package-macos.sh [options]

Build an arm64 Release binary, run its tests, create an ad-hoc-signed
Chess.app, and produce a relocatable ZIP archive.

Options:
  --build-dir PATH   Meson build directory (default: build-macos-release)
  --output-dir PATH  Artifact directory (default: dist/macos)
  --skip-build       Package an already-built chess-game executable
  --dmg              Also create Chess-<version>-macOS-arm64.dmg
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
    --dmg)
      create_dmg=true
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

for tool in meson otool install_name_tool codesign ditto sips realpath \
  plutil file lipo shasum; do
  command -v "${tool}" >/dev/null ||
    { echo "error: required tool not found: ${tool}" >&2; exit 1; }
done

case "${output_dir}" in
  "${source_dir}"/*) ;;
  *)
    echo "error: output directory must be inside the source tree: ${source_dir}" >&2
    exit 1
    ;;
esac

version="$(sed -n "s/^[[:space:]]*version: '\\([^']*\\)',/\\1/p" \
  "${source_dir}/meson.build" | head -1)"
[[ -n "${version}" ]] ||
  { echo "error: could not read project version from meson.build" >&2; exit 1; }

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

built_executable="${build_dir}/chess-game"
[[ -x "${built_executable}" ]] ||
  { echo "error: executable not found: ${built_executable}" >&2; exit 1; }

archs="$(lipo -archs "${built_executable}")"
[[ "${archs}" == "arm64" ]] ||
  { echo "error: expected an arm64-only executable, got: ${archs}" >&2; exit 1; }

minimum_macos="$(otool -l "${built_executable}" | awk '
  $1 == "cmd" && $2 == "LC_BUILD_VERSION" { in_build = 1; next }
  in_build && $1 == "minos" { print $2; exit }
')"
[[ -n "${minimum_macos}" ]] ||
  { echo "error: could not determine minimum macOS version" >&2; exit 1; }

app="${output_dir}/Chess.app"
frameworks="${app}/Contents/Frameworks"
resources="${app}/Contents/Resources"
macos="${app}/Contents/MacOS"
zip="${output_dir}/Chess-${version}-macOS-arm64.zip"
dmg="${output_dir}/Chess-${version}-macOS-arm64.dmg"

mkdir -p "${output_dir}"
rm -rf "${app}"
rm -f "${zip}" "${dmg}"
mkdir -p "${frameworks}" "${resources}" "${macos}"
ditto "${built_executable}" "${macos}/Chess"
codesign --remove-signature "${macos}/Chess" 2>/dev/null || true

cat >"${app}/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
  "https://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>en</string>
  <key>CFBundleDisplayName</key>
  <string>Chess</string>
  <key>CFBundleExecutable</key>
  <string>Chess</string>
  <key>CFBundleIconFile</key>
  <string>Chess.icns</string>
  <key>CFBundleIdentifier</key>
  <string>io.github.chess-game</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>Chess</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleShortVersionString</key>
  <string>${version}</string>
  <key>CFBundleVersion</key>
  <string>${version}</string>
  <key>LSApplicationCategoryType</key>
  <string>public.app-category.board-games</string>
  <key>LSMinimumSystemVersion</key>
  <string>${minimum_macos}</string>
  <key>NSHighResolutionCapable</key>
  <true/>
  <key>NSPrincipalClass</key>
  <string>NSApplication</string>
</dict>
</plist>
EOF
plutil -lint "${app}/Contents/Info.plist"

icon_source="${source_dir}/gui/resources/icons/app-icon.png"
sips -s format icns "${icon_source}" --out "${resources}/Chess.icns" >/dev/null

queue_file="$(mktemp "${TMPDIR:-/tmp}/chess-dylibs.XXXXXX")"
seen_file="$(mktemp "${TMPDIR:-/tmp}/chess-seen.XXXXXX")"
sources_file="$(mktemp "${TMPDIR:-/tmp}/chess-sources.XXXXXX")"
trap 'rm -f "${queue_file}" "${seen_file}" "${sources_file}"' EXIT
printf '%s\t%s\n' "${macos}/Chess" "$(realpath "${built_executable}")" \
  >"${queue_file}"
: >"${seen_file}"
: >"${sources_file}"

while IFS=$'\t' read -r macho original_macho; do
  otool -L "${macho}" | tail -n +2 | awk '{ print $1 }' |
    while IFS= read -r dependency; do
      dependency_source=""
      case "${dependency}" in
        /System/*|/usr/lib/*|@executable_path/*)
          continue
          ;;
        /*)
          dependency_source="${dependency}"
          ;;
        @rpath/*)
          basename_dep="$(basename "${dependency}")"
          if [[ -f "$(dirname "${original_macho}")/${basename_dep}" ]]; then
            dependency_source="$(dirname "${original_macho}")/${basename_dep}"
          elif [[ -f "/opt/homebrew/lib/${basename_dep}" ]]; then
            dependency_source="/opt/homebrew/lib/${basename_dep}"
          else
            echo "error: cannot resolve ${dependency} from ${original_macho}" >&2
            exit 1
          fi
          ;;
        @loader_path/*)
          relative_dependency="${dependency#@loader_path/}"
          dependency_source="$(dirname "${original_macho}")/${relative_dependency}"
          ;;
        *)
          continue
          ;;
      esac

      if [[ -n "${dependency_source}" ]]; then
          [[ -f "${dependency_source}" ]] ||
            { echo "error: missing dependency: ${dependency_source}" >&2; exit 1; }
          basename_dep="$(basename "${dependency_source}")"
          destination="${frameworks}/${basename_dep}"
          canonical_dependency="$(realpath "${dependency_source}")"
          if [[ -e "${destination}" ]]; then
            recorded_dependency="$(awk -F '\t' -v name="${basename_dep}" \
              '$1 == name { print $2; exit }' "${sources_file}")"
            [[ "${canonical_dependency}" == "${recorded_dependency}" ]] ||
              { echo "error: dylib basename collision: ${basename_dep}" >&2; exit 1; }
          else
            ditto "${dependency_source}" "${destination}"
            codesign --remove-signature "${destination}" 2>/dev/null || true
            printf '%s\t%s\n' "${basename_dep}" "${canonical_dependency}" \
              >>"${sources_file}"
          fi
          if ! grep -Fqx "${destination}" "${seen_file}"; then
            printf '%s\n' "${destination}" >>"${seen_file}"
            printf '%s\t%s\n' "${destination}" "${canonical_dependency}" \
              >>"${queue_file}"
          fi
      fi
    done
done <"${queue_file}"

rewrite_macho() {
  local macho="$1"
  local replacement_prefix="$2"
  local dependency
  local basename_dep
  local rpath

  while IFS= read -r dependency; do
    case "${dependency}" in
      /System/*|/usr/lib/*)
        ;;
      /*)
        basename_dep="$(basename "${dependency}")"
        install_name_tool -change "${dependency}" \
          "${replacement_prefix}/${basename_dep}" "${macho}"
        ;;
      @rpath/*|@loader_path/*)
        basename_dep="$(basename "${dependency}")"
        if [[ -f "${frameworks}/${basename_dep}" ]]; then
          install_name_tool -change "${dependency}" \
            "${replacement_prefix}/${basename_dep}" "${macho}"
        fi
        ;;
    esac
  done < <(otool -L "${macho}" | tail -n +2 | awk '{ print $1 }')

  while IFS= read -r rpath; do
    case "${rpath}" in
      @*) ;;
      *) install_name_tool -delete_rpath "${rpath}" "${macho}" ;;
    esac
  done < <(otool -l "${macho}" | awk '
    $1 == "cmd" && $2 == "LC_RPATH" { in_rpath = 1; next }
    in_rpath && $1 == "path" { print $2; in_rpath = 0 }
  ')
}

rewrite_macho "${macos}/Chess" "@executable_path/../Frameworks"
while IFS= read -r dylib; do
  rewrite_macho "${dylib}" "@loader_path"
  install_name_tool -id "@rpath/$(basename "${dylib}")" "${dylib}"
done <"${seen_file}"

codesign --force --sign - --timestamp=none "${frameworks}"/*.dylib
codesign --force --deep --sign - --timestamp=none "${app}"
codesign --verify --deep --strict --verbose=2 "${app}"

while IFS= read -r macho; do
  file "${macho}" | grep -q 'arm64' ||
    { echo "error: non-arm64 Mach-O: ${macho}" >&2; exit 1; }
  if otool -L "${macho}" | tail -n +2 | grep -E \
    '/opt/homebrew|/usr/local|/Cellar/|chess-game/build'; then
    echo "error: non-relocatable dependency in ${macho}" >&2
    exit 1
  fi
  if otool -l "${macho}" | tail -n +2 | grep -E \
    '/opt/homebrew|/usr/local|/Cellar/|chess-game/build'; then
    echo "error: non-relocatable RPATH in ${macho}" >&2
    exit 1
  fi
  while IFS= read -r dependency; do
    case "${dependency}" in
      /System/*|/usr/lib/*)
        ;;
      @executable_path/../Frameworks/*|@loader_path/*)
        basename_dep="$(basename "${dependency}")"
        [[ -f "${frameworks}/${basename_dep}" ]] ||
          { echo "error: unresolved bundled dependency: ${dependency}" >&2; exit 1; }
        ;;
      @rpath/*)
        [[ "${macho}" == "${frameworks}/"* &&
          "${dependency}" == "@rpath/$(basename "${macho}")" ]] ||
          { echo "error: unresolved RPATH dependency: ${dependency}" >&2; exit 1; }
        ;;
      *)
        echo "error: unexpected dependency in ${macho}: ${dependency}" >&2
        exit 1
        ;;
    esac
  done < <(otool -L "${macho}" | tail -n +2 | awk '{ print $1 }')
done < <(find "${app}/Contents/MacOS" "${frameworks}" -type f -print)

ditto -c -k --sequesterRsrc --keepParent "${app}" "${zip}"

if ${create_dmg}; then
  command -v hdiutil >/dev/null ||
    { echo "error: hdiutil is required for --dmg" >&2; exit 1; }
  dmg_root="$(mktemp -d "${TMPDIR:-/tmp}/chess-dmg.XXXXXX")"
  ditto "${app}" "${dmg_root}/Chess.app"
  ln -s /Applications "${dmg_root}/Applications"
  if ! hdiutil create -quiet -fs HFS+ -volname "Chess ${version}" \
    -srcfolder "${dmg_root}" -format UDZO -ov "${dmg}"; then
    rm -rf "${dmg_root}"
    exit 1
  fi
  rm -rf "${dmg_root}"
fi

echo
echo "Created:"
echo "  ${app}"
echo "  ${zip}"
if ${create_dmg}; then
  echo "  ${dmg}"
fi
echo
(
  cd "${output_dir}"
  find Chess.app -type f -print | LC_ALL=C sort |
    while IFS= read -r bundled_file; do
      shasum -a 256 "${bundled_file}"
    done |
    shasum -a 256 |
    sed 's/  -$/  Chess.app (file-tree)/'
)
shasum -a 256 "${zip}"
if ${create_dmg}; then
  shasum -a 256 "${dmg}"
fi
