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

./build-ci/dracoPho --capture-destination file >/dev/null 2>&1
[ $? -eq 2 ] && echo "file-alone requires --capture-to: OK (exit 2)" || echo "file-alone requires --capture-to: FAIL"

./build-ci/dracoPho --capture-destination bogus >/dev/null 2>&1
[ $? -eq 2 ] && echo "bogus destination rejected: OK (exit 2)" || echo "bogus destination rejected: FAIL"

./build-ci/dracoPho --doctor --capture-to /tmp/x.png >/dev/null 2>&1
[ $? -eq 2 ] && echo "doctor + capture rejected: OK (exit 2)" || echo "doctor + capture rejected: FAIL"

# 多显示器 inline：Xvfb + XINERAMA 双屏尽力构造两个 QScreen；构造失败则
# 如实记 SKIP（该路径与 file 去向共用分发逻辑，单屏环境无法真实复现）。
pkill -f "Xvfb :99" 2>/dev/null; sleep 1
Xvfb :99 +extension XINERAMA -screen 0 800x600x24 -screen 1 640x480x24 >/tmp/xvfb2.log 2>&1 &
sleep 2
./build-ci/dracoPho --list-displays > /tmp/smoke-multi-list.json 2>/dev/null
python3 - <<'PY'
import json, subprocess, base64
names = [d["name"] for d in json.load(open("/tmp/smoke-multi-list.json"))["displays"] if d.get("name")]
if len(names) >= 2:
    out = subprocess.run(["./build-ci/dracoPho", "--capture-destination", "inline",
                          "--display", names[0], "--display", names[1]],
                         capture_output=True, text=True)
    root = json.loads(out.stdout)
    caps = root.get("captures", [])
    ok = (out.returncode == 0 and len(caps) == 2
          and all(base64.b64decode(c["data"])[:8] == b"\x89PNG\r\n\x1a\n" for c in caps if c.get("data")))
    print("multi-display inline: %s (screens=%s)" % ("OK" if ok else "FAIL", names))
else:
    print("multi-display inline: SKIP (Xinerama yielded %d QScreen)" % len(names))
PY

kill %1 2>/dev/null
echo "== 冒烟完成 =="
