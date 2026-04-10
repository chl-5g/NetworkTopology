#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"
INC=( -Ilayer2 -Ilayer3 -Ilayer4 -Ilayer7 -I. )
cc -Wall -Wextra -std=c11 "${INC[@]}" -o topo main.c \
    layer3/dijkstra.c layer3/routing_table_conversion.c
cc -Wall -Wextra -std=c11 "${INC[@]}" -o sim sim_main.c sim_frame.c \
    layer2/eth.c layer2/switch.c \
    layer3/ipv4.c layer3/router.c layer3/dijkstra.c \
    layer3/routing_table_conversion.c \
    layer4/udp.c \
    layer7/app_node.c layer7/sm_session.c layer7/sm_payload.c layer7/sm4.c
exec ./sim
