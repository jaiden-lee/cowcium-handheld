#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ ! -d "${SCRIPT_DIR}/.venv" ]; then
  python3 -m venv "${SCRIPT_DIR}/.venv"
fi

cd "${SCRIPT_DIR}"

# shellcheck disable=SC1091
source "${SCRIPT_DIR}/.venv/bin/activate"

uvicorn app:app --host 0.0.0.0 --port 8090
