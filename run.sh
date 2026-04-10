#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"
cc -Wall -Wextra -std=c11 -o topo main.c dijkstra.c routing_table_conversion.c
cc -Wall -Wextra -std=c11 -o sim sim_main.c sim_frame.c node.c switch.c \
    router.c sm_session.c sm_payload.c sm4.c \
    dijkstra.c routing_table_conversion.c
exec ./sim
