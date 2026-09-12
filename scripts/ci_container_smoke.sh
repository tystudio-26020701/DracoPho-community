#!/usr/bin/env bash
# 容器内无头链路功能冒烟（B 契约：触发器/修饰符严格分离，退出码 0/1/2）。
dracoPho=./build-ci/dracoPho
set -u
cd /src
Xvfb :99 -screen 0 800x600x24 >/tmp/xvfb.log 2>&1 &
sleep 2
export DISPLAY=:99 QT_QPA_PLATFORM=xcb XDG_RUNTIME_DIR=/tmp/runtime-root
mkdir -p "$XDG_RUNTIME_DIR"
smoke() { echo "--- $*"; "$@" > "$SMOKE_OUT" 2>/tmp/smoke-err.log; echo "exit=$?"; }

expect2() { # expect2 <说明> <参数...>
    local label="$1"; shift
    "$dracoPho" "$@" >/dev/null 2>&1
    [ $? -eq 2 ] && echo "$label: OK (exit 2)" || echo "$label: FAIL"
}

SMOKE_OUT=/tmp/smoke-doctor.json smoke $dracoPho --doctor
python3 -c "import json;d=json.load(open('/tmp/smoke-doctor.json'));print('doctor: v=%s version=%s qt=%s displays=%d' % (d['v'], d['version'], d['qt']['platform'], len(d['displays'])))"

SMOKE_OUT=/tmp/smoke-displays.json smoke $dracoPho --list-displays
python3 -c "import json;d=json.load(open('/tmp/smoke-displays.json'));print('list-displays: v=%s n=%d' % (d['v'], len(d['displays'])))"

echo "== B 契约：显式触发的屏幕捕获 =="
SMOKE_OUT=/tmp/smoke-inline.json smoke $dracoPho --capture-screen --capture-destination inline
python3 -c "
import json,base64
d=json.load(open('/tmp/smoke-inline.json'))
data=d.get('data')
print('inline: v=%s dest=%s %sx%s png_magic=%s' % (d['v'], d['destination'], d['width'], d['height'], base64.b64decode(data)[:8]==b'\x89PNG\r\n\x1a\n' if data else False))"

SMOKE_OUT=/tmp/smoke-file.json smoke $dracoPho --capture-to /tmp --output-name ci-smoke
python3 -c "
import json,os
d=json.load(open('/tmp/smoke-file.json'))
print('file: v=%s dest=%s exists=%s' % (d['v'], d['destination'], os.path.exists(d['path']) if d.get('path') else False))"

SMOKE_OUT=/tmp/smoke-stage.json smoke $dracoPho --capture-screen --capture-destination stage
python3 -c "
import json
d=json.load(open('/tmp/smoke-stage.json'))
print('stage: v=%s dest=%s path=%s' % (d['v'], d['destination'], d.get('path')))"

$dracoPho --capture-to /tmp/dl.png --delay 1 >/dev/null 2>&1
[ $? -eq 0 ] && echo "headless --delay 1: OK" || echo "headless --delay 1: FAIL"

echo "== B 契约：修饰符离开触发器/非法组合一律退出 2 =="
expect2 "inline alone" --capture-destination inline
expect2 "stage alone" --capture-destination stage
expect2 "file alone" --capture-destination file
expect2 "bogus destination" --capture-destination bogus
expect2 "region alone" --region 0,0,10,10
expect2 "clipboard on screen path" --capture-destination clipboard --capture-to /tmp
expect2 "capture-screen without destination" --capture-screen
expect2 "capture-screen with file" --capture-screen --capture-destination file
expect2 "capture-screen with clipboard" --capture-screen --capture-destination clipboard
expect2 "capture-screen with capture-to" --capture-screen --capture-to /tmp/x.png
expect2 "stage with capture-to (redundant)" --capture-destination stage --capture-to /tmp
expect2 "capture-window footgun" --capture-window --capture-to /tmp
expect2 "invalid --delay" --capture-to /tmp/x.png --delay abc
expect2 "doctor + capture" --doctor --capture-to /tmp/x.png

# 多显示器 inline：Xvfb + XINERAMA 双屏尽力构造两个 QScreen；构造失败则
# 如实记 SKIP（该路径与 file 去向共用分发逻辑，单屏环境无法真实复现）。
pkill -f "Xvfb :99" 2>/dev/null; sleep 1
Xvfb :99 +extension XINERAMA -screen 0 800x600x24 -screen 1 640x480x24 >/tmp/xvfb2.log 2>&1 &
sleep 2
$dracoPho --list-displays > /tmp/smoke-multi-list.json 2>/dev/null
python3 - <<'PY'
import json, subprocess, base64
names = [d["name"] for d in json.load(open("/tmp/smoke-multi-list.json"))["displays"] if d.get("name")]
if len(names) >= 2:
    out = subprocess.run(["./build-ci/dracoPho", "--capture-screen", "--capture-destination", "inline",
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
