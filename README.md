# FFRT 学习文档包

本仓库**只包含文档**，不含 FFRT 源码。源码来自 OpenHarmony 上游，用一条命令即可还原「源码 + 文档」的完整环境（见下文）。

## 仓库结构

```text
FFRT独立文档/        可整体剥离的学习包（不依赖源码即可阅读）
├── README.md         入口导航
├── FFRT源码导读.md    骨架篇：全景、主链路、架构、学习路径
├── FFRT实现细节.md    细节篇：12 章机制深挖 + 运行时验证记录
├── FFRT参考手册.md    参考篇：全量 API、源码锚点、宏开关
├── FFRT问题集.md      问答篇：实战排查 / 自测 / 开放问题
└── smoke_test.cpp     行为冒烟测试（6 项，已验证）
guide.md             提示词复用指南（本次工作流的方法论提炼）
docs/module-map/     仓库内点击跳转版结构地图（需配合源码使用）
docs/README.md       用户指南入口（含 module-map 挂接，覆盖上游同名文件）
setup.sh / setup.ps1 还原「源码 + 文档」环境
```

## 还原完整环境（源码 + 文档）

文档基于上游快照 `1d559e70feda31fa8afd45d564daa242660e4edd` 梳理并验证，脚本默认检出该快照。

Linux / macOS / WSL：

```bash
./setup.sh              # 默认克隆到 ./ffrt-env
FFRT_COMMIT=latest ./setup.sh my-dir   # 可选：跟随上游最新（文档锚点可能漂移）
```

Windows PowerShell：

```powershell
.\setup.ps1 -Target ffrt-env
```

脚本做的事：克隆上游源码 → 检出基准快照（detached HEAD）→ 把本仓库的文档产物按原相对路径覆盖进去。`FFRT_REPO` 环境变量可替换上游地址。

还原后：
- `FFRT独立文档/` 可独立阅读，也可就地配合源码；
- `docs/module-map/` 的相对链接在源码树内全部可解析；
- 冒烟测试的构建与运行方法见 `FFRT独立文档/smoke_test.cpp` 文件头注释，验证记录见细节篇 §12。

## 上游来源

- 源码仓：<https://gitcode.com/openharmony/resourceschedule_ffrt>
- 文档基准：`1d559e70feda31fa8afd45d564daa242660e4edd`（2026-10 快照）
