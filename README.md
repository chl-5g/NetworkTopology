# NetworkTopology

教学用 C 语言网络仿真：报文从 **节点 A → 交换机 → 路由器 → 交换机 → 节点 B**，并在应用载荷上使用 **国密 SM4**（GB/T 32907-2016）**CBC 模式 + PKCS#7 填充**做对称加密演示。

## 国密算法说明

| 算法 | 标准/用途 | 本仓库 |
|------|-----------|--------|
| **SM2** | 椭圆曲线公钥密码 | 未实现（可对接证书/密钥协商扩展） |
| **SM3** | 密码杂凑 | 未实现（可与 SM4 组合做 HMAC/完整性，自行扩展） |
| **SM4** | 分组密码（128 位密钥/分组） | **已实现**：`sm4.c`（源自 [GmSSL](https://github.com/guanzhi/GmSSL)，Apache-2.0） |

载荷加密流程：`node_emit_frame_sm4` → `sm_payload.c`（PKCS#7）→ `sm4_cbc_encrypt_blocks`；节点 B 收包后 `sm4_decrypt_message` 还原明文。交换机与路由器**不解析**密文字段，仅透明转发 `SimFrame`。

**安全提示**：演示使用 `sm_session.c` 中的固定 **PSK / IV**，仅用于课堂与本地实验，**禁止**用于生产或真实业务。

## 构建与运行

```bash
./run.sh
```

将编译两个可执行文件：

- `./sim`：完整链路 + SM4 载荷（默认由 `run.sh` 执行）
- `./topo`：迪杰斯特拉 + 全局 FIB 最长前缀匹配演示（与 `main.c` 对应）

手动编译示例：

```bash
cc -Wall -Wextra -std=c11 -o sim sim_main.c sim_frame.c node.c switch.c \
  router.c sm_session.c sm_payload.c sm4.c dijkstra.c routing_table_conversion.c
```

## 源码结构（按设备/职责分文件）

| 文件 | 说明 |
|------|------|
| `sim_main.c` | 场景入口与步骤打印 |
| `node.c` / `node.h` | 主机：组帧、SM4 加解密、收包 |
| `switch.c` / `switch.h` | 二层：MAC 学习、转发/泛洪 |
| `router.c` / `router.h` | 三层：LPM、TTL、ARP 表（演示）、改写 MAC |
| `sim_frame.c` / `sim_frame.h` | 统一帧结构与打印 |
| `sm4.c` / `sm4.h` | SM4 分组密码与 CBC |
| `sm_payload.c` / `sm_payload.h` | PKCS#7 + SM4 消息封装 |
| `sm_session.c` / `sm_session.h` | 演示用 PSK、IV |
| `dijkstra.c`、`routing_table_conversion.c`、`topology.h` | 原路由表/LPM 模块（`topo` 与链接依赖） |

## 许可证

- 项目整体教学代码：可按需使用；**`sm4.c`** 为 GmSSL 摘录，遵循 **Apache-2.0**，见 `NOTICE` 与 `sm4.c` 文件头。

## 组织仓库

若推送到 GitHub 组织 **chl-5g**，可在本目录执行：

```bash
git init
git add .
git commit -m "Initial commit: network sim with SM4 payload"
gh repo create chl-5g/NetworkTopology --public --source=. --remote=origin --push
```

需已安装 [GitHub CLI](https://cli.github.com/) 并完成 `gh auth login`，且对组织 `chl-5g` 有创建仓库权限。
