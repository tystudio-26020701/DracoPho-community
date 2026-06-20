#!/usr/bin/env bash
# 从仓库根目录生成源码包并在本目录执行 makepkg
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../.." && pwd)"
pkgver="$(grep -E '^pkgver=' "${script_dir}/PKGBUILD" | cut -d= -f2)"
pkgname="$(grep -E '^pkgname=' "${script_dir}/PKGBUILD" | cut -d= -f2)"
archive="${script_dir}/${pkgname}-${pkgver}.tar.gz"
tag="v${pkgver}"

usage() {
    cat <<EOF
用法: $(basename "$0") [选项]

  默认: 生成 ${pkgname}-${pkgver}.tar.gz 后执行 makepkg -si

选项:
  --tar-only    只生成源码压缩包
  --no-install  执行 makepkg -s（构建并安装到 pkg/，不 pacman -U）
  -h, --help    显示本说明
EOF
}

tar_only=0
makepkg_args=(-si)

while [[ $# -gt 0 ]]; do
    case "$1" in
        --tar-only) tar_only=1; shift ;;
        --no-install) makepkg_args=(-s); shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "未知参数: $1" >&2; usage >&2; exit 1 ;;
    esac
done

cd "${repo_root}"
if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "错误: 未在 git 仓库内: ${repo_root}" >&2
    exit 1
fi

if git rev-parse "${tag}" >/dev/null 2>&1; then
    ref="${tag}"
else
    echo "提示: 未找到标签 ${tag}，使用当前 HEAD 打源码包" >&2
    ref=HEAD
fi

rm -f "${archive}"
git archive --format=tar.gz --prefix="${pkgname}-${pkgver}/" "${ref}" -o "${archive}"
echo "已生成: ${archive}"

if [[ "${tar_only}" -eq 1 ]]; then
    exit 0
fi

cd "${script_dir}"
makepkg "${makepkg_args[@]}"