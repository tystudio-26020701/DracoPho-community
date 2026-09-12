#!/usr/bin/env bash
# dracoPho 容器内测试流水线（Ubuntu 24.04 / 26.04 本地 ISO 派生镜像）。
# 在容器内执行：安装编译依赖 → 全量构建 → 全量 ctest → 无头链路功能冒烟。
# 显示面一律使用容器内 offscreen/Xvfb，绝不触碰宿主会话。
set -uo pipefail

export DEBIAN_FRONTEND=noninteractive
cd /src

echo "== [1/5] 安装编译依赖（与 GitHub CI 依赖清单一致 + Xvfb） =="
apt-get update -qq
apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build pkg-config \
    qt6-base-dev qt6-base-dev-tools \
    libgl1-mesa-dev libx11-xcb-dev libxcb1-dev libxcomposite-dev \
    xvfb xauth > /tmp/apt-install.log 2>&1
echo "apt exit=$? (详见 /tmp/apt-install.log)"

echo "== [2/5] CMake 配置（与 CI 一致） =="
cmake -S . -B build-ci -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCMAKE_INSTALL_PREFIX=/usr

echo "== [3/5] 全量构建 =="
cmake --build build-ci -j"$(nproc)" 2>&1 | tail -3

echo "== [4/5] 全量 ctest（offscreen） =="
QT_QPA_PLATFORM=offscreen ctest --test-dir build-ci --output-on-failure 2>&1 | tail -15

echo "== [5/5] 无头链路功能冒烟（容器内 Xvfb :99，复用冒烟脚本） =="
export SMOKE_OUT=/tmp/smoke-last.json
bash scripts/ci_container_smoke.sh

echo "== 容器内流水线完成 =="
