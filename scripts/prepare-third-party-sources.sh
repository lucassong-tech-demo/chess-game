#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "$0")" && pwd)"
source_dir="$(cd "${script_dir}/.." && pwd)"
manifest="${source_dir}/third_party/macos-arm64-v0.1.0-sources.tsv"
output_dir="${source_dir}/dist"
app=""
verify_only=false

usage() {
  cat <<'EOF'
Usage: scripts/prepare-third-party-sources.sh [options]

Validate the v0.1.0 macOS source manifest and optionally download every exact
upstream source archive and Homebrew formula snapshot into a release archive.
This script downloads files but never installs software.

Options:
  --app PATH                 Require exact dylib coverage for this Chess.app
  --output-dir PATH          Archive directory (default: dist)
  --verify-manifest-only     Validate metadata/coverage without downloading
  -h, --help                 Show this help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --app)
      [[ $# -ge 2 ]] ||
        { echo "error: --app needs a path" >&2; exit 2; }
      app="$2"
      shift 2
      ;;
    --output-dir)
      [[ $# -ge 2 ]] ||
        { echo "error: --output-dir needs a path" >&2; exit 2; }
      output_dir="$2"
      shift 2
      ;;
    --verify-manifest-only)
      verify_only=true
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

[[ -f "${manifest}" ]] ||
  { echo "error: source manifest not found: ${manifest}" >&2; exit 1; }

if [[ -n "${app}" && "${app}" != /* ]]; then
  app="${source_dir}/${app}"
fi
if [[ "${output_dir}" != /* ]]; then
  output_dir="${source_dir}/${output_dir}"
fi

case "${output_dir}" in
  "${source_dir}"/*) ;;
  *)
    echo "error: output directory must be inside the source tree" >&2
    exit 1
    ;;
esac

for tool in awk basename comm find mktemp shasum sort; do
  command -v "${tool}" >/dev/null ||
    { echo "error: required tool not found: ${tool}" >&2; exit 1; }
done

homebrew_commit="$(sed -n 's/^# homebrew_core_commit=//p' "${manifest}")"
[[ "${homebrew_commit}" =~ ^[0-9a-f]{40}$ ]] ||
  { echo "error: invalid Homebrew/core commit in manifest" >&2; exit 1; }

components_file="$(mktemp "${TMPDIR:-/tmp}/chess-components.XXXXXX")"
manifest_dylibs="$(mktemp "${TMPDIR:-/tmp}/chess-manifest-dylibs.XXXXXX")"
app_dylibs="$(mktemp "${TMPDIR:-/tmp}/chess-app-dylibs.XXXXXX")"
work_dir=""
archive_tmp=""
cleanup() {
  rm -f "${components_file}" "${manifest_dylibs}" "${app_dylibs}"
  [[ -z "${work_dir}" ]] || rm -rf "${work_dir}"
  [[ -z "${archive_tmp}" ]] || rm -f "${archive_tmp}"
}
trap cleanup EXIT

row_count=0
while IFS=$'\t' read -r component package_version upstream_version \
  build_origin bundled_dylibs distributed_license source_url source_sha256 \
  bottle_sha256 formula_path formula_sha256 scope_note extra; do
  [[ -n "${component}" ]] || continue
  [[ "${component}" == \#* ]] && continue
  [[ "${component}" == "component" ]] && continue
  [[ -z "${extra:-}" ]] ||
    { echo "error: too many columns for ${component}" >&2; exit 1; }
  [[ -n "${scope_note}" ]] ||
    { echo "error: missing scope note for ${component}" >&2; exit 1; }
  [[ "${source_url}" == https://* ]] ||
    { echo "error: non-HTTPS source URL for ${component}" >&2; exit 1; }
  [[ "${source_sha256}" =~ ^[0-9a-f]{64}$ ]] ||
    { echo "error: invalid source SHA-256 for ${component}" >&2; exit 1; }
  [[ "${formula_sha256}" =~ ^[0-9a-f]{64}$ ]] ||
    { echo "error: invalid formula SHA-256 for ${component}" >&2; exit 1; }
  if [[ "${bottle_sha256}" != "not-used" ]]; then
    [[ "${bottle_sha256}" =~ ^[0-9a-f]{64}$ ]] ||
      { echo "error: invalid bottle SHA-256 for ${component}" >&2; exit 1; }
  fi
  printf '%s\n' "${component}" >>"${components_file}"
  IFS=',' read -r -a dylibs <<<"${bundled_dylibs}"
  for dylib in "${dylibs[@]}"; do
    [[ "${dylib}" == *.dylib ]] ||
      { echo "error: invalid dylib name for ${component}: ${dylib}" >&2; exit 1; }
    printf '%s\n' "${dylib}" >>"${manifest_dylibs}"
  done
  row_count=$((row_count + 1))
done <"${manifest}"

[[ "${row_count}" -eq 34 ]] ||
  { echo "error: expected 34 source rows, got ${row_count}" >&2; exit 1; }
duplicate_component="$(sort "${components_file}" | uniq -d | head -1)"
[[ -z "${duplicate_component}" ]] ||
  { echo "error: duplicate component: ${duplicate_component}" >&2; exit 1; }
duplicate_dylib="$(sort "${manifest_dylibs}" | uniq -d | head -1)"
[[ -z "${duplicate_dylib}" ]] ||
  { echo "error: duplicate dylib mapping: ${duplicate_dylib}" >&2; exit 1; }

sort -o "${manifest_dylibs}" "${manifest_dylibs}"
[[ "$(wc -l <"${manifest_dylibs}" | tr -d ' ')" -eq 46 ]] ||
  { echo "error: source manifest must map exactly 46 dylibs" >&2; exit 1; }

if [[ -n "${app}" ]]; then
  frameworks="${app}/Contents/Frameworks"
  [[ -d "${frameworks}" ]] ||
    { echo "error: App Frameworks directory not found: ${frameworks}" >&2; exit 1; }
  find "${frameworks}" -maxdepth 1 -type f -name '*.dylib' -exec basename {} \; |
    sort >"${app_dylibs}"
  coverage_difference="$(comm -3 "${manifest_dylibs}" "${app_dylibs}")"
  [[ -z "${coverage_difference}" ]] || {
    echo "error: source manifest and App dylib closure differ:" >&2
    printf '%s\n' "${coverage_difference}" >&2
    exit 1
  }
fi

echo "Manifest valid: ${row_count} components, 46 bundled dylibs"
[[ -z "${app}" ]] || echo "App dylib coverage: exact"

if ${verify_only}; then
  exit 0
fi

for tool in curl tar; do
  command -v "${tool}" >/dev/null ||
    { echo "error: required tool not found: ${tool}" >&2; exit 1; }
done

mkdir -p "${output_dir}"
archive="${output_dir}/Chess-0.1.0-third-party-sources.tar.gz"
[[ ! -e "${archive}" ]] ||
  { echo "error: archive already exists: ${archive}" >&2; exit 1; }

work_dir="$(mktemp -d "${output_dir}/.third-party-sources.XXXXXX")"
payload="${work_dir}/Chess-0.1.0-third-party-sources"
mkdir -p "${payload}/sources" "${payload}/homebrew-core" \
  "${payload}/project-build"

download_and_verify() {
  local url="$1"
  local destination="$2"
  local expected_sha256="$3"
  local actual_sha256

  echo "Downloading $(basename "${destination}")"
  curl --fail --location --retry 3 --silent --show-error \
    "${url}" --output "${destination}"
  actual_sha256="$(shasum -a 256 "${destination}" | awk '{ print $1 }')"
  [[ "${actual_sha256}" == "${expected_sha256}" ]] || {
    echo "error: SHA-256 mismatch for ${destination}" >&2
    echo "expected: ${expected_sha256}" >&2
    echo "actual:   ${actual_sha256}" >&2
    exit 1
  }
}

while IFS=$'\t' read -r component package_version upstream_version \
  build_origin bundled_dylibs distributed_license source_url source_sha256 \
  bottle_sha256 formula_path formula_sha256 scope_note extra; do
  [[ -n "${component}" ]] || continue
  [[ "${component}" == \#* ]] && continue
  [[ "${component}" == "component" ]] && continue

  source_basename="$(basename "${source_url%%\\?*}")"
  source_file="${payload}/sources/${component}-${source_basename}"
  download_and_verify "${source_url}" "${source_file}" "${source_sha256}"

  formula_file="${payload}/homebrew-core/${formula_path}"
  mkdir -p "$(dirname "${formula_file}")"
  formula_url="https://raw.githubusercontent.com/Homebrew/homebrew-core/${homebrew_commit}/${formula_path}"
  download_and_verify "${formula_url}" "${formula_file}" "${formula_sha256}"
done <"${manifest}"

cp "${manifest}" "${payload}/SOURCES.tsv"
cp "${source_dir}/THIRD_PARTY_NOTICES.md" "${payload}/THIRD_PARTY_NOTICES.md"
cp "${source_dir}/docs/lgpl-relinking.md" "${payload}/LGPL_RELINKING.md"
cp "${source_dir}/scripts/build-macos-cairo-no-lzo.sh" \
  "${payload}/project-build/"
cp "${source_dir}/scripts/relink-macos-lgpl.sh" "${payload}/project-build/"
cp "${source_dir}/scripts/lgpl-local-modification.entitlements" \
  "${payload}/project-build/"

{
  echo "Chess 0.1.0 third-party corresponding sources"
  echo
  echo "SOURCES.tsv records the exact source and binary provenance."
  echo "sources/ contains verified upstream archives."
  echo "homebrew-core/ contains formula snapshots from ${homebrew_commit}."
  echo "project-build/ contains the LZO-free Cairo and local relinking tools."
  echo
  echo "This archive is source/compliance material; it does not contain secrets."
} >"${payload}/README.txt"

(
  cd "${payload}"
  find . -type f ! -name SHA256SUMS -print | sort |
    while IFS= read -r file; do
      shasum -a 256 "${file}"
    done >SHA256SUMS
)

archive_tmp="${archive}.tmp"
tar -czf "${archive_tmp}" -C "${work_dir}" \
  "Chess-0.1.0-third-party-sources"
mv "${archive_tmp}" "${archive}"
archive_tmp=""

echo "Created: ${archive}"
shasum -a 256 "${archive}"
