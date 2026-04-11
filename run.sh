#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"
if command -v python3 >/dev/null 2>&1; then
  python3 "$DIR/scripts/gen_compile_commands.py"
fi
INC=(
  -Iclient -Iserver
  -Ilayer1 -Ilayer2 -Ilayer3 -Ilayer4 -Ilayer5 -Ilayer6 -Ilayer7
)
cc -Wall -Wextra -std=c11 "${INC[@]}" -o topo layer3/topo_main.c \
  layer3/dijkstra.c layer3/routing_table_conversion.c
cc -Wall -Wextra -std=c11 "${INC[@]}" -o sim client/sim_main.c \
  client/client.c client/sim_arp.c client/sim_config.c server/server.c \
  layer4/sim_frame.c layer4/layer_pdu_print.c \
  layer1/layer1_forward.c layer7/layer7_forward.c \
  layer2/eth.c layer2/switch.c \
  layer3/ipv4.c layer3/router.c layer3/dijkstra.c \
  layer3/routing_table_conversion.c \
  layer4/udp.c \
  layer7/sm_session.c layer7/sm_payload.c layer7/sm4.c
exec ./sim
