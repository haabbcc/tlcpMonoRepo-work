#!/bin/sh
set -e

cat >&2 <<'EOF'
km.sh is a legacy build helper for the old local ./lib and ./include layout.
It does not build against tlcpMonoRepo/tongsuo and should not be used for the
current Tongsuo NTLS/TLS demo.

Use:

  cd demo/demo
  sh ./mk.sh

mk.sh links against:

  ../../tongsuo
  ../../tongsuo/include
EOF

exit 1
