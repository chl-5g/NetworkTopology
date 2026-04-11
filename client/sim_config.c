#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipv4.h"
#include "sim_config.h"

enum {
    M_NODE_A_ID = 1u << 0,
    M_NODE_B_ID = 1u << 1,
    M_SWITCH_PORT_COUNT = 1u << 2,
    M_NODE_A_IP = 1u << 3,
    M_NODE_B_IP = 1u << 4,
    M_NODE_A_GW_IP = 1u << 5,
    M_NODE_A_MAC = 1u << 6,
    M_NODE_B_MAC = 1u << 7,
    M_NODE_A_GW_MAC = 1u << 8,
    M_NODE_B_GW_IP = 1u << 14,
    M_NODE_B_GW_MAC = 1u << 15,
    M_ROUTE_PREFIX_A = 1u << 16,
    M_ROUTE_PREFIX_B = 1u << 17,
    M_EXTRA_ARP1_IP = 1u << 18,
    M_EXTRA_ARP1_MAC = 1u << 19,
    M_EXTRA_ARP2_IP = 1u << 20,
    M_EXTRA_ARP2_MAC = 1u << 21,
    M_UDP_SPORT = 1u << 9,
    M_UDP_DPORT = 1u << 10,
    M_USE_SM4 = 1u << 11,
    M_PACKET_FILE = 1u << 12,
    M_LAN_PREFIX_LEN = 1u << 13,
    M_ALL = M_NODE_A_ID | M_NODE_B_ID | M_SWITCH_PORT_COUNT | M_NODE_A_IP |
            M_NODE_B_IP | M_NODE_A_GW_IP | M_NODE_A_MAC | M_NODE_B_MAC |
            M_NODE_A_GW_MAC | M_NODE_B_GW_IP | M_NODE_B_GW_MAC |
            M_ROUTE_PREFIX_A | M_ROUTE_PREFIX_B | M_EXTRA_ARP1_IP |
            M_EXTRA_ARP1_MAC | M_EXTRA_ARP2_IP | M_EXTRA_ARP2_MAC |
            M_UDP_SPORT | M_UDP_DPORT | M_USE_SM4 | M_PACKET_FILE |
            M_LAN_PREFIX_LEN
};

static void str_trim(char *s) {
    char *p = s;
    while (*p != '\0' && isspace((unsigned char)*p)) {
        p++;
    }
    if (p != s) {
        memmove(s, p, strlen(p) + 1);
    }
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        n--;
    }
    s[n] = '\0';
}

static int parse_mac(const char *s, uint8_t m[6]) {
    unsigned x[6];
    if (sscanf(s, "%2x:%2x:%2x:%2x:%2x:%2x", &x[0], &x[1], &x[2], &x[3], &x[4],
               &x[5]) == 6) {
        goto ok;
    }
    if (sscanf(s, "%2x-%2x-%2x-%2x-%2x-%2x", &x[0], &x[1], &x[2], &x[3], &x[4],
               &x[5]) != 6) {
        return -1;
    }
ok:
    for (int i = 0; i < 6; i++) {
        if (x[i] > 255u) {
            return -1;
        }
        m[i] = (uint8_t)x[i];
    }
    return 0;
}

static int key_match(const char *key, const char *upper_name) {
    return strcmp(key, upper_name) == 0;
}

int sim_net_config_load(const char *path, SimNetConfig *c) {
    memset(c, 0, sizeof(*c));
    c->packet_file[0] = '\0';

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return -1;
    }

    uint32_t mask = 0;
    char line[512];
    while (fgets(line, sizeof line, fp) != NULL) {
        str_trim(line);
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        char *eq = strchr(line, '=');
        if (eq == NULL) {
            fclose(fp);
            return -1;
        }
        *eq = '\0';
        char *key = line;
        char *val = eq + 1;
        str_trim(key);
        str_trim(val);

        if (key_match(key, "NODE_A_ID")) {
            long v = strtol(val, NULL, 10);
            if (v < 1 || v > 65535) {
                fclose(fp);
                return -1;
            }
            c->node_a_id = (int)v;
            mask |= M_NODE_A_ID;
        } else if (key_match(key, "NODE_B_ID")) {
            long v = strtol(val, NULL, 10);
            if (v < 1 || v > 65535) {
                fclose(fp);
                return -1;
            }
            c->node_b_id = (int)v;
            mask |= M_NODE_B_ID;
        } else if (key_match(key, "SWITCH_PORT_COUNT")) {
            long v = strtol(val, NULL, 10);
            if (v < 2 || v > 8) {
                fclose(fp);
                return -1;
            }
            c->switch_port_count = (int)v;
            mask |= M_SWITCH_PORT_COUNT;
        } else if (key_match(key, "NODE_A_IP")) {
            if (ipv4_parse(val, &c->node_a_ip) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_NODE_A_IP;
        } else if (key_match(key, "NODE_B_IP")) {
            if (ipv4_parse(val, &c->node_b_ip) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_NODE_B_IP;
        } else if (key_match(key, "NODE_A_GW_IP")) {
            if (ipv4_parse(val, &c->node_a_gw_ip) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_NODE_A_GW_IP;
        } else if (key_match(key, "NODE_A_MAC")) {
            if (parse_mac(val, c->node_a_mac) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_NODE_A_MAC;
        } else if (key_match(key, "NODE_B_MAC")) {
            if (parse_mac(val, c->node_b_mac) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_NODE_B_MAC;
        } else if (key_match(key, "NODE_A_GW_MAC")) {
            if (parse_mac(val, c->node_a_gw_mac) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_NODE_A_GW_MAC;
        } else if (key_match(key, "NODE_B_GW_IP")) {
            if (ipv4_parse(val, &c->node_b_gw_ip) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_NODE_B_GW_IP;
        } else if (key_match(key, "NODE_B_GW_MAC")) {
            if (parse_mac(val, c->node_b_gw_mac) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_NODE_B_GW_MAC;
        } else if (key_match(key, "ROUTE_PREFIX_A")) {
            long v = strtol(val, NULL, 10);
            if (v < 1 || v > 32) {
                fclose(fp);
                return -1;
            }
            c->route_prefix_a = (int)v;
            mask |= M_ROUTE_PREFIX_A;
        } else if (key_match(key, "ROUTE_PREFIX_B")) {
            long v = strtol(val, NULL, 10);
            if (v < 1 || v > 32) {
                fclose(fp);
                return -1;
            }
            c->route_prefix_b = (int)v;
            mask |= M_ROUTE_PREFIX_B;
        } else if (key_match(key, "EXTRA_ARP1_IP")) {
            if (ipv4_parse(val, &c->extra_arp1_ip) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_EXTRA_ARP1_IP;
        } else if (key_match(key, "EXTRA_ARP1_MAC")) {
            if (parse_mac(val, c->extra_arp1_mac) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_EXTRA_ARP1_MAC;
        } else if (key_match(key, "EXTRA_ARP2_IP")) {
            if (ipv4_parse(val, &c->extra_arp2_ip) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_EXTRA_ARP2_IP;
        } else if (key_match(key, "EXTRA_ARP2_MAC")) {
            if (parse_mac(val, c->extra_arp2_mac) != 0) {
                fclose(fp);
                return -1;
            }
            mask |= M_EXTRA_ARP2_MAC;
        } else if (key_match(key, "UDP_SPORT")) {
            unsigned long p = strtoul(val, NULL, 10);
            if (p == 0 || p > 65535u) {
                fclose(fp);
                return -1;
            }
            c->udp_sport = (uint16_t)p;
            mask |= M_UDP_SPORT;
        } else if (key_match(key, "UDP_DPORT")) {
            unsigned long p = strtoul(val, NULL, 10);
            if (p == 0 || p > 65535u) {
                fclose(fp);
                return -1;
            }
            c->udp_dport = (uint16_t)p;
            mask |= M_UDP_DPORT;
        } else if (key_match(key, "USE_SM4")) {
            if (strcmp(val, "1") == 0) {
                c->use_sm4 = 1;
            } else if (strcmp(val, "0") == 0) {
                c->use_sm4 = 0;
            } else {
                fclose(fp);
                return -1;
            }
            mask |= M_USE_SM4;
        } else if (key_match(key, "LAN_PREFIX_LEN")) {
            long v = strtol(val, NULL, 10);
            if (v < 1 || v > 32) {
                fclose(fp);
                return -1;
            }
            c->lan_prefix_len = (int)v;
            mask |= M_LAN_PREFIX_LEN;
        } else if (key_match(key, "PACKET_FILE")) {
            if (strlen(val) == 0 || strlen(val) >= sizeof(c->packet_file)) {
                fclose(fp);
                return -1;
            }
            strcpy(c->packet_file, val);
            mask |= M_PACKET_FILE;
        } else {
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);

    if (mask != M_ALL) {
        return -1;
    }
    return 0;
}

int sim_packet_load(const char *path, uint8_t *buf, size_t cap, size_t *out_len) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return -1;
    }
    size_t n = fread(buf, 1, cap, fp);
    if (ferror(fp)) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    *out_len = n;
    return 0;
}
