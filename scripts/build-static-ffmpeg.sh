#!/usr/bin/env bash
# 构建静态 FFmpeg 库（Windows MinGW，供 dracoPho 静态单文件 exe 链接）。
#
# 许可策略：dracoPho 是 MIT，静态链接 GPL 库（libx264）会污染分发。本脚本用
# FFmpeg 最小 LGPL 配置（--disable-everything + 内建编码器），不含任何 GPL/外部
# 依赖库：gif/mpeg4/aac 均为 FFmpeg 内建，零外部静态依赖，链接后应用仍为 MIT。
#
# 用法:
#   MSYSTEM=UCRT64 ./scripts/build-static-ffmpeg.sh <source-dir> <install-prefix>
#   <source-dir>   FFmpeg 源码目录（不存在则 git clone n8.1）
#   <install-prefix> 安装前缀（lib/*.a 与 include/ 输出到这里）
set -euo pipefail

SOURCE_DIR="${1:?missing FFmpeg source dir}"
PREFIX="${2:?missing install prefix}"
FFMPEG_BRANCH="n8.1"

if [ ! -f "$SOURCE_DIR/configure" ]; then
    echo "Cloning FFmpeg $FFMPEG_BRANCH ..."
    git clone --depth 1 --branch "$FFMPEG_BRANCH" \
        https://github.com/FFmpeg/FFmpeg.git "$SOURCE_DIR"
fi

mkdir -p "$PREFIX"
cd "$SOURCE_DIR"

./configure \
    --prefix="$PREFIX" \
    --enable-static \
    --disable-shared \
    --disable-programs \
    --disable-doc \
    --disable-debug \
    --disable-network \
    --disable-zlib \
    --disable-bzlib \
    --disable-lzma \
    --disable-x86asm \
    --disable-everything \
    --enable-avcodec \
    --enable-avformat \
    --enable-avutil \
    --enable-swresample \
    --enable-swscale \
    --enable-encoder=gif \
    --enable-encoder=mpeg4 \
    --enable-encoder=aac \
    --enable-muxer=gif \
    --enable-muxer=mp4 \
    --enable-muxer=mov \
    --enable-muxer=image2 \
    --enable-muxer=ipod \
    --enable-protocol=file \
    --enable-protocol=pipe \
    --enable-decoder=gif \
    --enable-decoder=mpeg4 \
    --enable-decoder=aac \
    --enable-parser=aac \
    --enable-parser=gif

make -j"$(nproc)"
make install

echo "Static FFmpeg installed to: $PREFIX"
ls -la "$PREFIX/lib"/*.a
