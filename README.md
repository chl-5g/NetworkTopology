# NetworkTopology

教学用 C 语言网络仿真：**客户端**在端系统完成 **L7/L4/L3/L2 组帧** → 经 **L1 透传**、**L2 交换**、（可选）**L3 路由**、再 **L1 透传** → **B 侧 L5–L7 不实现仅透传** → **服务端**收包解密；载荷为 **L4 UDP**，国密 **SM4**（GB/T 32907-2016）**CBC + PKCS#7** 在**客户端/服务端**完成加解密演示。

## 项目原则

**尽量与真实网络实现一致**：控制面/数据面的先后关系（如先 ARP 再组帧）、子网与掩码语义、交换机泛洪与 CAM、路由器 FIB/ARP 表等，能对齐现网或教材处则对齐；**不得已简化时**（单副本 `SimFrame`、无真 ARP 以太网类型、无校验和与线速比特流等），在本文或代码注释中**写清差异与原因**，后续加功能时优先**缩小与现网的差距**，而非再堆一层「仅仿真专用」的黑魔法。

## 拓扑（由配置自动选择）

根据 **`LAN_PREFIX_LEN`** 判断 `NODE_A_IP` 与 `NODE_B_IP` 是否在同一前缀下：

| 模式 | 条件 | 数据路径 |
|------|------|----------|
| **同网段（二层）** | 两 IP 在 **`LAN_PREFIX_LEN`** 下属于同一子网 | **节点 A — SW1 — 节点 B**，不经路由器；以太网目的 MAC 为 B 的 MAC。 |
| **跨网段（三层）** | 否则 | **节点 A — SW1 — 路由器 — SW2 — 节点 B**；组帧时以太网目的为 **A 侧网关 MAC**；路由器根据 **`ROUTE_PREFIX_A` / `ROUTE_PREFIX_B`** 与 **`NODE_A_GW_IP` / `NODE_B_GW_IP`** 生成两条**直连 FIB**（非写死网段）。 |

跨网段时启动会校验：A 与 `NODE_A_GW_IP` 同属 `ROUTE_PREFIX_A`；B 与 `NODE_B_GW_IP` 同属 `ROUTE_PREFIX_B`；且 B 不在 A 侧前缀内、A 不在 B 侧前缀内，避免与「单交换机两侧」模型冲突。

**ARP（教学仿真）**：`sim_arp_resolve_from_node_a` 用 **`ROUTE_PREFIX_A`** 判断是否与 A 同子网；同子网则模拟 **Who-has**（**NODE_B**、**NODE_A_GW_IP**、**EXTRA_ARP1/2**）；跨子网则 **Who-has 默认网关**。未实现以太网 ARP 报文、超时与表项老化。

**交换机**：未知单播时向**除入端口外所有端口**打印泛洪（仿真仍只沿**首个出端口**继续后续逻辑，避免复制 `SimFrame`）。

## 与真实网络实现的差异（备忘）

以下为当前实现与现网/真协议栈的常见差距，便于对照笔记；多数为**教学刻意简化**，非「实现错误」。

### 1. 总体与执行模型

1. **单线程、单包、固定顺序流水线**：现实为多包并发、中断/NAPI、多队列等；`sim_main` 按步骤串行走一遍。
2. **`SimFrame` 为 C 结构体拼成的逻辑帧**：现实为网卡 DMA 线序字节流，再解析为 `skb`；本仓库直接读写 `EthHeader`/`Ipv4Header`/…，`server_receive` 亦读结构体而非从线速缓冲再拆栈。
3. **无时间轴**：无定时器、无 ARP 超时、无 TCP 式重传等状态机驱动。

### 2. 物理层 L1

4. **无前导码、SFD、IFG、FCS、比特级语义**：`layer1_forward` 仅打印透传。
5. **无 PHY 速率/双工/误码模型**。

### 3. 数据链路层 L2

6. **以太网仅有 `dst/src/type`**：无 802.1Q VLAN、无双标签、无优先级域等。
7. **交换机 CAM**：无老化、无端口安全策略、无 ACL/TCAM；仅有简单上限。
8. **未知单播泛洪**：已逐出端口打印，但**仍只有一个 `SimFrame` 实例**，后续只沿首出端口继续，与「每口独立副本」不等价。
9. **无 STP/RSTP、无 LACP/MLAG**。
10. **L2 对 IP 载荷按「不透明块」打印**：与会做深度解析的智能交换机不同，属教学取舍。

### 4. 网络层 L3

11. **`Ipv4Header` 极简**（`src/dst/ttl/protocol`）：真实 IPv4 至少 20 字节固定头（Version、IHL、TOS、Total Length、Id、Flags、Fragment Offset、TTL、Protocol、**Header Checksum**、Src、Dst 等）；无选项、无分片字段语义。
12. **无分片/重组、无 DF/PMTU、无 MTU 检查**。
13. **无 IPv4 头校验和的计算与校验**。
14. **路由器**：仅两接口、FIB 仅两条直连；无默认路由、无 IGP/BGP、无策略路由/VRF。
15. **`lpm_lookup_in` 用 `popcount(netmask)` 当前缀长度**：假设**连续掩码**；非连续掩码或策略路由与现网可能不同。
16. **路由器 ARP**：表项以启动时 `router_add_arp` 与 `EXTRA_ARP*` 为主；**无收包动态学习、无超时、无邻居状态机**。
17. **无 ICMP**（不可达、TTL exceeded、PMTU、echo 等）。
18. **目的为本机接口 IP**：仅打印「上送控制平面」，无真协议栈上送。
19. **无 uRPF、无 ECMP** 等。

### 5. 传输层 L4

20. **UDP checksum 固定为 0**（见 `udp.h` 注释）：与常见「计算/校验或 offload 语义」不一致。
21. **无 socket/bind/listen、无端口冲突与内核调度**：直接填 `UdpHeader`。

### 6. 会话层以上 / 应用与加密

22. **`layer7_forward` 仅打印**：不拆 TLS/HTTP 等真实 PDU。
23. **SM4 在 `AppPayload` 内、PSK/IV 固定**：与生产密钥管理、握手、证书链不一致；仅实验演示。

### 7. ARP 与主机侧

24. **`sim_arp` 为打印级仿真**：无以太网类型 **0x0806**、无 ARP 报文布局、无广播域上真实帧交换。
25. **同子网邻居表很小**：除 B、网关、`EXTRA_ARP*` 外，任意 IP 的 ARP 失败路径未建模。
26. **`LAN_PREFIX_LEN` 与 `ROUTE_PREFIX_A` 可独立配置**：现实多来自**同一接口掩码**；不一致时行为可能与单掩码主机不同（已有 stderr 提示场景）。

### 8. 配置与运维

27. **全大写、全必填、无 DHCP/无 LLDP 等自动发现**：键多是为**显式教学**；现实常由 DHCP/控制器下发，表面配置更短。

### 9. 与 `topo`/迪杰斯特拉

28. **`./topo`、迪杰斯特拉、`routing_table_conversion` 与 `./sim` 主路径独立**：现实可在同一设备上 IGP 与转发表联动；本仿真路由器不走迪杰斯特拉结果。

### 10. 安全与资源

29. **无认证授权、无防篡改（除 SM4 解密失败提示）**。
30. **无队列、无拥塞丢包、无 QoS/ECN**。

### 11. 字节序与线序

31. **IPv4 在结构体中为「主机序 uint32」**：线路上 IPv4 字段为大端字节序；与 Wireshark raw 逐字节对照时需注意。
32. **仅常见 `ether_type = IPv4` 路径**：未建模 QinQ 等类型链。

### 小结

本项目本质是 **「把现网关键概念拆成可见的 C 结构 + 打印 + 少量校验」**。若优先向现网靠拢，通常投入收益较大的方向是：**IPv4 完整首部与头校验和、MTU/ICMP、ARP 报文与邻居表状态机、线序缓冲区入口**（工作量也最大）。

## 目录结构

| 路径 | 说明 |
|------|------|
| **`client/`** | 客户端：`sim_main.c` 主流程；`sim_arp.c` 仿真 **ARP**（同网段解析 B，跨网段解析默认网关）；`client_emit_frame_payload_l2` 组帧；配置 `sim_config` |
| **`server/`** | 服务端：`server_receive`（含 SM4 解密） |
| **`layer1/`** | 物理层：`layer1_forward` 透传打印 |
| **`layer2/`** | 数据链路层：`eth`、交换机 CAM 学习/转发 |
| **`layer3/`** | 网络层：IPv4、路由器 LPM/TTL/ARP（演示）、迪杰斯特拉与 FIB；`topo_main.c` 为独立拓扑演示入口 |
| **`layer4/`** | 传输层：`udp`、`SimFrame`；`layer_pdu_print` 供各层打印 |
| **`layer5/`** / **`layer6/`** | 预留目录（本仿真不在此实现会话层/表示层） |
| **`layer7/`** | `layer7_forward` 透传打印；**SM4** 与载荷逻辑供客户端、服务端链接 |
| **`config/`** | `network.conf`：见下表 |
| **`data/`** | 默认载荷文件（由 `PACKET_FILE` 指定） |
| **`scripts/`** | `gen_compile_commands.py`：生成根目录 `compile_commands.json`（供 clangd / C++ 扩展；已加入 `.gitignore`） |
| **`.vscode/`** | 可选：构建任务、`compileCommands`、调试 `sim` 的示例配置 |

交换机只处理 **L2**（打印以太网首部 + **不透明** IP 数据报字节块）；路由器只处理 **L3**。**L1**、**B 侧 L5–L7** 在仿真里**仅打印透传**。

## 国密算法说明

| 算法 | 标准/用途 | 本仓库 |
|------|-----------|--------|
| **SM2** | 椭圆曲线公钥密码 | 未实现 |
| **SM3** | 密码杂凑 | 未实现 |
| **SM4** | 分组密码 | **`layer7/sm4.c`**（摘自 [GmSSL](https://github.com/guanzhi/GmSSL)，Apache-2.0） |

**安全提示**：`layer7/sm_session.c` 中为固定 **PSK / IV**，仅用于实验，**禁止**用于生产。

## 构建与运行

```bash
./run.sh
```

- 脚本会先 **`cd` 到项目根**，若存在 **`python3`** 则运行 **`scripts/gen_compile_commands.py`**，写出 **`compile_commands.json`**（本机绝对路径，勿提交；已 `.gitignore`）。
- **`./run.sh`**：编译 **`./sim`** 与 **`./topo`**，并**执行** `./sim`（默认配置 `config/network.conf`）。
- **`./sim`** / **`./sim /路径/xxx.conf`**：全链路仿真；配置键名须**全大写**，且须列齐所有项（无内置默认）。载荷路径由 `PACKET_FILE` 指定。
- **`./topo`**：`layer3/topo_main.c` 迪杰斯特拉与 LPM 演示。

手动编译须加入与 `run.sh` 相同的 **`-Iclient -Iserver -Ilayer1` … `-Ilayer7`** 及各 `.c` 源文件列表。

### 配置文件 `config/network.conf`

`KEY=value` 一行一项；`#` 开头为注释。须包含下表**全部**键。

| 键 | 含义 |
|----|------|
| **`LAN_PREFIX_LEN`** | **1–32**：用于判定 A/B 是否同网段（见上文「拓扑」）。 |
| `NODE_A_ID` / `NODE_B_ID` | 节点编号（正整数） |
| `SWITCH_PORT_COUNT` | 每台交换机端口数（2–8） |
| `NODE_A_IP` / `NODE_B_IP` | 两端 IPv4（点分十进制） |
| `NODE_A_GW_IP` | A 侧默认网关（即路由器 **if0** 地址；跨网段组帧写以太网目的 MAC 为 `NODE_A_GW_MAC`） |
| `NODE_B_GW_IP` | B 侧默认网关（路由器 **if1** 地址，须在 B 所在前缀内） |
| `NODE_A_MAC` / `NODE_B_MAC` | 主机 MAC，十六进制，`:` 或 `-` 分隔 |
| `NODE_A_GW_MAC` / `NODE_B_GW_MAC` | 路由器在 A 侧 / B 侧接口的 MAC（与 `layer1_forward`、交换机学习一致） |
| **`ROUTE_PREFIX_A`** / **`ROUTE_PREFIX_B`** | **1–32**：路由器在 if0 / if1 上直连的 IPv4 前缀长度，用于生成 FIB（与两端主机、网关同网段一致）；**主机 ARP 子网判定与 A 侧一致，用 `ROUTE_PREFIX_A`** |
| **`EXTRA_ARP1_IP` / `EXTRA_ARP1_MAC`** | 可选静态邻居：`0.0.0.0` 与全零 MAC 表示未使用；同子网时 `sim_arp` 可查 |
| **`EXTRA_ARP2_IP` / `EXTRA_ARP2_MAC`** | 同上第二条 |
| `UDP_SPORT` / `UDP_DPORT` | UDP 端口（1–65535） |
| `USE_SM4` | `1` 加密载荷，`0` 明文 |
| `PACKET_FILE` | 载荷文件路径（相对路径相对**运行时当前工作目录**） |

**同网段示例**：`LAN_PREFIX_LEN=24`，`NODE_A_IP=192.168.1.10`，`NODE_B_IP=192.168.1.20`；**建议 `ROUTE_PREFIX_A` 与 `LAN_PREFIX_LEN` 相同**（ARP 用前者；不经路由时 `ROUTE_PREFIX_B` 可保留默认，仅不参与本路径）。

**跨网段示例**（默认 `network.conf`）：`ROUTE_PREFIX_A=24`、`ROUTE_PREFIX_B=8`，A 在 `192.168.1.0/24`，B 在 `10.0.0.0/8`，两网关分别为 `192.168.1.1` 与 `10.0.0.1`。亦可改为例如 B 在 `172.16.0.0/16` 等，只要上述校验通过。

### 编辑器（Cursor / VS Code）

若使用 **clangd** 或 **C/C++** 扩展，依赖根目录 **`compile_commands.json`**：克隆后先执行一次 **`./run.sh`** 或 **`python3 scripts/gen_compile_commands.py`**。单文件「Run」不会带上 `run.sh` 的 `-I` 与多文件链接，请用 **`./run.sh`** 或工作区里的 **构建任务**（见 `.vscode/tasks.json`）。

## 许可证

- **`layer7/sm4.c`**：Apache-2.0，见 `layer7/NOTICE` 与文件头。

## 推送到 GitHub

已配置 `origin` 时：

```bash
git add -A
git status
git commit -m "你的提交说明"
git push origin main
```

首次在 GitHub 上建库并推送时，可使用 GitHub CLI（需已 `gh auth login`）：

```bash
gh repo create chl-5g/NetworkTopology --public --source=. --remote=origin --push
```
