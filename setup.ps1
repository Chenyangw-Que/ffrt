# 还原「FFRT 源码 + 本文档包」的完整环境。
# 用法: .\setup.ps1 [-Target 目标目录]   默认: ffrt-env
# 环境变量:
#   FFRT_REPO    上游仓库地址（默认 OpenHarmony 官方仓）
#   FFRT_COMMIT  上游提交，默认固定为文档基准快照；设为 latest 跟随 master 最新
param([string]$Target = "ffrt-env")

$ErrorActionPreference = "Stop"

$FfrtRepo = if ($env:FFRT_REPO) { $env:FFRT_REPO } else { "https://gitcode.com/openharmony/resourceschedule_ffrt.git" }
$FfrtCommit = if ($env:FFRT_COMMIT) { $env:FFRT_COMMIT } else { "1d559e70feda31fa8afd45d564daa242660e4edd" }
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

if (Test-Path $Target) {
    Write-Error "目标目录 $Target 已存在"
    exit 1
}

Write-Host "克隆上游源码: $FfrtRepo"
git clone $FfrtRepo $Target

Push-Location $Target
if ($FfrtCommit -ne "latest") {
    Write-Host "检出文档基准快照: $FfrtCommit"
    git checkout $FfrtCommit
}
Pop-Location

Write-Host "覆盖文档产物..."
Copy-Item -Recurse "$ScriptDir\FFRT独立文档" "$Target\FFRT独立文档"
Copy-Item "$ScriptDir\guide.md" "$Target\guide.md"
New-Item -ItemType Directory -Force "$Target\docs" | Out-Null
Copy-Item -Recurse "$ScriptDir\docs\module-map" "$Target\docs\module-map"
Copy-Item -Force "$ScriptDir\docs\README.md" "$Target\docs\README.md"

Write-Host ""
Write-Host "完成: $Target"
Write-Host "  - FFRT独立文档\     可独立阅读的学习包"
Write-Host "  - docs\module-map\  仓库内导航（源码树内链接可解析）"
Write-Host "  - 冒烟测试构建方法见 FFRT独立文档\smoke_test.cpp 文件头注释"
