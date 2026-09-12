#!/usr/bin/env bash
# dracoPho Windows 一次性构建+测试脚本（MSYS2 UCRT64，步骤镜像 .github/workflows/build.yml）
# 用法：在 Windows VM 的 "MSYS2 UCRT64" 终端中，进入源码根目录执行：
#   bash scripts/windows_one_shot_test.sh 2>&1 | tee windows-test.log
# 产物：build/ 构建树、ctest 结果、末尾的冒烟小结；把 windows-test.log 交回即可审计。
set -uo pipefail

cd "$(dirname "$0")/.."

PKGS=(
    git mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-cppwinrt
    mingw-w64-ucrt-x86_64-ffmpeg mingw-w64-ucrt-x86_64-gcc
    mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-onnxruntime
    mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-tools
    mingw-w64-ucrt-x86_64-zxing-cpp
)

echo "== [0/4] 预检：工具链与依赖 =="
missing=()
for t in cmake ninja g++; do command -v "$t" >/dev/null || missing+=("$t"); done
if command -v pacman >/dev/null; then
    for p in "${PKGS[@]}"; do pacman -Q "$p" >/dev/null 2>&1 || missing+=("$p"); done
fi
if [ ${#missing[@]} -gt 0 ]; then
    echo "缺失: ${missing[*]}"
    if command -v pacman >/dev/null; then
        echo "尝试在线补装（需要网络）..."
        pacman -S --needed --noconfirm "${PKGS[@]}" || {
            echo "预检失败：无法补装依赖。请把上面缺失清单连同本日志交回。" >&2
            exit 2
        }
    else
        echo "预检失败：未找到 pacman（请确认在 MSYS2 UCRT64 shell 中运行）。" >&2
        exit 2
    fi
else
    echo "依赖齐全"
fi

echo "== [1/4] CMake 配置（与 CI 完全一致） =="
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON \
    -DMARK_SHOT_REQUIRE_FFMPEG=ON \
    -DMARK_SHOT_WITH_LAYER_SHELL=OFF \
    -DMARK_SHOT_WITH_LIBPORTAL=OFF || exit 2

echo "== [2/4] 构建 =="
cmake --build build --parallel || exit 2

echo "== [3/4] 全量 ctest（offscreen） =="
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
TEST_RC=$?

echo "== [4/4] 无头 CLI 冒烟（真实 RDP 会话显示面） =="
BIN=./build/dracoPho.exe
[ -f "$BIN" ] || BIN=./build/bin/dracoPho.exe
"$BIN" --doctor > /tmp/w-doctor.json 2>&1 && echo "doctor: exit=0" || echo "doctor: exit=$?"
"$BIN" --list-displays > /tmp/w-displays.json 2>&1 && echo "list-displays: exit=0" || echo "list-displays: exit=$?"
"$BIN" --capture-destination inline > /tmp/w-inline.json 2>&1 && echo "inline: exit=0" || echo "inline: exit=$?"
"$BIN" --capture-window --capture-to /tmp >/dev/null 2>&1 && echo "footgun: UNEXPECTED exit=0" || echo "footgun: exit=$? (expect 2)"

echo "== 完成：ctest 退出码 $TEST_RC；请交回 windows-test.log 与 /tmp/w-*.json =="
exit $TEST_RC
