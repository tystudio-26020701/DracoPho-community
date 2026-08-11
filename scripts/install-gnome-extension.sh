#!/usr/bin/env bash
# 安装/更新 GNOME Shell 扩展 mark-shot-scroll-helper@snemc.org。
#
# 首选安装到用户指定的工具目录（默认 /run/media/.../Tools），并把该目录加入
# 系统级 XDG_DATA_DIRS（/etc/environment，需 root）——GNOME Shell 按 XDG_DATA_DIRS
# 顺序查找 <dir>/gnome-shell/extensions/<uuid>，Tools 目录排在最前，优先生效，
# 无需 root 写 /usr/share。实机当前会话的 gnome-shell 启动时已读取旧 XDG_DATA_DIRS，
# 需重新登录（或 Xorg 下重启 shell）后新扩展才被加载。
#
# 用法:
#   MARK_SHOT_TOOLS_BASE=/path/to/Tools scripts/install-gnome-extension.sh
#   # 环境变量未指定时默认安装到 /run/media/.../Tools，并提示需要 root 持久化。
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$REPO_DIR/packaging/gnome-extension/mark-shot-scroll-helper@snemc.org"
UUID="mark-shot-scroll-helper@snemc.org"

# 默认工具目录：探测当前工作树旁的工具集（与构建环境同一磁盘）。
if [ -n "${MARK_SHOT_TOOLS_BASE:-}" ]; then
    TOOLS_BASE="$MARK_SHOT_TOOLS_BASE"
elif [ -d "/run/media/lcz/b9694bf8-68f6-456d-bb43-03f8d2d9eec2/Tools" ]; then
    TOOLS_BASE="/run/media/lcz/b9694bf8-68f6-456d-bb43-03f8d2d9eec2/Tools"
else
    echo "无法确定工具目录，请设置 MARK_SHOT_TOOLS_BASE" >&2
    exit 1
fi

SHARE_DIR="$TOOLS_BASE/share"
DST="$SHARE_DIR/gnome-shell/extensions/$UUID"

if [ ! -f "$SRC/extension.js" ] || [ ! -f "$SRC/metadata.json" ]; then
    echo "扩展源文件缺失: $SRC" >&2
    exit 1
fi

mkdir -p "$DST"
install -m 644 \
    "$SRC/extension.js" \
    "$SRC/scroll-preview-overlay.js" \
    "$SRC/metadata.json" \
    "$DST/"
echo "已安装扩展到: $DST"

# 持久化 XDG_DATA_DIRS（把 Tools/share 置前，保证优先于 /usr/share 等系统目录）。
ENV_FILE="/etc/environment"
VAR_NAME="XDG_DATA_DIRS"
DESIRED="$SHARE_DIR:/usr/local/share/:/usr/share/"

if [ -w "$ENV_FILE" ]; then
    # 直接改写（理论上仅在无 root 归属的容器/特殊环境出现）。
    if grep -q "^$VAR_NAME=" "$ENV_FILE"; then
        sed -i "s|^$VAR_NAME=.*|$VAR_NAME=\"$DESIRED${XDG_DATA_DIRS:+:$XDG_DATA_DIRS}\"|" "$ENV_FILE"
    else
        printf '%s="%s"\n' "$VAR_NAME" "$DESIRED" >> "$ENV_FILE"
    fi
    echo "已写入 $ENV_FILE: $VAR_NAME=$DESIRED"
else
    echo
    echo "需要 root 权限持久化系统环境变量（把 Tools/share 置入 XDG_DATA_DIRS）："
    echo "  sudo sh -c 'grep -q \"^$VAR_NAME=\" $ENV_FILE && sed -i \"s|^$VAR_NAME=.*|$VAR_NAME=\\\"$DESIRED\\\"|\" $ENV_FILE || printf \\\"%s=\\\\\\\"%s\\\\\\\"\\\\n\\\" $VAR_NAME $DESIRED >> $ENV_FILE'"
    echo "完成后请重新登录（或 Xorg 下 gnome-shell --replace）让 gnome-shell 生效。"
fi

# 尝试重载当前会话的扩展（仅当当前 gnome-shell 已按新路径找到该扩展时有效）。
if command -v gnome-extensions >/dev/null 2>&1; then
    gnome-extensions reset "$UUID" 2>/dev/null || true
    gnome-extensions enable "$UUID" 2>/dev/null || true
    echo "已尝试 gnome-extensions 重载。若未生效，请重新登录后验证。"
else
    echo "未找到 gnome-extensions 工具，请重新登录以加载扩展。"
fi
