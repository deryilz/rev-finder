#!/bin/bash

set -e

# ==============================================================================
# Configuration
# ==============================================================================
PATCH_FILE="modifications.patch"
CLEAN_SUBMODULE="cubiomes"
TEMP_DIR="cubiomes-patch-temp"
PATCH_DIR="cubiomes-patch"

# ==============================================================================
# 1. Validation
# ==============================================================================
if [ ! -d "$CLEAN_SUBMODULE" ]; then
    echo "Error: Source directory '$CLEAN_SUBMODULE' does not exist." >&2
    exit 1
fi

if [ -d "$TEMP_DIR" ]; then
    rm -rf "$TEMP_DIR"
fi

cleanup() {
    if [ -d "$TEMP_DIR" ]; then
        echo ""
        echo "||CLEANING UP||" >&2
        rm -rf "$TEMP_DIR"
        echo "Removed temporary directory '$TEMP_DIR'." >&2
    fi
}
trap cleanup EXIT

# ==============================================================================
# 2. Workspace Setup & Baseline Git Init
# ==============================================================================
echo "||SETTING UP PATCH WORKSPACE||" >&2

# Ensure the submodule directory exists and contains files; populate if empty
if [ ! -d "$CLEAN_SUBMODULE" ] || [ -z "$(ls -A "$CLEAN_SUBMODULE")" ]; then
    echo "Submodule '$CLEAN_SUBMODULE' is empty/missing. Initializing..." >&2
    git submodule update --init --recursive "$CLEAN_SUBMODULE"
fi

# Copy clean upstream files to the temporary folder
cp -r "$CLEAN_SUBMODULE" "$TEMP_DIR"

# Remove any nested .git folder/file inside the copy so local git tracks raw files
rm -rf "$TEMP_DIR/.git"

# Initialize git repository in temp workspace
git -C "$TEMP_DIR" init > /dev/null
git -C "$TEMP_DIR" config user.name "Patch Utility"
git -C "$TEMP_DIR" config user.email "patch@local"

# Add files and use --allow-empty so commit never fails
git -C "$TEMP_DIR" add -A > /dev/null
git -C "$TEMP_DIR" commit --allow-empty -m "upstream baseline" > /dev/null

# Tell Git to ignore patch artifact files (.rej and .orig)
echo "*.rej" >> "$TEMP_DIR/.gitignore"
echo "*.orig" >> "$TEMP_DIR/.gitignore"

# ==============================================================================
# 3. Attempt Patch Application
# ==============================================================================
if [ -f "$PATCH_FILE" ]; then
    echo "Applying existing patch ($PATCH_FILE)..." >&2

    set +e
    patch -p1 -d "$TEMP_DIR" < "$PATCH_FILE"
    PATCH_EXIT_CODE=$?
    set -e

    if [ $PATCH_EXIT_CODE -ne 0 ]; then
        echo "" >&2
        echo "⚠️  WARNING: Patch did not apply cleanly!" >&2
        echo "Rejection logs (*.rej files) have been left in '$TEMP_DIR' for you to review." >&2
    else
        echo "Patch applied cleanly." >&2
    fi
else
    echo "No existing patch file found ($PATCH_FILE). Starting with clean source." >&2
fi

# ==============================================================================
# 4. Interactive Pause
# ==============================================================================
echo ""
echo "------------------------------------------------------------------------"
echo "Workspace ready in: ./$TEMP_DIR/"
echo "Make your changes or fix any patch conflicts directly inside that folder."
echo "Press [ENTER] when you are finished editing to generate the new patch."
echo "------------------------------------------------------------------------"
read -r -p "Ready? "

# ==============================================================================
# 5. Diff Generation
# ==============================================================================
echo ""
echo "||GENERATING NEW PATCH||" >&2

# Compare working directory directly against initial baseline HEAD
# Output diff to a temporary patch file first to protect existing patch if empty
NEW_PATCH=$(git -C "$TEMP_DIR" diff HEAD -- . ":(exclude).gitignore")

if [ -n "$NEW_PATCH" ]; then
    echo "$NEW_PATCH" > "$PATCH_FILE"
    echo "Successfully updated $PATCH_FILE!"
else
    echo "⚠️  No changes detected in $TEMP_DIR. $PATCH_FILE was left untouched."
fi

rm -rf "$PATCH_DIR"
echo "Deleting $PATCH_DIR so you can recompile!"
