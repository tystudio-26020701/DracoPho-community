#!/usr/bin/env bash
# 容器内无头链路功能冒烟（由 ci_container_test.sh 第 5 阶段拆出，供复跑）。
set -u
cd /src
Xvfb :99 -screen 0 800x600x24 >/tmp/xvfb.log 2>&1 &
sleep 2
export DISPLAY=:99 QT_QPA_PLATFORM=xcb XDG_RUNTIME_DIR=/tmp/runtime-root
mkdir -p "$XDG_RUNTIME_DIR"
smoke() { echo "--- $*"; "$@" > "$SMOKE_OUT" 2>/tmp/smoke-err.log; echo "exit=$?"; }

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

./build-ci/dracoPho --capture-window --capture-to /tmp >/dev/null 2>&1
[ $? -eq 2 ] && echo "footgun guard: OK (exit 2)" || echo "footgun guard: FAIL"

./build-ci/dracoPho --capture-to /tmp/x.png --delay abc >/dev/null 2>&1
[ $? -eq 2 ] && echo "invalid --delay: OK (exit 2)" || echo "invalid --delay: FAIL"

./build-ci/dracoPho --capture-destination clipboard --capture-to /tmp >/dev/null 2>&1
[ $? -eq 2 ] && echo "clipboard rejection: OK (exit 2)" || echo "clipboard rejection: FAIL"

SMOKE_OUT=/tmp/smoke-stage.json smoke ./build-ci/dracoPho --capture-destination stage
python3 -c "
import json
d=json.load(open('/tmp/smoke-stage.json'))
print('stage: v=%s dest=%s path=%s' % (d['v'], d['destination'], d.get('path')))"

./build-ci/dracoPho --capture-to /tmp/dl.png --delay 1 >/dev/null 2>&1
[ $? -eq 0 ] && echo "headless --delay 1: OK" || echo "headless --delay 1: FAIL"

kill %1 2>/dev/null
echo "== 冒烟完成 =="
