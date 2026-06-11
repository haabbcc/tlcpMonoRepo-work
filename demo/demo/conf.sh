#!/bin/sh
set -e

cat >&2 <<'EOF'
conf.sh is a legacy hardware/SDF helper and is not part of the current
tlcpMonoRepo demo flow.

Use the Tongsuo-based demo commands instead:

  cd demo/demo
  ./mk.sh
  ./server
  ./client

For the standard TLS interface test:

  ./test_tls.sh
EOF

exit 1
