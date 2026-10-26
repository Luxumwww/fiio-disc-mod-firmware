# 源码范围与构建

本仓库公开社区扩展和修改代码，保留生产目录结构 `firmware_dev/features`，以及版本锁、组件版本表和便携刷写脚本源码。**这不是飞傲完整 Linux SDK，也不包含原厂 mq_ui/mq_player 的源码。** 不应把整个二进制固件描述成全部采用单一开源协议。

## 版本对应

DISI-02：Host 1.3.220 / UI 1.10.220、DSP 1.17.0、硬件合成 provider 1.0.7、万花筒 1.0.4，完整组件版本和开关状态见 `build-info/firmware-components.json`。

`build-info/host220-build-lock.json` 保留构建输入锁；发布前对其中 178 个直接/递归源码依赖逐项 SHA-256 核对。DSP 的输入锁见 `build-info/dsp117-build-lock.json`。`source-sha256.json` 记录本次公开源文件摘要。版本锁用于识别构建输入，不代替源代码。

固件和源码同一 Release 提供 `DISI-02.zip`、`DISI-02-source.zip` 及 `SHA256SUMS.txt`，源码也直接在仓库中。固件为清理个人数据后的初始化镜像，不是作者个人 NAND 备份。

## 主要入口

- 宿主与本机菜单：`firmware_dev/features/high-rate-visuals/build_native_220.py`，源码在 bluetooth-meter、settings-debug 及其引用目录。
- DSP：`audio-dsp/build.py`；SoXR 和降采样桥在 dsp-resample；`high-rate-visuals/build_dsp_1170.py` 保存相关构建锁逻辑。
- 图形 provider：hardware-accel/runtime-provider 及 audio-visuals 下的 build.py。
- 在线音乐：online-music 中 Go 模块、修改后的 upstream、vendor 依赖及输入法代码，继续按 AGPL 和依赖许可发布。
- BLE 控制后端：mobile-remote/server；时间同步：time-sync；热成像：usb-tools。
- USB/调试/组件管理：component-runtime、adb-debug 及相关目录；Windows 刷写入口源码在 tools。

生产源文件及引用的第三方代码保留原始内容与许可；部署现场日志、账号、备份、密钥、一次性安装脚本及撤回的高刷试验不属于公开源码包。Android APK 不随此固件发布。

## 构建前提

使用 Linux/WSL、Clang/LLD、Python 3、Go、SquashFS/UBIFS 工具及匹配目标的 MIPS sysroot。项目使用 mipsel/o32、hard-float、FP64、NaN2008，部分优化代码使用 MSA。普通 ARM 或 Android 库不能代替目标库。

现有脚本保留原固定构建目录约定：原厂 V2.57 的解包基线 `/root/disc-official-component-base-20261004-r1/base`，匹配的交叉工具链/sysroot 位于 `/root/disc-accel-research-20261006/toolchain`。这些基础文件应从合法取得的原厂固件和匹配 SDK 准备，未作为“本项目原创源码”重新授权。

常规顺序为 provider/可视化依赖 → SoXR/DSP → 宿主/菜单；构建脚本会检查目标 ABI、原厂程序摘要和 provider 摘要。若使用构建锁检查，需将 `build-info` 中对应锁文件放回脚本所指的 reports 路径。脚本中机器固定路径可按本地环境调整，不能用旧版 provider 绕过新桥的校验。

本次发布核对现有对应源码和二进制的构建记录，没有新增一次整套编译/刷机验收，也不承诺任意工具链下逐字节重建整个厂商固件。原厂闭源及厂商开源基础软件完整对应源码需依据原厂各自的许可和源码渠道取得；本仓库的 GPL 授权不代表已提供它们的全部源码或重新授予其权利。
