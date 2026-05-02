# Mark Shot

[English README](README.md)

`mark-shot` 是一个基于 Qt 6 的 Wayland 截图标注工具，适合 niri 等窗口管理器使用。它通过 `grim` 获取冻结的屏幕画面，打开全屏标注界面，然后支持选区、标注、保存、复制，或使用其他桌面应用打开处理后的图片。

## 功能

- 默认捕获当前输出，可通过 `--all-outputs` 捕获整个合成器画面。
- 支持区域选择、全屏标注，以及选区移动和缩放。
- 提供画笔、直线、荧光笔、矩形、椭圆、箭头、文本、编号和马赛克工具。
- 支持撤销、重做、保存、复制到 Wayland 剪贴板，以及使用其他应用打开。
- 默认使用 layer-shell 覆盖层，也可通过 `--xdg-window` 使用普通全屏 xdg 窗口。
- 面向支持 `wlr-screencopy` 的 Wayland 合成器，尤其适合 niri。

## 编译

Arch Linux 依赖：

```bash
sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-wayland layer-shell-qt grim wl-clipboard
```

编译并运行：

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/mark-shot
```

安装到本地：

```bash
cmake --install build --prefix "$HOME/.local"
```

## 使用

捕获当前输出：

```bash
mark-shot
```

捕获所有输出：

```bash
mark-shot --all-outputs
```

跳过选区步骤，直接标注完整截图：

```bash
mark-shot --fullscreen
```

使用普通全屏 xdg 窗口而不是 layer-shell：

```bash
mark-shot --xdg-window
```

niri 快捷键示例：

```kdl
Mod+Shift+S { spawn "mark-shot"; }
```

## 控制

- 拖动鼠标创建选区。
- `V`、`P`、`L`、`H`、`R`、`E`、`A`、`T`、`N`、`M` 分别切换到移动、画笔、直线、荧光笔、矩形、椭圆、箭头、文本、编号和马赛克。
- 移动工具可拖动选区，也可从边缘和角落调整选区尺寸。
- 绘制矩形或椭圆时按住 `Ctrl`，可约束为正方形或圆形。
- 右键打开径向调色板。
- 鼠标滚轮调整当前工具的线宽、编号尺寸、文字尺寸或马赛克块大小。
- `Ctrl+Z` 撤销，`Ctrl+Shift+Z` 或 `Ctrl+Y` 重做。
- `Ctrl+C` 复制编辑后的选区。
- `Ctrl+S` 或 `Enter` 保存编辑后的选区。
- `Esc` 退出。

## 许可证

MIT License。
