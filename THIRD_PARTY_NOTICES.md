# 第三方许可与来源

本项目是原厂系统、独立组件和社区修改的组合。根目录 GPL-3.0 仅覆盖本项目有权授权的原创程序和修改，不能覆盖或取消下面的原始许可；各文件中的版权头、LICENSE、COPYING 和 NOTICE 优先保留。标准 GPL 允许的兼容组合按其条款处理，不把较严格的 AGPL 改成 GPL。

| 部分 | 来源与处理 |
| --- | --- |
| SNOWSKY DISC 基础系统、原厂 UI/player、厂商库和硬件资源 | 飞傲/厂商原有权利。本仓库不声称拥有原厂程序版权，不把原厂闭源部分重新许可为 GPL |
| Linux/BusyBox/glibc 等原厂开源基础软件 | 保持各自 GPL/LGPL 等许可。完整原厂源码/SDK不在本项目自产源码范围；不能把本仓库当作原厂所有组件的完整源码替代品 |
| 在线音乐后端 | 基于 guohuiyuan/music-lib，原有 **AGPL-3.0** 继续适用；对应上游、修改和 Go 依赖放在 `firmware_dev/features/online-music`。项目：https://github.com/guohuiyuan/music-lib |
| Dragonfly / Freeverb3 移植 | 原有 GPL 条款、作者信息保留。移植代码位于 `audio-dsp/third_party/dragonfly-port`；附原始 Dragonfly 许可。项目：https://github.com/michaelwillis/dragonfly-reverb |
| Airwindows Tube2、PowerSag2、StereoFX 等 | 保留原有 MIT 许可和作者 Chris Johnson 等署名。相关源码与出处随 `audio-dsp/third_party` 保留。项目：https://github.com/airwindows/airwindows |
| FFmpeg Haas | 保留原 LGPL 许可和源码署名，出处随 `audio-dsp/third_party/ffmpeg-haas` 保留。项目：https://ffmpeg.org/ |
| SoXR | 保留 LGPL 许可及作者信息，源码位于 `dsp-resample/third_party/soxr`。项目：https://sourceforge.net/projects/soxr/ |
| LHDC 适配与蓝牙依赖 | 保留 `bluetooth-lhdcv5/third_party` 中各项目原有许可；不声称拥有 LHDC 商标、专利或额外编码器授权 |
| 拼音输入、二维码、Go 图片处理、D-Bus 等 | 保留各自 Apache-2.0、MIT、BSD 等许可和依赖作者信息，具体以随附 LICENSE/NOTICE 为准 |
| stb_image | 保留 Sean Barrett 原 MIT/公有领域双授权文本，见 `usb-tools/third_party/LICENSE` |
| Cloner、USB 驱动及所含依赖 | 厂商及相关上游原许可；不归本项目重新授权。发布包保持必要文件，不能因本仓库根 GPL 而推断全部厂商工具都是 GPL-3.0 |

转载整个固件包时也应保留第三方说明与原始许可。源码范围和构建前提见 [SOURCE.md](SOURCE.md)。发现归属或许可遗漏时，可在仓库 Issues 提供具体文件与来源以便修正。
