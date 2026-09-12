# Windows 发布说明

## 录制依赖

Windows 录制使用 Windows Graphics Capture 采集画面，使用 FFmpeg/libav 写入 MP4、GIF 或动画 WebP。MSYS2/UCRT64 构建必须安装以下包：

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-ffmpeg \
  mingw-w64-ucrt-x86_64-qt6-base \
  mingw-w64-ucrt-x86_64-qt6-tools
```

发布构建使用 `-DMARK_SHOT_REQUIRE_FFMPEG=ON`。如果缺少 FFmpeg 头文件或导入库，CMake 会在配置阶段失败，避免生成无法录制的 Windows 包。

Windows 视频录制音频使用 WASAPI loopback 采集默认播放设备输出，不依赖 PulseAudio。当前实现录制系统声音；麦克风混音和单独音轨属于后续增强范围。

## 运行时部署

Windows 包通过 `scripts/windows-deploy-runtime.sh` 运行 `windeployqt`，并用 `objdump` 递归复制 `dracoPho.exe` 和 DLL 的依赖项。FFmpeg 的 `avcodec`、`avformat`、`avutil`、`swresample`、`swscale` 相关 DLL 会随依赖闭包进入 `app/bin`。

发布前需要确认 Windows CI 的 `Build`、`Test`、`Deploy runtime libraries`、`Sign Windows binaries`、`Package artifact` 和 `Upload artifact` 步骤全部通过。

## 安装器与静态单文件 exe

发布流水线（`release-binaries.yml`）在普通 zip 包之外，还产出两种 Windows 产物：Inno Setup 安装器与静态单文件 exe。

### Inno Setup 安装器

- 使用 runner 预装的 Inno Setup（`C:\Program Files (x86)\Inno Setup*\ISCC.exe`），无需 choco 安装；PowerShell 调用时用 `$iscc.FullName`。
- 脚本模板为 `packaging/windows/dracoPho-setup.iss`，通过 `/D` 定义参数传入版本号、tag、exe 名、产品名、源目录与图标。注意 **`/D` 参数必须放在 `.iss` 路径之前**（Inno Setup 语法），参数值用单引号包裹避免 PowerShell 解析反斜杠。
- ISCC 的 `OutputDir=.` 相对于 `.iss` 文件所在目录，产物先落在 `packaging/windows/`，再移动到工作区根供上传步骤统一引用。
- 历史 tag 回填时，辅助文件（`.iss` 模板、静态 FFmpeg 脚本）从默认分支拉取；该步骤在 MSYS2 安装前执行，必须显式指定 `shell: bash` 而非 msys2 shell。
- 安装器不内置 Inno Setup 中文语言文件（runner 预装版不含 `ChineseSimplified.isl`），界面语言以脚本内置语言为准。

### 静态单文件 exe

- MSYS2/UCRT64 环境，静态 Qt6 从清华镜像手动安装（主镜像未同步 `mingw-w64-ucrt-x86_64-qt6-static`）；pacman 是 MSYS2 命令，安装步骤需在 msys2 shell 中执行。
- 静态 FFmpeg 由 `scripts/build-static-ffmpeg.sh` 构建（LGPL），构建环境需额外安装 `diffutils`（cmp）与 `make`。
- 链接时静态 FFmpeg / Qt 依赖系统库，用 `-Wl,--start-group … --end-group` 包裹系统库，解决库间循环依赖。
- 产物用 `objdump` 校验仅剩系统 DLL 依赖，并附 SHA256 校验文件。

## 代码签名

发布工作流支持 Authenticode 签名。需要在 GitHub 配置：

- `WINDOWS_CODESIGN_CERTIFICATE_BASE64`：PFX 证书的 base64 内容，放在 Secrets。
- `WINDOWS_CODESIGN_CERTIFICATE_PASSWORD`：PFX 密码，放在 Secrets。
- `WINDOWS_CODESIGN_TIMESTAMP_URL`：时间戳服务器地址，放在 Variables；未配置时使用 `http://timestamp.digicert.com`。

本地生成 base64 内容可使用：

```powershell
[Convert]::ToBase64String([IO.File]::ReadAllBytes("codesign.pfx")) | Set-Content codesign.pfx.base64
```

脚本 `scripts/windows-sign-artifact.ps1` 会在压缩发布包前签名 `app` 目录下的 `.exe` 和 `.dll`。未配置证书时脚本会跳过签名，不影响普通 CI。

代码签名可以显著降低 Windows 和杀毒软件误报概率，但误报还与证书信誉、下载量、文件行为和分发域名有关。新证书发布初期仍可能触发 SmartScreen 提示，需要持续使用同一证书发布版本来积累信誉。
