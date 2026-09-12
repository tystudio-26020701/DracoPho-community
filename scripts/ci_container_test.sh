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

echo "== [5/5] 无头链路功能冒烟（容器内 Xvfb :99） =="
Xvfb :99 -screen 0 800x600x24 >/tmp/xvfb.log 2>&1 &
XVFB_PID=$!
sleep 2
export DISPLAY=:99 QT_QPA_PLATFORM=xcb

smoke() { echo "--- $*"; "$@" > "$SMOKE_OUT" 2>/tmp/smoke-err.log; echo "exit=$?"; }

export XDG_RUNTIME_DIR=/tmp/runtime-root; mkdir -p "$XDG_RUNTIME_DIR"

SMOKE_OUT=/tmp/smoke-doctor.json smoke ./build-ci/dracoPho --doctor
python3 -c "import json;d=json.load(open('/tmp/smoke-doctor.json'));print('doctor: v=%s version=%s qt=%s displays=%d' % (d['v'], d['version'], d['qt']['platform'], len(d['displays'])))"

SMOKE_OUT=/tmp/smoke-displays.json smoke ./build-ci/dracoPho --list-displays
python3 -c "import json;d=json.load(open('/tmp/smoke-displays.json'));print('list-displays: v=%s n=%d' % (d['v'], len(d['displays'])))"

SMOKE_OUT=/tmp/smoke-inline.json smoke ./build-ci/dracoPho --capture-destination inline
python3 -c "
import json,base64
d=json.load(open('/tmp/smoke-inline.json'))
data=d.get('data')
print('inline: v=%s dest=%s %sx%s png_magic=%s' % (d['v'], d['destination'], d['width'], d['height'], base64.b64decode(data)[:8]==b'\x89PNG\r\n\x1a\n' if data else False))"

SMOKE_OUT=/tmp/smoke-file.json smoke ./build-ci/dracoPho --capture-to /tmp --output-name ci-smoke
python3 -c "
import json,os
d=json.load(open('/tmp/smoke-file.json'))
print('file: v=%s dest=%s exists=%s' % (d['v'], d['destination'], os.path.exists(d['path']) if d.get('path') else False))"

smoke ./build-ci/dracoPho --capture-window --capture-to /tmp; [ $? -eq 2 ] && echo "footgun guard: OK (exit 2)"

./build-ci/dracoPho --capture-to /tmp/x.png --delay abc >/dev/null 2>&1; [ $? -eq 2 ] && echo "invalid --delay: OK (exit 2)"

SMOKE_OUT=/tmp/smoke-stage.json smoke ./build-ci/dracoPho --capture-destination stage
python3 -c "
import json
d=json.load(open('/tmp/smoke-stage.json'))
print('stage: v=%s dest=%s path=%s' % (d['v'], d['destination'], d.get('path')))"

kill $XVFB_PID 2>/dev/null

echo "== 容器内流水线完成 =="
