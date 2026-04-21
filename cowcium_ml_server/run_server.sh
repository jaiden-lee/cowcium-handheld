#!/usr/bin/env bash
set -euo pipefail

uvicorn app:app --host 0.0.0.0 --port 8090
