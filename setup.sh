#!/usr/bin/env bash
# 还原「FFRT 源码 + 本文档包」的完整环境。
# 用法: ./setup.sh [目标目录]        默认: ffrt-env
# 环境变量:
#   FFRT_REPO    上游仓库地址（默认 OpenHarmony 官方仓）
#   FFRT_COMMIT  上游提交，默认固定为文档基准快照；设为 latest 跟随 master 最新
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FFRT_REPO="${FFRT_REPO:-https://gitcode.com/openharmony/resourceschedule_ffrt.git}"
FFRT_COMMIT="${FFRT_COMMIT:-1d559e70feda31fa8afd45d564daa242660e4edd}"
TARGET="${1:-ffrt-env}"

if [ -e "$TARGET" ]; then
    echo "错误: 目标目录 $TARGET 已存在" >&2
    exit 1
fi

echo "克隆上游源码: $FFRT_REPO"
git clone "$FFRT_REPO" "$TARGET"

cd "$TARGET"
if [ "$FFRT_COMMIT" = "latest" ]; then
    echo "使用上游 master 最新提交"
else
    echo "检出文档基准快照: $FFRT_COMMIT"
    git checkout "$FFRT_COMMIT"
fi

echo "覆盖文档产物..."
cp -r "$SCRIPT_DIR/FFRT独立文档" .
cp "$SCRIPT_DIR/guide.md" .
mkdir -p docs
cp -r "$SCRIPT_DIR/docs/module-map" docs/
cp "$SCRIPT_DIR/docs/README.md" docs/README.md

echo ""
echo "完成: ./$TARGET"
echo "  - FFRT独立文档/     可独立阅读的学习包"
echo "  - docs/module-map/  仓库内导航（源码树内链接可解析）"
echo "  - 冒烟测试构建方法见 FFRT独立文档/smoke_test.cpp 文件头注释"
