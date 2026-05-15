#!/usr/bin/env bash
set -euo pipefail

root="${1:-${GITHUB_WORKSPACE:-$(pwd)}}"
build_dir="${2:-$root/build-linux}"
dist_dir="${3:-$root/dist}"
package_name="${4:-kainote-linux-x86_64}"
stage_dir="$dist_dir/$package_name"
archive="$dist_dir/$package_name.tar.xz"

if [[ ! -x "$build_dir/kainote" ]]; then
  echo "error: missing executable: $build_dir/kainote" >&2
  exit 1
fi

rm -rf "$stage_dir" "$archive"
mkdir -p "$stage_dir"

install -m 0755 "$build_dir/kainote" "$stage_dir/kainote"

shopt -s nullglob
for lib in "$build_dir"/lib*.so*; do
  cp -a "$lib" "$stage_dir/"
done

for dir in Kainote Locale pulseaudio Csri; do
  if [[ -d "$build_dir/$dir" ]]; then
    cp -a "$build_dir/$dir" "$stage_dir/"
  fi
done

for file in README.md LICENSE LICENSE.txt; do
  if [[ -f "$root/$file" ]]; then
    cp -a "$root/$file" "$stage_dir/"
  fi
done

# Do not package mutable first-run state such as Config/, Catalog/, AudioCache/,
# Indices/, or Subs/. Those are generated beside the executable by Kainote.
find "$stage_dir" -type f -printf '%P\t%s\n' | sort > "$stage_dir/artifact-manifest.txt"
tar -C "$dist_dir" -cJf "$archive" "$package_name"

if command -v du >/dev/null 2>&1; then
  du -h "$archive"
fi

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  {
    echo "archive=$archive"
    echo "package_name=$package_name"
  } >> "$GITHUB_OUTPUT"
fi
