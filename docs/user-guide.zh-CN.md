# 太殷龙摄 用户操作手册

本文档介绍 太殷龙摄 的日常使用，重点覆盖**指定窗口/组件悬停框选截图**功能
（鼠标移动时自动跟踪并高亮光标下方的窗口，单击即可选中该窗口）、标注工作流、
headless 截图与配置。

> 本仓库的文档在社区版 fork 中编写，并镜像到上游与商业版仓库。商业版额外包含
> 针对其本地 MCP server 的专门章节。

---

## 1. 快速开始

### 1.1 启动

开始一次区域截图会话：

```bash
dracoPho --capture
```

或按配置的启动行为启动（见 § 1.3）。按下桌面快捷键（见 § 8）或在终端运行。
焦点显示器上会打开一个冻结的全屏覆盖层。移动鼠标拉出选择矩形，松开后进入
标注编辑器。

### 1.2 便携版应用

如果使用便携版（`mark-shot-upstream` / `mark-shot-community` /
`mark-shot-enterprise`），请用自带的启动脚本启动，以便找到内置的 Qt 库、
插件与辅助脚本：

```bash
portable/mark-shot-community/bin/run-mark-shot.sh
```

启动脚本会把其 `bin/` 目录加入 `PATH`，这是窗口检测脚本
（`dracoPho-window-detection-*`）以及 OCR / 上传辅助脚本能够被找到的必要条件。

### 1.3 启动行为（悬浮球 / 托盘 / 设置窗口 / 直接截图）

启动 太殷龙摄（例如点击桌面图标）后会发生什么，是可配置的。打开
**设置 → 通用 → 启动行为**，任意勾选组合：

| 选项 | 启动时行为 |
| :--- | :--- |
| **托盘图标**（默认） | 太殷龙摄 在系统托盘保持后台运行。左键单击托盘图标快速截图，右键打开菜单。 |
| **悬浮球**（默认） | 屏幕右下角出现一个可拖动的置顶小球。单击弹出快捷菜单，双击直接截图，拖动可改变位置（位置会被记住）。截图或录制进行中悬浮球自动隐藏，不会出现在你自己的截图或录制画面里。 |
| **直接截图** | 启动后立即进入截图模式。 |
| **设置窗口** | 启动 太殷龙摄 时打开设置窗口。 |

多个选项可组合使用。**直接截图默认关闭**；除非勾选它，否则启动 太殷龙摄
不会自动弹出截图覆盖层。若所有选项都被取消，太殷龙摄 会回退为托盘运行，
保证始终有入口。

悬浮球与托盘菜单提供相同的快捷操作：**截图**、**捕获窗口**、**全屏截图**、
**延时截图**、**开始录制**、**截图历史**、**设置**、**退出**。悬浮球菜单
额外提供**隐藏悬浮球**，可将其隐藏到下次启动。悬浮球在所有平台上都保持
置顶；GNOME Wayland（合成器不遵守置顶提示）通过随软件附带的
MarkShotScrollHelper Shell 扩展实现置顶。

---

## 2. 窗口 / 组件悬停框选

太殷龙摄 在选区开始前会检测当前桌面的窗口。覆盖层打开后，**移动鼠标即可在
光标所在窗口上高亮显示青色边框**；**直接单击（不拖动）即可选中整个窗口**作为
截图区域，随后可直接标注、复制、钉住或保存。

高亮的窗口来自按合成器区分的检测脚本，它们在覆盖层出现前运行：

| 桌面 | 检测来源 | 说明 |
| :--- | :--- | :--- |
| GNOME Wayland | 内置 `mark-shot-scroll-helper@snemc.org` Shell 扩展（D-Bus） | 需要启用扩展（见 § 2.1） |
| KDE Plasma Wayland | 通过 `qdbus6` / `qdbus` + journalctl 的一次性 KWin 脚本 | 需要 KWin 会话 |
| Hyprland | `hyprctl -j clients` | |
| niri | `niri msg -j windows` + 配置解析 | |
| X11 | 进程内 XCB 枚举 `_NET_CLIENT_LIST_STACKING` | 无需脚本 |
| Windows | 进程内 `EnumWindows` | 无需脚本 |

只跟踪**顶层窗口**。窗口内部的单个控件（“组件”）在 Wayland 合成器中不暴露，
因此所有平台上的悬停框选都针对整个窗口。

### 2.1 GNOME Wayland：启用辅助扩展

```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```

验证 D-Bus 辅助接口可用：

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
# -> ('5',)
```

如果调用失败，请注销后重新登录（X11 上重启 GNOME Shell）再试。没有扩展时
GNOME 检测脚本会报错退出，悬停框选保持关闭（普通拖拽框选不受影响）。

### 2.2 使用方法

1. 触发截图（`dracoPho` 或桌面快捷键）。
2. 不按任何鼠标键，把光标移到某个窗口上，出现青色边框标示将被选中的窗口。
3. **单击一次**（按下并松开、位移不超过几像素）即可选中该窗口。窗口重叠时，
   优先选中光标下最顶层的窗口（按 z 序）。
4. 松开后进入标注编辑器，窗口刚好被精确框选。
5. 如果只想做**手动**框选，正常拖动矩形即可——一旦拖动超过单击阈值，悬停框
   即被忽略。

当取色器（`C`）或标尺（`R`）启动工具激活时，悬停高亮被禁用；二维码扫描（`Q`）、
显示器快照（`D`）以及 GIF / 视频录制的启动模式仍可使用。

### 2.3 多显示器下正确选择窗口

窗口检测按每个捕获目标分别执行。在多显示器环境，每个冻结窗口只接收与其自身
几何相交的窗口，因此悬停框与你在该显示器上看到的内容一致。

### 2.4 启用 / 禁用

该功能默认开启（`windowDetection.enabled = true`）。可在
**设置 → 高级 → 窗口检测已启用** 中切换，或编辑 `~/.config/dracoPho/config.json`：

```json
{
  "windowDetection": {
    "enabled": true,
    "command": "dracoPho-window-detection-gnome",
    "timeoutMs": 1000,
    "env": {}
  }
}
```

- `command`：检测脚本。GNOME / KDE / Hyprland / niri Wayland 下会自动选择与
  当前会话匹配的内置 `dracoPho-window-detection-*` 脚本；X11 与 Windows 在
  进程内枚举平台，`command` 可留空。**用户自定义命令（例如绝对路径）始终会被
  尊重，不会被覆盖。**
- `timeoutMs`：等待脚本的最长时间（100–30000 ms，默认 1000）。
- `env`：传给脚本的额外环境变量。各合成器的微调（偏移量等）见脚本头注释。

### 2.5 故障排查

| 现象 | 检查项 |
| :--- | :--- |
| GNOME Wayland 下没有青色框 | 扩展是否启用？上面 `gdbus` 调用必须返回版本号 |
| X11 / Windows 下没有青色框 | 平台枚举是内置的，无需操作；确认没有启用取色器 / 标尺启动工具 |
| 悬停框选到了错误的（下层）窗口 | 自定义检测脚本缺少 z 序数据；没有 `zOrder` 的窗口按最底层处理 |
| 截图启动变慢 | 检测脚本在覆盖层之前运行；只有桌面响应慢才需要调大 `timeoutMs`，或设 `enabled:false` 跳过 |
| 查看诊断 | 运行 `dracoPho --debug --debug-log /tmp/dracoPho.log`，查找 `window-detection` 日志行 |

---

## 3. 区域选择与启动工具

提交区域之前可使用以下启动工具：

| 快捷键 | 工具 | 行为 |
| :---: | :--- | :--- |
| `C` | 取色器 | 采样像素；滚轮缩放放大镜；左键打开颜色面板（HEX / RGB / HSL / HSV / Qt 格式）；右键或 `Esc` 退出 |
| `R` | 标尺 | 悬停读取像素坐标；左键拖动测量矩形（宽、高、对角线、面积）；右键或 `Esc` 退出 |
| `Q` | 二维码扫描 | 圈选二维码 / 条形码区域；解码结果在可复制窗口中打开 |
| `W` | 窗口捕获 | 悬停高亮窗口（显示标题徽标）、点击即捕获并进入编辑器；X11/XWayland（XComposite 合成缓冲）、Windows（`PrintWindow(PW_RENDERFULLCONTENT)`）、KDE Wayland 原生窗口（KWin ScreenShot2 `CaptureWindow`）读取窗口**自身内容**，被遮挡/最小化窗口同样真实；其余平台截取窗口所在屏幕区域 |
| `D` | 显示器快照 | 捕获全部输出、按显示器裁剪并显示可悬停缩略图（复制 / 编辑 / 保存） |
| `S` | 停止录制 | 停止覆盖层中显示的 GIF / 视频录制 |

`Esc` 取消会话；右键（无启动工具时）同样取消。

#### 3.1 多显示器冻结行为

使用默认的 **Freeze All Screens**（冻结全部屏幕）选区范围时，选区期间所有已连接的
显示器都会冻结。在某台显示器完成选区后，其余显示器保持显示其冻结画面并作为不可
操作的背景：鼠标、键盘、滚轮与快捷键输入均被拦截，覆盖层不显示任何工具栏，整张
虚拟桌面保持冻结直到截图会话结束。若改用 **Cursor Screen**（光标所在屏幕）范围
（设置 → 截图 → 冻结范围），则仅光标所在显示器被冻结，其余屏幕保持完全可用。

---

## 4. 标注工具

选中区域（或打开本地图片）后进入编辑器，显示标注工具栏。工具可用数字键或
工具栏切换：

| 快捷键 | 工具 | 说明 |
| :---: | :--- | :--- |
| `V` | 移动 / 平移 | 移动整个选区；本地图片模式下平移画布 |
| `S` | 选择 | 选择、移动、缩放、旋转、删除已有标注 |
| `P` | 画笔 | 平滑手绘笔迹 |
| `L` | 直线 | 直线 |
| `H` | 荧光笔 | 半透明标记；支持手绘或直线样式 |
| `R` | 矩形 | 支持 `描边` / `高亮` / `反色` 三种样式与圆角 |
| `E` | 椭圆 | 椭圆 / 正圆 |
| `A` | 箭头 | 经典箭头（实心、KDE、双向） |
| `T` | 文字 | 富文本；滚轮或滑块调节大小；对角手柄等比缩放、边侧手柄调节换行宽度；字体面板支持精确字号、字体族、粗体 / 斜体 |
| `N` | 序号 | 自动递增的编号标记（阿拉伯、字母、罗马、中文等） |
| `M` | 马赛克 | 亚克力磨砂模糊，用于遮挡敏感信息 |
| `G` | 激光 | 自动消散的临时笔迹 |

绘制技巧：

- 绘制矩形 / 椭圆时按住 `Ctrl` 约束为正圆 / 正方形。
- 工具激活时滚动滚轮可动态调节描边宽度、文字大小、编号缩放或马赛克块大小
  （实时预览）。
- `选择` 工具下滚动滚轮缩放画布，按住中键平移；双击 `Ctrl` 重置。

### 4.1 编辑已有标注

切换到**选择**（`S`）。点击标注显示控制手柄：

- 内部拖动：移动；
- 拖动角 / 边手柄：缩放；
- 拖动上边缘外侧的圆形手柄：旋转；
- 按 `Delete` / `Backspace`：删除；
- 双击文字：就地编辑。

右侧属性面板编辑选中标注：颜色、宽度、样式、文字字体 / 字号 / 粗体 / 斜体。
`选择` 工具下拖出选框可多选，多选后可整体移动、缩放、旋转、删除。

### 4.2 动作

| 快捷键 | 动作 |
| :--- | :--- |
| `Ctrl+C` | 复制到剪贴板 |
| `Ctrl+S` / `Enter` | 保存（路径模板来自设置） |
| `Ctrl+P` | 钉住为悬浮贴纸窗口 |
| `Ctrl+U` | 上传到配置的图床；返回的 URL 自动复制 |
| `Ctrl+Z` / `Ctrl+Y` | 撤销 / 重做 |
| `F` | 切换捕获范围（选区 ↔ 全屏） |

### 4.3 导出相框

开启 **设置 → 导出 → 苹果风格相框** 后，保存 / 复制 / 上传的图片会带上透明
内边距、圆角与柔和阴影。

---

## 5. 钉住的贴纸窗口

| 手势 / 快捷键 | 行为 |
| :--- | :--- |
| 左键拖动 | 移动贴纸 |
| 滚轮 | 等比缩放 |
| 双击左键 / `Esc` | 关闭 |
| 右键 | 上下文菜单（旋转、缩放、置顶、复制文字、翻译、保存、复制、关闭） |

贴纸窗口内的 OCR 文字可直接选择复制（`Ctrl+C` / 右键菜单）。翻译
（OpenAI 兼容接口）会把译文按原布局位置渲染回图片上。

---

## 6. 长截图（滚动截图）

1. 选择区域（超大区域会显示浮动拖拽手柄）。
2. 覆盖层滚动目标窗口，捕获的帧被拼接为长图。
3. GNOME Wayland 需要 太殷龙摄 Scroll Helper 扩展（§ 2.1）。

滚动截图在 niri 及类似的 wlroots/Wayland 合成器上已可稳定使用；在 KDE、X11
等环境属于测试特性。失败时可改用普通截图或自定义扩展命令。

---

## 7. Headless 截图（CLI）

非交互截图会写入 PNG 并输出 JSON：

```bash
# 主屏
dracoPho --capture-to /tmp/shot.png

# 目录（自动生成时间戳文件名）
dracoPho --capture-to /tmp/shots/

# 区域
dracoPho --capture-to /tmp/r.png --region 0,0,1280,720

# 指定显示器，包含鼠标
dracoPho --capture-to /tmp/w.png --display DP-1 --include-cursor

# 一次捕获多个显示器（每个一张 PNG）
dracoPho --capture-to /tmp/shots/ --display DP-1 --display DP-2

# 列出输出
dracoPho --list-displays
```

所有 headless 选项与位置参数（图片文件）互斥。完整参数表见 README。

无人值守录制：无需打开录制对话框，即可通过正在运行的实例开始录制。
`dracoPho --record-region 0,0,640,480 --record-output ~/Videos/clip.mp4
--record-duration 30 --record-wait-json` 会录制该区域 30 秒并输出最终的 JSON
状态。使用 `--record-display <id>` 录制整个显示器，`--record-format webp`
生成动画 WebP（支持 GIF/MP4/WebP），`--record-audio` 收录系统音频，
`--record-duration 0` 则一直录到 `--stop-recording` 为止。

### 7.1 无头窗口 / 组件截图

太殷龙摄 可以**不打开任何界面**，从脚本、构建流水线或智能体直接截取**指定
窗口——或窗口内的组件（子区域）**。进程在写出或返回图片后立即退出，从不创建
窗口、从不弹对话框、从不抢占焦点，因此用户可继续正常操作桌面，捕获在后台
无感完成。

先列出窗口看看有哪些可选：

```bash
dracoPho --list-windows
```

示例输出（GNOME Wayland）：

```json
{"count":2,"platform":"wayland","source":"compositor-script","windows":[
  {"index":0,"id":"0x3c00007","title":"太殷龙摄 - VSCodium","class":"codium","instance":"codium","x":1920,"y":0,"width":1680,"height":1050,"zOrder":1},
  {"index":1,"title":"Terminal","class":"org.gnome.Terminal","x":67,"y":32,"width":800,"height":600}
]}
```

每条记录都带有选择器可匹配的字段：`index`、`id`（X11 窗口 id / 后端 id）、
`title`、`class`、`instance`，以及 `x`/`y`/`width`/`height` 和可选的 `zOrder`。
当检测后端能提供进程归属时（X11 的 `_NET_WM_PID`、KDE/KWin、Hyprland、GNOME
扩展、niri），记录还会带 `pid`（进程号）与 `process`（进程名）。

#### 7.1.1 选择窗口（单选 / 任意多选）

`--window` 可重复传入，**一次调用截取任意数量的窗口**。每个选择器默认自动
解释（`--window-by auto`）：

| 选择器值               | 匹配规则                                             |
| :---                  | :---                                                |
| `0`、`1`、…           | 列表 `index`                                        |
| `0x3c00007`           | 窗口 `id`                                           |
| `12345`               | 进程 `pid`                                          |
| `VSCodium`            | `class` 或 `instance`，再按 `title`（精确 → 子串）     |
| `太殷龙摄 - VSCodium`| `title`                                             |

可用 `--window-by id|title|class|index|pid|process` 强制指定某一种匹配规则。
`pid` 按进程号精确匹配；`process` 按进程名匹配（支持大小写不敏感子串，取自
`/proc/<pid>/comm`）。注意 `pid`/`process` 依赖窗口声明的 `_NET_WM_PID`（或
合成器上报的进程号），属尽力而为的元数据——X11 上该属性由客户端自行声明，
自动化脚本请勿将其当作可信的进程归属证明。按 PID/进程名选窗时，即使目标
窗口被其他窗口完全遮挡、已最小化或不在当前工作区，X11 会话仍能从合成器保留
的窗口缓冲直接读取其真实内容（`windowCapture: true`），既不弹起窗口也不抢占
焦点；其他平台按区域抓屏并受合成器能力限制。一个选择器命中多个窗口时会
**全部**截取。

在窗口选择器后追加 `@x,y,width,height` 即可截取窗口内的组件子区域（偏移量
相对窗口左上角，自动裁剪到窗口范围内）：

```bash
# 窗口 0 的顶部 100px 条带
dracoPho --window "0@0,0,1680,100" --capture-destination file --capture-to /tmp/shots/
```

#### 7.1.2 选择图片去向

`--capture-destination` 决定输出方式，可搭配任意数量的 `--window` 选择器与
组件子区域：

| 去向 | 行为 |
| :--- | :--- |
| `inline`（默认） | 在 JSON 输出中直接内嵌 Base64 PNG。**不写任何文件，也绝不触碰剪贴板。** 只想拿到像素的智能体最安全的选择。 |
| `file` | 把 PNG 写入 `--capture-to <目录>`；需要提供该参数。 |
| `stage` | 把 PNG 写入临时暂存目录（`$TMPDIR/dracoPho-staging`），适合"先存着稍后取用"。 |
| `clipboard` | 图片进入系统剪贴板；多张图片时**最后一张生效**。内容在 CLI 退出后依然可用（会拉起持久的 `wl-copy` / `xclip` 所有者进程）。 |

示例：

```bash
# 多个窗口，保存到目录（每个窗口一张 PNG）
dracoPho --window VSCodium --window Terminal --capture-destination file --capture-to /tmp/shots/

# 一个窗口 + 另一个窗口的组件，暂存待用
dracoPho --window "VSCodium@0,0,400,300" --window 1 --capture-destination stage

# 多选，直接以 base64 返回，不写文件、不动剪贴板
dracoPho --window 0 --window "Terminal" --capture-destination inline

# 把窗口复制到剪贴板
dracoPho --window 0 --capture-destination clipboard
```

**剪贴板策略。** 交互式编辑器刻意把选区放入系统剪贴板（`Copy` 动作 /
`Ctrl+C`），因为这是截图工具最核心的使用方式。无头模式（CLI 与商业版 MCP
服务）遵循相反的规则：**除非显式选择 `clipboard` 作为目标、且已在
「设置 → 存储 → 无头模式」中开启剪贴板写权限，否则绝不改动剪贴板**——
`inline`（默认）与 `stage` 都不会触碰用户当前剪贴板内容，这样定时任务或
Agent 发起的截图不会覆盖用户正在别处使用的文本或图片。当 `clipboard`
请求因无头剪贴板写权限未开启而被拒绝时，捕获会降级为配置的无头默认去向，
在 JSON 输出的 `"warning"` 字段与 stderr 中同时给出提示，并以非零退出码
结束，便于自动化脚本检测。在设置中开启无头剪贴板写权限需要输入确认口令。

输出是一个 JSON 对象 `{"captures":[...]}`，每个被截窗口对应一条记录；每条
记录都会回显选择器、窗口身份与最终截取矩形，并带有 `path`（file/stage）或
`data`（inline）或两者皆无（clipboard）。只有当所有选择器都命中且全部截取
成功时退出码才为 `0`；某个选择器未命中或截取失败时退出码为 `1`，并在
`"error"` 字段给出原因，而不是静默返回成功。

#### 7.1.3 无窗口无弹窗保证

所有无头模式都保证全程无感、不干扰：

- **绝不创建任何窗口**——包括图片编辑窗口、截屏覆盖层与托盘；捕获复用无头
  捕获路径；
- **绝不弹出任何对话框**——包括错误对话框：错误一律走 stderr；即使是畸形
  命令（例如 `--window-by` 没有配 `--window`、未知的 `--capture-destination`、
  多余的图片文件位置参数）也会立即以非零退出码结束并在 stderr 给出原因，
  而不会弹出 `QMessageBox`，更不会落入交互式界面；
- 绝不弹出交互式 portal 授权框（`allowInteractivePortal` 已禁用）；
- 无人值守录制（`--record-*`）同样静默执行：**不弹桌面通知、不发起交互式
  portal 授权**，录制期间不抢焦点、不弹窗，结束状态通过 `--recording-status` /
  `--record-wait-json` 查询，而不是打扰用户；
- 写出输出后进程立即退出；
- 捕获前后窗口列表完全一致；
- 无头模式绝不触碰系统剪贴板，除非显式请求了 `clipboard` **且**已在
  「设置 → 存储 → 无头模式」中开启剪贴板写权限。

如果检测不到任何窗口（例如合成器辅助扩展未启用，或 X11 会话无法枚举窗口），
命令会在 stderr 打印明确错误并以退出码 `1` 结束，而不是静默截取到空结果。

同一捕获流水线也能程序化产出带标注的图片——见商业版的 MCP server 章节；也可
把保存下来的 PNG 交回交互编辑器继续加工。

---

## 8. 桌面快捷键与托盘

托盘模式（默认开启，见 § 1.3）默认注册 `Ctrl+Alt+S` 区域截图，并提供捕获 /
录制 / 设置 / 退出菜单。托盘图标平时不打扰，需要时随时可用。桌面快捷键配置：

- **GNOME**：设置 → 键盘 → 快捷键 → 自定义快捷键，绑定到 `dracoPho`。
- **KDE**：自定义快捷键绑定 `dracoPho`（精确 KDE 捕获还需 KWin ScreenShot2
  权限，见 README）。
- **Hyprland**：`bind = SUPER SHIFT, S, exec, dracoPho` 与
  `bind = , Print, exec, dracoPho`。
- **niri**：`binds { Mod+Shift+S { spawn "dracoPho"; } }`。
- **Sway / i3**：`bindsym Mod4+Shift+S exec dracoPho`。

---

## 9. 配置与后端

- 配置文件：`~/.config/dracoPho/config.json`（Linux），首次运行自动创建。
- 完整参考：[配置文档](configuration.zh-CN.md)。
- 后端：Wayland（PipeWire portal / grim / wlroots screencopy）、X11
  （`QScreen::grabWindow`）、Windows（原生 WGC）。录制优先使用 PipeWire
  portal，失败时自动回退。
- 设置窗口以确定性方式跟踪未保存的修改：每个控件（下拉框、开关、数值框、
  文本框、快捷键框、取色器）都会立即更新"未保存修改"标记，包括下拉框弹出层
  与模态取色对话框选择的取值；改回原值即清除标记，因此关闭窗口时只对真实
  存在的待保存修改进行确认。

可选辅助：

```bash
# OCR（RapidOCR / Tesseract）
python3 -m venv ~/.local/share/dracoPho/ocr-venv
~/.local/share/dracoPho/ocr-venv/bin/pip install -U pip rapidocr onnxruntime

# 二维码扫描（zxing-cpp）
python3 -m venv ~/.local/share/dracoPho/code-scan-venv
~/.local/share/dracoPho/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

---

## 10. 功能自测清单

按以下步骤端到端验证一个构建：

1. **启动** — `dracoPho` 按配置的启动行为启动（默认托盘 + 悬浮球），不会自动弹出截图覆盖层。
2. **启动行为** — 设置 → 通用 → 启动行为：勾选"直接截图"后重新启动：立即打开冻结覆盖层；取消勾选后重新启动：不再弹出覆盖层。
3. **悬浮球** — 单击弹出快捷菜单（截图 / 全屏截图 / 开始录制 / 设置 / 隐藏 / 退出）；双击直接截图；拖动移动位置且位置跨启动保留；截图进行中自动隐藏、结束后恢复显示。
4. **窗口悬停** — 鼠标移到窗口上：青色框跟随；单击选中窗口；重叠窗口选中
   最顶层。
5. **手动框选** — 拖动矩形；松开进入编辑器。
6. **标注** — 逐个试用工具（画笔、直线、矩形、椭圆、箭头、荧光笔、文字、序号、
   马赛克、放大镜、激光）；撤销 / 重做；选择工具移动 / 缩放 / 旋转 / 删除；
   双击文字编辑。
5. **复制 / 保存 / 钉住 / 上传** — `Ctrl+C`、`Ctrl+S`、`Ctrl+P`、`Ctrl+U`。
6. **启动工具** — `C` 取色器、`R` 标尺、`Q` 扫码、`D` 显示器快照。
7. **Headless** — `--capture-to`、`--region`、`--display`、`--list-displays`。
8. **无头窗口截图** — `--list-windows` 列出桌面窗口；重复 `--window` 一次截取
   多个窗口；逐一验证 `--capture-destination` 的四种模式（inline / file /
   stage / clipboard）；测试组件选择器（`--window "0@0,0,400,300"`）；确认
   捕获前后窗口列表不变（无干扰）。
9. **托盘与快捷键** — `dracoPho --tray`，按 `Ctrl+Alt+S`。
10. **便携版细节** — 包内自带 Qt 库 / 插件 / 脚本可被找到。

---

## 11. 反馈

使用内置的[问题提交指南](../.doc/submit-issue-via-gh.md)，通过 `gh issue
create` 提交问题，并附上 `dracoPho --debug --debug-log /tmp/dracoPho.log`
抓取的调试日志。
