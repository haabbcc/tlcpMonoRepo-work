#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

echo "==> [1/2] build tlcpMonoRepo base stack"
bash "${ROOT}/build-all.sh"

echo "==> [2/2] build Angie with Tongsuo NTLS"
bash "${ROOT}/scripts/Angie_TLCP_PQC/build-angie.sh"

echo ""
echo "Build complete. Next:"
echo "  bash ${ROOT}/scripts/Angie_TLCP_PQC/render-conf.sh"
echo "  bash ${ROOT}/scripts/Angie_TLCP_PQC/start-angie.sh"
echo "  bash ${ROOT}/scripts/Angie_TLCP_PQC/test-client.sh"
