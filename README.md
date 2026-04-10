# NetworkTopology

教学用 C 语言网络仿真：**客户端**在端系统完成 **L7/L4/L3/L2 组帧** → 经 **L1 透传**、**L2 交换**、**L3 路由**、再 **L1 透传** → **B 侧 L5–L7 不实现仅透传** → **服务端**收包解密；拓扑为 **节点 A → 交换机 → 路由器 → 交换机 → 节点 B**，载荷为 **L4 UDP**，国密 **SM4**（GB/T 32907-2016）**CBC + PKCS#7** 在**客户端/服务端**完成加解密演示。

## 目录结构

| 路径 | 说明 |
|------|------|
| **`client/`** | 客户端：组帧、`client_emit_frame` / `client_emit_frame_sm4` |
| **`server/`** | 服务端：`server_receive`（含 SM4 解密） |
| **`layer1/`** | 物理层：比特流仿真直通 `layer1_forward` |
| **`layer2/`** | 数据链路层：`eth`、交换机 CAM 学习/转发 |
| **`layer3/`** | 网络层：IPv4、路由器 LPM/TTL/ARP（演示）、迪杰斯特拉与 FIB；`topo_main.c` 为独立拓扑演示入口 |
| **`layer4/`** | 传输层：`udp`、`SimFrame` 与端系统类型 `SimHost`；`layer_pdu_print` 供各层打印 |
| **`layer5/`** | 预留目录（本仿真不在此实现会话层） |
| **`layer6/`** | 预留目录（本仿真不在此实现表示层） |
| **`layer7/`** | 端系统侧：`layer7_forward` 仅打印「L5–L7 透传」；**SM4** 与载荷逻辑（`sm4`/`sm_payload`/`sm_session`）供客户端、服务端链接 |
| **`config/`** | `network.conf`：节点 IP/MAC、网关、UDP 端口、`use_sm4`、`packet_file` 路径 |
| **`data/`** | 默认 `payload.txt`：应用层载荷文本文件（按字节读入），由客户端读入后组帧 |
| **根目录** | 仅保留 `run.sh`、`README.md`（构建与说明） |

交换机只处理 **L2**（打印以太网首部 + **不透明** IP 数据报字节块）、路由器只处理 **L3**（解析 IPv4 首部，IP 数据区**不透明**）。**L1**、**B 侧 L5–L7** 在仿真里**仅打印透传**，不重组线帧、不组 PDU。**L7** 业务逻辑仅在发送端组帧与接收端 `server_receive` 中体现。

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

- `./sim`：全链路仿真（默认执行，`client/sim_main.c`）。固定参数**全部**从配置文件读取（默认 `config/network.conf`）；键名须**全大写**，且须列齐所有项（代码内无默认值）。载荷路径由 `PACKET_FILE` 指定（仓库自带 `data/payload.txt`）。
- `./sim /其它路径/xxx.conf`：指定配置文件；路径相对于**当前工作目录**（`run.sh` 会先 `cd` 到项目根）。
- `./topo`：`layer3/topo_main.c` + 迪杰斯特拉与 LPM 演示

### 配置文件 `config/network.conf`

`KEY=value` 一行一项，**键名必须全大写**；`#` 开头为注释。须包含下表**全部**键，缺一不可；不识别的键或缺项会导致加载失败（无内置默认）。

| 键 | 含义 |
|----|------|
| `NODE_A_ID` / `NODE_B_ID` | 节点编号（正整数，写入 `SimHost`） |
| `SWITCH_PORT_COUNT` | 每台交换机端口数（2–8，与 `layer2/switch` 上限一致） |
| `NODE_A_IP` / `NODE_B_IP` | 两端 IPv4（点分十进制） |
| `NODE_A_GW_IP` | A 的默认网关（演示用） |
| `NODE_A_MAC` / `NODE_B_MAC` / `NODE_A_GW_MAC` | 十六进制，支持 `:` 或 `-` 分隔 |
| `UDP_SPORT` / `UDP_DPORT` | UDP 端口（1–65535） |
| `USE_SM4` | `1` 加密载荷，`0` 明文 |
| `PACKET_FILE` | 载荷文件路径（按字节读入；相对路径相对运行时的当前目录） |

**注意**：内置路由器 FIB 仍为 `192.168.1.0/24` 与 `10.0.0.0/8`；若修改 IP，须仍落在这两个网段内，否则 L3 会丢弃。

手动编译须加入各层与 `client`/`server` 的 `-I` 路径，源文件列表见 `run.sh`。

## 许可证

- **`layer7/sm4.c`**：Apache-2.0，见 `layer7/NOTICE` 与文件头。

## 推送到 GitHub 组织 chl-5g

```bash
gh repo create chl-5g/NetworkTopology --public --source=. --remote=origin --push
```

（需已 `gh auth login` 且具备组织权限。）
