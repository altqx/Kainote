#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PATCHES_DIR="$SCRIPT_DIR/patches"

declare -A PATCHED_SUBMODULES=(
    ["Thirdparty/ffms2"]="ffms2-kainote.patch"
    ["Thirdparty/xy-VSFilter-xy_sub_filter_rc5"]="xy-vsfilter-kainote.patch"
    ["Thirdparty/wxWidgets"]="wxwidgets-kainote.patch"
    ["Thirdparty/Hunspell"]="hunspell-kainote.patch"
)

mkdir -p "$PATCHES_DIR"

for submodule in "${!PATCHED_SUBMODULES[@]}"; do
    patch_file="${PATCHED_SUBMODULES[$submodule]}"
    submodule_path="$REPO_ROOT/$submodule"

    if [ ! -d "$submodule_path/.git" ] && [ ! -f "$submodule_path/.git" ]; then
        echo "SKIP $submodule (not initialized)"
        continue
    fi

    echo -n "Updating $patch_file ... "

    (
        cd "$submodule_path"
        git add -A
        git diff --cached --binary --no-color > "$PATCHES_DIR/$patch_file"
        git reset HEAD --quiet
    )

    lines=$(wc -l < "$PATCHES_DIR/$patch_file")
    size=$(du -h "$PATCHES_DIR/$patch_file" | cut -f1)
    echo "done ($lines lines, $size)"
done

echo ""
echo "All patches updated in $PATCHES_DIR/"
