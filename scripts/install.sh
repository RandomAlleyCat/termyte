#!/usr/bin/env bash
set -euo pipefail

# Determine project root
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV_DIR="$PROJECT_ROOT/.venv"

if [ ! -d "$VENV_DIR" ]; then
    python3 -m venv "$VENV_DIR"
fi

"$VENV_DIR/bin/pip" install -r "$PROJECT_ROOT/requirements.txt"

echo "Virtual environment created at $VENV_DIR"
echo "Activate with: source $VENV_DIR/bin/activate"
