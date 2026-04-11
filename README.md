# NetworkTopology

教学用 C 语言网络仿真：**客户端**在端系统完成 **L7/L4/L3/L2 组帧** → 经 **L1 透传**、**L2 交换**、（可选）**L3 路由**、再 **L1 透传** → **B 侧 L5–L7 不实现仅透传** → **服务端**收包解密；载荷为 **L4 UDP**，国密 **SM4**（GB/T 32907-2016）**CBC + PKCS#7** 在**客户端/服务端**完成加解密演示。

## 拓扑（由配置自动选择）

根据 **`LAN_PREFIX_LEN`** 判断 `NODE_A_IP` 与 `NODE_B_IP` 是否在同一前缀下：

| 模式 | 条件 | 数据路径 |
|------|------|----------|
| **同网段（二层）** | 两 IP 在该前缀下属于同一子网 | **节点 A — SW1 — 节点 B**，不经路由器；以太网目的 MAC 为 B 的 MAC。 |
| **跨网段（三层）** | 否则 | **节点 A — SW1 — 路由器 — SW2 — 节点 B**；组帧时以太网目的为 A 的网关 MAC。 |

跨网段时，内置路由器 FIB 仍为 **`192.168.1.0/24`**（A 侧）与 **`10.0.0.0/8`**（B 侧）；若 IP 不满足该约束，程序会在启动时拒绝并提示错误。

## 目录结构

| 路径 | 说明 |
|------|------|
| **`client/`** | 客户端：`sim_main.c` 主流程；`client_emit_frame_payload` / `client_emit_frame_payload_l2`（同网段）；配置 `sim_config` |
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
| `NODE_A_GW_IP` | A 的默认网关（跨网段组帧写以太网目的时用；同网段组帧不使用但仍须填写） |
| `NODE_A_MAC` / `NODE_B_MAC` / `NODE_A_GW_MAC` | 十六进制，`:` 或 `-` 分隔 |
| `UDP_SPORT` / `UDP_DPORT` | UDP 端口（1–65535） |
| `USE_SM4` | `1` 加密载荷，`0` 明文 |
| `PACKET_FILE` | 载荷文件路径（相对路径相对**运行时当前工作目录**） |

**同网段示例**：`LAN_PREFIX_LEN=24`，`NODE_A_IP=192.168.1.10`，`NODE_B_IP=192.168.1.20`。

**跨网段示例**（默认）：A 在 `192.168.1.0/24`，B 在 `10.0.0.0/8`。

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
