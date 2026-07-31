#!/bin/bash

set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "error: the LGPL relink helper is only supported on macOS" >&2
  exit 1
fi

script_dir="$(cd "$(dirname "$0")" && pwd)"
entitlements="${script_dir}/lgpl-local-modification.entitlements"
source_app=""
output_app=""
replacements=()
output_created=false

usage() {
  cat <<'EOF'
Usage: scripts/relink-macos-lgpl.sh --app PATH --output-app PATH \
  --replace PATH [--replace PATH ...]

Create a separate, locally modified copy of Chess.app, replace one or more
interface-compatible bundled dylibs, and ad-hoc sign the copy with Hardened
Runtime plus Disable Library Validation.

The source App is never modified and the output path must not already exist.
The resulting copy is not Developer ID signed, notarized, or suitable for
redistribution as an official Chess release.
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --app)
      [[ $# -ge 2 ]] || { echo "error: --app needs a path" >&2; exit 2; }
      source_app="$2"
      shift 2
      ;;
    --output-app)
      [[ $# -ge 2 ]] ||
        { echo "error: --output-app needs a path" >&2; exit 2; }
      output_app="$2"
      shift 2
      ;;
    --replace)
      [[ $# -ge 2 ]] ||
        { echo "error: --replace needs a dylib path" >&2; exit 2; }
      replacements+=("$2")
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

[[ -n "${source_app}" && -n "${output_app}" ]] ||
  { echo "error: --app and --output-app are required" >&2; exit 2; }
[[ "${#replacements[@]}" -gt 0 ]] ||
  { echo "error: at least one --replace is required" >&2; exit 2; }

for tool in codesign ditto file install_name_tool lipo otool plutil realpath; do
  command -v "${tool}" >/dev/null ||
    { echo "error: required tool not found: ${tool}" >&2; exit 1; }
done

[[ -f "${entitlements}" ]] ||
  { echo "error: local modification entitlements not found" >&2; exit 1; }
plutil -lint "${entitlements}" >/dev/null

if [[ "${source_app}" != /* ]]; then
  source_app="$(realpath "${source_app}")"
fi
[[ -d "${source_app}/Contents/Frameworks" ]] ||
  { echo "error: Chess.app Frameworks directory not found" >&2; exit 1; }

if [[ "${output_app}" != /* ]]; then
  output_parent="$(realpath "$(dirname "${output_app}")")"
  output_app="${output_parent}/$(basename "${output_app}")"
else
  output_parent="$(realpath "$(dirname "${output_app}")")"
  output_app="${output_parent}/$(basename "${output_app}")"
fi

[[ "${source_app}" != "${output_app}" ]] ||
  { echo "error: output App must differ from source App" >&2; exit 1; }
[[ ! -e "${output_app}" ]] ||
  { echo "error: output path already exists: ${output_app}" >&2; exit 1; }

cleanup_on_failure() {
  status=$?
  if [[ "${status}" -ne 0 && "${output_created}" == true ]]; then
    rm -rf "${output_app}"
  fi
  exit "${status}"
}
trap cleanup_on_failure EXIT

ditto "${source_app}" "${output_app}"
output_created=true
frameworks="${output_app}/Contents/Frameworks"

for replacement in "${replacements[@]}"; do
  if [[ "${replacement}" != /* ]]; then
    replacement="$(realpath "${replacement}")"
  fi
  [[ -f "${replacement}" ]] ||
    { echo "error: replacement dylib not found: ${replacement}" >&2; exit 1; }
  file "${replacement}" | grep -q 'Mach-O.*dynamically linked shared library' ||
    { echo "error: replacement is not a Mach-O dylib: ${replacement}" >&2; exit 1; }
  [[ "$(lipo -archs "${replacement}")" == "arm64" ]] ||
    { echo "error: replacement must be arm64-only: ${replacement}" >&2; exit 1; }

  dylib_name="$(basename "${replacement}")"
  destination="${frameworks}/${dylib_name}"
  [[ -f "${destination}" ]] ||
    { echo "error: App does not bundle ${dylib_name}" >&2; exit 1; }
  ditto "${replacement}" "${destination}"
  codesign --remove-signature "${destination}" 2>/dev/null || true
  install_name_tool -id "@rpath/${dylib_name}" "${destination}"

  while IFS= read -r dependency; do
    dependency_name="$(basename "${dependency}")"
    case "${dependency}" in
      /System/*|/usr/lib/*)
        ;;
      @loader_path/*|@rpath/*)
        if [[ "${dependency_name}" != "${dylib_name}" &&
          ! -f "${frameworks}/${dependency_name}" ]]; then
          echo "error: replacement dependency is not bundled: ${dependency}" >&2
          exit 1
        fi
        if [[ "${dependency_name}" != "${dylib_name}" ]]; then
          install_name_tool -change "${dependency}" \
            "@loader_path/${dependency_name}" "${destination}"
        fi
        ;;
      /*)
        [[ -f "${frameworks}/${dependency_name}" ]] || {
          echo "error: replacement has an external dependency: ${dependency}" >&2
          exit 1
        }
        install_name_tool -change "${dependency}" \
          "@loader_path/${dependency_name}" "${destination}"
        ;;
      *)
        echo "error: unsupported replacement dependency: ${dependency}" >&2
        exit 1
        ;;
    esac
  done < <(otool -L "${destination}" | tail -n +2 | awk '{ print $1 }')
done

for dylib in "${frameworks}"/*.dylib; do
  codesign --force --sign - --options runtime --timestamp=none "${dylib}"
done
codesign --force --sign - --options runtime --timestamp=none \
  --entitlements "${entitlements}" "${output_app}"
codesign --verify --deep --strict --verbose=2 "${output_app}"

embedded_entitlements="$(codesign -d --entitlements :- "${output_app}" 2>/dev/null)"
printf '%s\n' "${embedded_entitlements}" |
  grep -q 'com.apple.security.cs.disable-library-validation' || {
    echo "error: local modification entitlement was not embedded" >&2
    exit 1
  }

output_created=false
echo
echo "Created local LGPL test copy:"
echo "  ${output_app}"
echo
echo "This copy is ad-hoc signed for local testing only."
echo "It is not Developer ID signed, notarized, or an official release artifact."
