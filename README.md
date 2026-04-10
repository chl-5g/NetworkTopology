# NetworkTopology

教学用 C 语言网络仿真：**客户端**组帧发出 → **L1–L7 中间层**（以 `SimFrame` 透传或真实转发）→ **服务端**收包解密；拓扑仍为 **节点 A → 交换机 → 路由器 → 交换机 → 节点 B**，载荷经 **L4 UDP**，国密 **SM4**（GB/T 32907-2016）**CBC + PKCS#7** 在**客户端/服务端**完成加解密演示。

## 目录结构

| 路径 | 说明 |
|------|------|
| **`client/`** | 客户端：组帧、`client_emit_frame` / `client_emit_frame_sm4` |
| **`server/`** | 服务端：`server_receive`（含 SM4 解密） |
| **`layer1/`** | 物理层：比特流仿真直通 `layer1_forward` |
| **`layer2/`** | 数据链路层：`eth`、交换机 CAM 学习/转发 |
| **`layer3/`** | 网络层：IPv4、路由器 LPM/TTL/ARP（演示）、迪杰斯特拉与 FIB；`topo_main.c` 为独立拓扑演示入口 |
| **`layer4/`** | 传输层：`udp`、`SimFrame` 与端系统类型 `SimHost`；`layer4_forward` 中段透传 |
| **`layer5/`** | 会话层：`layer5_forward` 透传 |
| **`layer6/`** | 表示层：`layer6_forward` 透传 |
| **`layer7/`** | 应用层中继：`layer7_forward` 透传；**SM4** 实现与国密载荷逻辑（`sm4`/`sm_payload`/`sm_session`）供客户端、服务端链接 |
| **根目录** | 仅保留 `run.sh`、`README.md`（构建与说明） |

交换机、路由器只处理 **L2/L3**（及以太类型判断 IPv4），**不解析** UDP 与 L7 密文；L1/L4/L5/L6/L7 的中间函数在仿真里主要体现**分层调用顺序**与**透传语义**。

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

- `./sim`：全链路仿真（默认执行，`client/sim_main.c`）
- `./topo`：`layer3/topo_main.c` + 迪杰斯特拉与 LPM 演示

手动编译须加入各层与 `client`/`server` 的 `-I` 路径，源文件列表见 `run.sh`。

## 许可证

- **`layer7/sm4.c`**：Apache-2.0，见 `layer7/NOTICE` 与文件头。

## 推送到 GitHub 组织 chl-5g

```bash
gh repo create chl-5g/NetworkTopology --public --source=. --remote=origin --push
```

（需已 `gh auth login` 且具备组织权限。）
