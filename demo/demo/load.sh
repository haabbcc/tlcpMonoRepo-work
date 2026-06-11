#!/bin/sh
set -e

cat >&2 <<'EOF'
load.sh is a legacy environment helper with machine-specific paths.
It is not used by the current tlcpMonoRepo build or demo tests.

Use the repository environment instead:

  source ../../env.sh
EOF

exit 1
