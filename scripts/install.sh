#!/usr/bin/env bash
set -euo pipefail

if ! command -v python >/dev/null 2>&1; then
    echo "Error: python is not installed." >&2
    exit 1
fi

if ! command -v pip >/dev/null 2>&1; then
    echo "Error: pip is not installed." >&2
    exit 1
fi

# Determine project root
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV_DIR="$PROJECT_ROOT/.venv"

if [ ! -d "$VENV_DIR" ]; then
    python -m venv "$VENV_DIR"
fi

"$VENV_DIR/bin/python" -m pip install --upgrade pip

REQ_FILE="$PROJECT_ROOT/requirements.txt"
if [ "$(uname)" != "Linux" ]; then
    TMP_REQ="$(mktemp)"
    grep -v '^evdev' "$REQ_FILE" > "$TMP_REQ"
    REQ_FILE="$TMP_REQ"
fi

"$VENV_DIR/bin/python" -m pip install -r "$REQ_FILE"

if [ -n "${TMP_REQ:-}" ]; then
    rm -f "$TMP_REQ"
fi

echo "Virtual environment created at $VENV_DIR"
echo "Activate with: source $VENV_DIR/bin/activate"
