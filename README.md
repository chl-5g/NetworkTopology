# NetworkTopology

教学用 C 语言网络仿真：报文从 **节点 A → 交换机 → 路由器 → 交换机 → 节点 B**，按 **OSI 分层** 分目录存放源码；载荷经 **L4 UDP** 封装，**L7** 使用国密 **SM4**（GB/T 32907-2016）**CBC + PKCS#7** 做对称加密演示。

## 目录与分层对应

| 层级 | 目录 | 文件 | 说明 |
|------|------|------|------|
| **L2 数据链路层** | `layer2/` | `eth.c`/`eth.h`，`switch.c`/`switch.h` | 以太网首部、MAC 格式、交换机 CAM 学习/转发 |
| **L3 网络层** | `layer3/` | `ipv4.c`/`ipv4.h`，`router.c`/`router.h`，`topology.h`，`dijkstra.c`，`routing_table_conversion.c` | IPv4 字段、路由器 LPM/TTL/ARP（演示）、迪杰斯特拉与 FIB |
| **L4 传输层** | `layer4/` | `udp.c`/`udp.h` | 简化 UDP 首部（源/目的端口、length） |
| **L7 应用层** | `layer7/` | `app_node.c`/`app_node.h`，`sm4.c`/`sm4.h`，`sm_payload.c`/`sm_payload.h`，`sm_session.c`/`sm_session.h` | 主机收发、SM4 与 PSK/IV（演示） |
| **跨层 PDU** | 项目根目录 | `sim_frame.h`/`sim_frame.c` | `SimFrame` = `EthHeader` + `Ipv4Header` + `UdpHeader` + `AppPayload` |

交换机、路由器只处理 **L2/L3**（及以太类型判断 IPv4），**不解析** UDP 与 L7 密文。

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

- `./sim`：全链路仿真（默认执行）
- `./topo`：`main.c` + `layer3/` 中迪杰斯特拉与 LPM 演示

手动编译须加 include 路径，例如：

```bash
INC="-Ilayer2 -Ilayer3 -Ilayer4 -Ilayer7 -I."
cc -Wall -Wextra -std=c11 $INC -o sim sim_main.c sim_frame.c \
  layer2/eth.c layer2/switch.c \
  layer3/ipv4.c layer3/router.c layer3/dijkstra.c layer3/routing_table_conversion.c \
  layer4/udp.c \
  layer7/app_node.c layer7/sm_session.c layer7/sm_payload.c layer7/sm4.c
```

## 许可证

- **`layer7/sm4.c`**：Apache-2.0，见 `NOTICE` 与文件头。

## 推送到 GitHub 组织 chl-5g

```bash
gh repo create chl-5g/NetworkTopology --public --source=. --remote=origin --push
```

（需已 `gh auth login` 且具备组织权限。）
