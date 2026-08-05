<div align="center">
  <img src="data/icons/DracoPho-logo.png" alt="DracoPho Logo" width="128" />
  <h1>DracoPho</h1>
  <p>
    <a href="https://github.com/tystudio-26020701/mark-shot-community/releases">
      <img src="https://img.shields.io/github/v/release/tystudio-26020701/mark-shot-community?color=6da0f2&labelColor=4a5054&label=release&style=flat-square&logo=github" alt="Release" />
    </a>
    <a href="https://gitter.im/mark-shot/community">
      <img src="https://img.shields.io/badge/gitter-join%20chat-46bc99?labelColor=4a5054&style=flat-square&logo=gitter" alt="Gitter" />
    </a>
    <img src="https://img.shields.io/badge/language-C%2B%2B-dfb56c?labelColor=4a5054&style=flat-square&logo=c%2B%2B" alt="Language C++" />
    <img src="https://img.shields.io/badge/framework-Qt%206-92d076?labelColor=4a5054&style=flat-square&logo=qt" alt="Framework Qt 6" />
    <img src="https://img.shields.io/badge/platform-Linux%20%7C%20Windows-28c0e7?labelColor=4a5054&style=flat-square" alt="Platform Linux | Windows" />
    <img src="https://img.shields.io/badge/display-Wayland%20%7C%20X11-9979d9?labelColor=4a5054&style=flat-square" alt="Display Wayland | X11" />
    <img src="https://img.shields.io/badge/features-Screenshot%20%7C%20OCR%20%7C%20Pin%20%7C%20Scroll-ff8f59?labelColor=4a5054&style=flat-square" alt="Features Screenshot | OCR | Pin | Scroll" />
  </p>
</div>

[中文说明](README.zh-CN.md)

Read this README in other languages:
[简体中文](README.zh-CN.md) · [繁體中文](READMEs/README.zh-TW.md) ·
[日本語](READMEs/README.ja.md) · [한국어](READMEs/README.ko.md) ·
[Русский](READMEs/README.ru.md) · [Italiano](READMEs/README.it.md) ·
[العربية](READMEs/README.ar.md) · [Français](READMEs/README.fr.md) ·
[Deutsch](READMEs/README.de.md) · [Español](READMEs/README.es.md) ·
[Português](READMEs/README.pt.md)

**Tags**: `C++` / `Qt 6` / `Screenshot` / `Annotation` / `Pin Sticker` / `OCR` / `Scroll Screenshot` / `Wayland` / `Windows`


<details>
<summary>Video Demo</summary>
<p align="center">
  <video src="https://github.com/user-attachments/assets/4f86fcee-fef9-409e-98ba-1491ecee06c7" width="100%" controls></video>
</p>
</details>

`mark-shot` is a high-performance screenshot and annotation tool built with Qt 6. Originally designed for Wayland compositors such as `niri`, it now supports standard screenshot and annotation workflows on Linux (X11, GNOME, and wlroots/Wayland desktops) as well as Windows environments.

It captures screen frames instantly and opens an interactive fullscreen overlay, providing region cropping, rich annotation, clipboard copying, saving, and desktop pinning features.

---

## Features

### Advanced Annotation Toolset
- **Pen & Highlighter**: Smooth freehand drawing and semi-transparent overlay highlighting.
- **Geometric Shapes**: High-precision Line, Rectangle, and Ellipse paths. Rectangle additionally supports a style selector with three modes:
  - `Stroke`: outlined or filled rectangle with optional rounded corners.
  - `Highlight`: marker-pen overlay rendered with `CompositionMode_Multiply` and a semi-transparent fill.
  - `Invert`: inverts the RGB pixels covered by the rectangle while keeping the outline as a visual cue.
- **Refined Arrow**: Sharp 6-vertex acute arrow path rendering with anti-aliasing.
- **Dual-Gesture Text**:
  - Supports dynamic, ultra-large font sizing with fluid adjustment via scroll wheel or property sliders.
  - Implements a physical width buffer to prevent unexpected wrapping across extreme scales.
  - **Diagonal handles** scale font size and boundary box proportionally; **side borders** only adjust wrap width.
  - **Precise font control**: the text font panel provides an exact point-size input (8-300 pt), a font family list, and Bold / Italic toggles. All of them apply to new text, the inline editor, and existing annotations, and persist across sessions.
- **Laser Pointer**: Dedicated presentation tool with pen traces that dissolve smoothly over time.
- **Auto-Increment Marker**: Click to stamp sequential numbered markers.
- **Mosaic**: Applies high-fidelity acrylic frost blur to obscure sensitive information.
- **Magnifier with Independent Frames**: The magnifier loupe exposes resize handles on both the inner source viewfinder and the outer lens. Rectangle lenses get 8 corner/edge handles per frame, circular lenses get 4. Resizing either frame keeps the magnification ratio constant by scaling the other frame proportionally; translating one frame leaves the other untouched.
- **Startup Code Scan**: Press `Q` before selecting a region, drag around a QR code or barcode, and open the decoded result in a copyable window.
- **Quick Display Capture**: Press `D` before selecting a region to instantly capture all outputs, crop them by display, and hover a thumbnail to copy, edit, or save that display image.
- **GIF, MP4 and Animated WebP Recording**: Press the configured startup recording shortcuts or use the tray menu to record a selected display or a custom region as GIF, MP4 or animated WebP. Recording is crash-safe: MP4 is written to a temporary MKV and remuxed on finish, so an interrupted recording stays recoverable. Active recordings show tray and frozen-frame status, can be stopped with `S`, the overlay button, the tray menu, or `--stop-recording`, and send desktop notifications when recording starts or saves. On Wayland, recording prefers the PipeWire portal backend and can fall back to wlroots screencopy or polling capture when portal capture is unavailable.
- **Image Host Upload**: Press `Ctrl+U` or click the toolbar upload button after selecting a region to upload the screenshot to a custom image host (ImgURL, sm.ms, imgbb, litterbox, etc.). The returned URL is automatically copied to the clipboard. Configure the host via `upload.env`, or plug in any custom uploader via `upload.command`.
- **Mac-style Export Frame**: Adds transparent padding, rounded corners, and a soft shadow to saved, copied, uploaded, Open With, and extension-command images.

### Pinned Window Stickers
- Pins any cropped region or annotated screenshot as an independent, frameless, and top-level floating window.
- Supports direct selection of OCR-recognized text in the pinned window, with `Ctrl + C` and context-menu copying.
- Supports OpenAI-compatible LLM translation for OCR text, rendering translated text back onto the image at the original layout positions.
- **Interactive Gestures**:
  - Drag with left click to reposition.
  - Scroll mouse wheel to scale.
  - Double left-click or press `Esc` to close.
  - Right-click to open a context menu with options to rotate, copy image text, translate, save, copy, or close.

### Scrolling Screenshot Capture
- Captures a long scrolling region by combining PipeWire screencast frames with an interactive scrolling overlay and stitcher.
- Designed primarily for `niri` and similar Wayland environments where output geometry and capture timing can be controlled predictably.
- **Floating Drag Handle for Large Regions**: When the selected capture region is too large to fit the preview panel on the screen, the preview panel is hidden, and a **floating drag handle** (a small floating button with direction arrows) is shown near the selection edge instead.
  - **Drag to reposition selection**: Press and drag the floating handle to slide the capture region along the active scroll axis. This allows adjusting the target area and reaching off-screen content.
  - **Click to toggle axis**: Click the handle directly before capture starts to switch between vertical and horizontal scroll directions.
- **GNOME Wayland**: scrolling capture requires the bundled `mark-shot-scroll-helper@snemc.org` GNOME Shell extension. GNOME does not expose the capture and preview hooks DracoPho needs to normal desktop applications, so the extension provides a private D-Bus helper for area screenshots and the scroll preview panel.
- **Compatibility notice**: scrolling capture on KDE, X11, and other non-`niri` environments is a test feature and is not complete yet. Portal backends, shell policies, window geometry behavior, frame timing, and scroll event handling differ substantially across these desktop stacks.
- If scrolling capture fails, use normal screenshots or configure an external long-screenshot command through DracoPho extension commands.
- To report a scrolling capture issue, run `mark-shot --debug --debug-log /path/to/mark-shot.log`, reproduce the failure, then attach the log to a GitHub issue. The same logging can be enabled through `debug.enabled` and `debug.logPath` in `config.json`; `DEBUG=1` and `MARK_SHOT_DEBUG_LOG=/path/to/log` remain supported.

### Cross-Platform Display Server Support
- **Wayland**: Uses PipeWire portal screencast for recording and experimental scrolling capture, including shared-memory and DMA-BUF frame paths, `grim` for wlroots screenshot capture, `layer-shell-qt` for native overlay, and `wl-copy` for clipboard persistence.
- **GNOME Wayland**: Uses the DracoPho Scroll Helper GNOME Shell extension for scrolling capture. Without the extension, DracoPho disables the scrolling capture action on GNOME Wayland.
- **X11**: Uses `QScreen::grabWindow` for screen capture, fullscreen top-level window for overlay, and `xclip` for clipboard persistence.
- **Windows**: Uses Qt's native screen capture and clipboard APIs for the core screenshot, annotation, copy, save, and pin workflows. Linux-specific backends such as PipeWire, xdg-desktop-portal, `grim`, XCB window detection, LayerShellQt, and GNOME Shell helpers are disabled at build time.
- Linux display server backends are detected at runtime via `$XDG_SESSION_TYPE`; Windows uses Qt's native platform backend.
- **Multi-monitor freeze scope**: by default, region selection freezes every connected display (single virtual-desktop window when DPRs match on X11/Windows), and after committing a selection on one monitor the other displays stay frozen and non-interactive until the session ends. The **Cursor Screen** scope freezes only the monitor under the cursor.

### Desktop Integration
- **Configurable startup behavior**: launching DracoPho no longer opens the
  capture overlay by default. In Settings → General → **Startup Behavior** you
  can combine **Tray Icon**, **Floating Ball**, **Settings Window** and
  **Direct Capture** modes. Fresh installs default to Tray Icon + Floating
  Ball; direct capture is opt-in.
- **Floating ball**: a small draggable always-on-top ball (bottom-right by
  default) offers quick access to capture, fullscreen capture, recording and
  settings. Single click opens the menu, double click captures, drag to move
  (position is remembered), and it hides automatically during a capture.
  Dropping it near a screen edge snaps and docks it with **half of the ball
  sliding off-screen** (hover to reveal, move away to re-hide) on
  X11/Windows/macOS; on native Wayland the compositor controls window
  positions so the ball stays free-floating (protocol limitation). It fades
  to semi-transparent after a few idle seconds, returning to full opacity on
  hover.
- **Desktop Entries**:
  - `mark-shot.desktop`: Configures the utility system-wide, triggerable by custom shortcuts.
  - `mark-shot-edit.desktop`: Registers as an image editor, enabling users to right-click local files in file managers (Dolphin, Nautilus, etc.) and open them directly in annotation mode.
- Ships with scalable vector icons (`dracoPho.svg`, plus the upstream `mark-shot.svg` and `mark-shot-edit.svg`).

### KDE KWin ScreenShot2 Authorization

On KDE Wayland, DracoPho can use KWin's `org.kde.KWin.ScreenShot2` interface for exact area capture. KWin treats this as a restricted D-Bus interface, so the application must have a desktop entry that declares the permission.

<details>
<summary>KDE KWin ScreenShot2 Authorization Details (Click to expand)</summary>

Desktop entry permission:
```ini
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

Distribution packages and `cmake --install` install the required desktop entries automatically. If you run a locally built binary without installing the project, create or update `~/.local/share/applications/mark-shot.desktop`:

```ini
[Desktop Entry]
Type=Application
Name=DracoPho
Comment=Wayland screenshot selection and annotation tool
Exec=/absolute/path/to/mark-shot
Icon=dracoPho
Terminal=false
Categories=Graphics;Utility;
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

If you bind DracoPho through KDE's command shortcut service, also create `~/.local/share/applications/net.local.mark-shot.desktop`:

```ini
[Desktop Entry]
Type=Application
Name=DracoPho Shortcut Service
Exec=/absolute/path/to/mark-shot
Icon=dracoPho
Terminal=false
NoDisplay=true
StartupNotify=false
Categories=Utility;
X-KDE-GlobalAccel-CommandShortcut=true
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

After changing desktop entries, refresh KDE's desktop file cache by logging out and back in. If the current KDE session still returns `NoAuthorized`, restart KWin or reboot once.
</details>

---

## Product Comparison

DracoPho Community Edition is an **open source (MIT), cross-platform (native Linux X11/Wayland + Windows), fully offline** all-in-one screenshot, annotation, pin, OCR, translation and recording tool. The tables below are compiled from each product's official documentation and website (as of **August 2026**) and cover the most popular screenshot tools — open source and commercial — across every major desktop platform. Capabilities are honestly marked: **✅ built-in**; **⭕ partial (limited by paid tier, platform or external tool/service)**; **❌ not available**. They are counted on an "out of the box" basis — not tied to paid tiers, cloud services or experimental branches; the exact ⭕ limitations are explained in the detailed notes below.

### Core capability matrix

#### I. Capture (can you get what you need)

| Capability | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Region capture | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Fullscreen / multi-monitor capture | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Window capture | ⭕ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Occluded / minimized window content | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Scrolling / long screenshot | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ⭕ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Timed / delayed capture | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⭕ | ❌ | ✅ |

#### II. Annotation and intelligence (can you work efficiently after capture)

| Capability | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Annotation toolset | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Pin to screen | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ |
| OCR text recognition | ✅ | ✅ | ✅ | ⭕ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Translation | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| QR / barcode recognition | ✅ | ✅ | ✅ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ⭕ |
| Color picker | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ✅ | ✅ | ✅ | ✅ |

#### III. Output and automation (can you share / archive the result)

| Capability | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Screen recording | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ |
| Animated GIF / WebP output | ✅ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ |
| Image upload / cloud | ✅ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ |
| Headless CLI / scripting | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| Floating ball launcher | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Capture history | ❌ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ | ⭕ | ❌ | ✅ | ✅ | ⭕ | ❌ | ⭕ |

#### IV. Platform and ecosystem

| Capability | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Native Linux Wayland | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Cross-platform (≥2 desktop OS) | ⭕ | ❌ | ⭕ | ✅ | ✅ | ✅ | ❌ | ⭕ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ |
| Plugin / extension mechanism | ✅ | ⭕ | ❌ | ❌ | ⭕ | ✅ | ⭕ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| Open source / free | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ✅ | ⭕ | ✅ | ❌ | ❌ | ✅ | ⭕ | ⭕ |

### Detailed feature notes

**I. Capture** (can you get what you need)

- **Region / fullscreen / multi-monitor capture** — built into all 15 tools listed; DracoPho additionally handles negative-coordinate displays and per-display HiDPI scaling correctly, with no ghosting when stitching multi-monitor captures.
- **Window capture** — DracoPho's interactive mode uses a fullscreen selection overlay (no dedicated "active window" one-click mode, hence ⭕), while its headless mode (`--window`) selects windows precisely by id, title, class, **PID or process name**; Flameshot has no window mode at all, every other tool ships one.
- **Occluded / minimized window content** — on X11, DracoPho reads the window's own composited buffer via XComposite, capturing real content even when the window is fully occluded or minimized (without raising it or stealing focus); among similar tools only scrot's `--stack` can do this. Wayland's protocol prevents every tool from reading minimized-window content.
- **Scrolling / long screenshot** — DracoPho works natively on niri and GNOME Wayland (official extension), with KDE/X11 as experimental. ShareX, PixPin, PicPick, Snagit, CleanShot X, Shottr, Xnip and iShot ship it built-in; Greenshot only for legacy IE scenarios; Snipaste, Flameshot, ksnip, Spectacle and Windows Snipping Tool do not.
- **Timed / delayed capture** — DracoPho waits a configurable number of seconds with a fullscreen countdown overlay (Esc to cancel) before entering capture, via `--delay`, the tray/floating-ball "Delayed Capture" submenu, or the Capture settings page; Flameshot, ksnip, Spectacle and most others support a plain delayed start, ShareX a timed capture, and PixPin, Snipaste and Xnip none.

**II. Annotation and intelligence** (can you work efficiently after capture)

- **Annotation toolset** — DracoPho ships 12+ tools: pen, highlighter, line, rectangle, ellipse, arrow, text, step numbers, mosaic, dual-frame magnifier, laser pointer, color picker and ruler; all 15 tools have built-in annotation with varying depth.
- **Pin to screen** — DracoPho offers frameless always-on-top stickers with scale, rotate, OCR word picking and on-sticker LLM translation; Snipaste, PixPin, ShareX, ksnip, CleanShot X, Shottr, Xnip and iShot include it too; Flameshot, Spectacle, Greenshot, PicPick, Snipping Tool and Snagit do not.
- **OCR text recognition** — DracoPho bundles RapidOCR (offline PP-OCR models) with a Tesseract fallback, working out of the box; Windows Snipping Tool (local Text Actions), ShareX, Snagit, CleanShot X, Shottr, Xnip and iShot ship it; Snipaste gates it behind a paid tier, ksnip via a plugin and Spectacle via an external Tesseract install; Flameshot, Greenshot and PicPick have none.
- **Translation** — DracoPho bundles OpenAI-compatible LLM screenshot translation (offline / self-hosted supported). Only iShot ships screenshot translation, PixPin gates it behind membership and no other tool offers it. This remains one of the strongest differentiators.
- **QR / barcode recognition** — DracoPho scans QR, 1D codes and PDF417 with `Q` during capture; ShareX and PixPin support it; Snipaste (paid) plus CleanShot X / Shottr / iShot read QR only, via their OCR engines.
- **Color picker** — DracoPho has a built-in screen color picker with a history palette; most tools ship one except Spectacle and Snagit; Windows Snipping Tool only on Copilot+ AI PCs.
- **One-click AI redaction** — WeChat's 2026 auto-redaction is not built in yet (❌); DracoPho's mosaic / blur is manual. Snagit (AI Smart Redact) and Snipping Tool's text redaction are comparable implementations; automated redaction is a possible follow-up.

**III. Output and automation** (can you share / archive the result)

- **Screen recording** — DracoPho records MP4 / GIF / animated WebP with silent unattended region/display recording (no windows, no dialogs, no focus stealing); ShareX, PixPin, Spectacle (Plasma 6), PicPick, Snipping Tool, Snagit, CleanShot X and iShot ship it; Snipaste, Flameshot, ksnip, Greenshot, Shottr and Xnip do not.
- **Animated GIF / WebP output** — DracoPho natively records **animated WebP** (smaller files, alpha support); no competitor produces animated WebP natively. PixPin and iShot record GIF, ShareX records GIF (WebP static-save only), Snagit and CleanShot X export GIF; the rest have no animated output.
- **Image upload / cloud** — DracoPho ships ImgURL / sm.ms / imgbb / litterbox / custom-command upload (`Ctrl+U`); ShareX has the most targets; Greenshot, PicPick, Snagit and CleanShot X have built-in cloud upload; Flameshot only Imgur, ksnip Imgur/FTP/scripts, Shottr S3 after activation; Snipaste, PixPin, Snipping Tool, Xnip and iShot have none.
- **Headless CLI / scripting** — DracoPho provides a complete headless pipeline: `--capture-to` region/display screenshots, `--list-windows` + `--window` window/component captures, `--record-region` / `--record-display` unattended recording and `--record-wait-json` structured status output — all without windows, dialogs or focus stealing, with **PID/process-name targeting and occluded/minimized window capture**. The enterprise MCP server reuses the same pipeline. ShareX (Windows-only), Flameshot, ksnip and Spectacle have solid CLIs; Snipaste's CLI is a paid-tier feature; PixPin only offers a JS action engine.
- **Floating ball launcher** — DracoPho has a draggable ball that snaps to screen edges (X11/Windows/macOS; Wayland keeps it free-floating due to protocol limits) and fades out when idle; only PixPin offers something similar.
- **Capture history** — DracoPho has no dedicated history panel yet; ShareX, Snipaste, Flameshot, Snagit and CleanShot X do, ksnip / PicPick / Shottr / iShot have lightweight alternatives (tabs / gallery / pinned stash), the rest none.

**IV. Platform and ecosystem**

- **Native Linux Wayland** — DracoPho natively supports PipeWire portal, grim, layer-shell, KDE KWin ScreenShot2 and GNOME extensions; among open source tools only Spectacle (KDE) matches that depth, while Flameshot / ksnip are experimental or portal-dependent.
- **Cross-platform** — Snipaste, Flameshot, ksnip and Snagit cover all three major desktop OSes (Windows + macOS + Linux); DracoPho covers Linux + Windows (macOS planned); ShareX, PicPick, Snipping Tool, Spectacle, CleanShot X, Shottr, Xnip and iShot are single-platform.
- **Plugin / extension mechanism** — DracoPho provides a Qt plugin system with a GitHub plugin marketplace (extensible OCR / translation / code-scan providers); ksnip and Greenshot have plugin APIs; ShareX, Spectacle, Snagit, CleanShot X and Shottr substitute custom actions / integrations.
- **Open source / free** — DracoPho CE is MIT licensed, fully free, ad-free, account-free and network-optional; ShareX, Flameshot, ksnip, Spectacle and Greenshot (Windows) are open-source free too; Shottr and Snipping Tool are free; PixPin / Snipaste / PicPick / Xnip / iShot are closed-source freemium; Snagit / CleanShot X are paid commercial software.

> Note: compiled from each product's official website and documentation (2026-08); capability changes with each release, so refer to the latest docs. FastStone Capture, Nimbus, Lightshot, Sogou Capture and WeChat / QQ screenshot are not in the matrix above (WeChat's 2026 one-click AI redaction leads the segment).

---

## Usage

### Command Line Interface (CLI)

```bash
# Capture screen with interactive region selection
mark-shot

# Capture all outputs on a multi-monitor setup
mark-shot --all-outputs

# Annotate full captured screen directly (skipping selection)
mark-shot --fullscreen

# Start with Move after region selection, Laser in fullscreen, and a red default color
mark-shot --default-tool move --fullscreen-default-tool laser --default-color '#FF4D4D'

# Open and annotate an existing local image file
mark-shot path/to/image.png

# Open an existing image directly as a pinned sticker window
mark-shot --pin-image path/to/image.png

# Force standard XDG window instead of Wayland layer-shell
mark-shot --xdg-window
```

#### Headless (non-interactive) capture

Scripts, CI jobs, and other programs can capture the screen without opening
the annotation UI. The captured frame is written to a PNG and a compact JSON
summary is printed to stdout:

```bash
# Capture the primary screen to a PNG
mark-shot --capture-to /tmp/shot.png

# Capture into a directory (a timestamped file name is generated)
mark-shot --capture-to /tmp/shots/

# Capture a logical screen region (x,y,width,height)
mark-shot --capture-to /tmp/region.png --region 0,0,1280,720

# Capture a specific monitor by name, with the mouse cursor included
mark-shot --capture-to /tmp/window.png --display DP-1 --include-cursor

# Capture several monitors at once (repeat --display; one PNG per monitor)
mark-shot --capture-to /tmp/shots/ --display DP-1 --display DP-2

# Print the available outputs as JSON and exit
mark-shot --list-displays
```

The JSON output of a single-display `--capture-to` looks like:

```json
{"path":"/tmp/shot.png","width":2560,"height":1440,"output":"DP-1","error":null}
```

When more than one `--display` is requested the output becomes an array of
captures, one per monitor:

```json
{"captures":[{"path":"/tmp/shots/mark-shot-DP-1-20260801-000000.png","width":2560,"height":1440,"output":"DP-1","error":null},
             {"path":"/tmp/shots/mark-shot-DP-2-20260801-000000.png","width":1920,"height":1080,"output":"DP-2","error":null}]}
```

Each selected monitor is captured with its own source geometry, so portal-based
backends return exactly that display instead of the whole virtual desktop.

Headless capture reuses the same capture backends as the interactive UI
(QScreen, xdg-desktop-portal, PipeWire, grim, KWin/GNOME helpers, and Windows
Graphics Capture), so image quality and region handling are identical. All
headless options are mutually exclusive with the positional image-file
argument.

Headless mode can also capture **specific windows or components** (no windows,
no dialogs, no focus stealing):
```bash
# list windows (including pid/process for process-based targeting)
mark-shot --list-windows

# capture by window title / process name / process id
mark-shot --window "VSCodium" --capture-destination file --capture-to /tmp/shots/
mark-shot --window-by process --window vscode --capture-destination inline

# capture by PID even when occluded or minimized (X11 reads the window's own
# composited buffer without raising it)
mark-shot --window-by pid --window 12345 --capture-destination file --capture-to /tmp/shots/

# capture a 100px component strip at the top of window 0
mark-shot --window "0@0,0,1680,100" --capture-destination stage
```

Unattended recording (`--record-*`) is equally silent: no desktop notification,
no interactive portal prompt, no focus stealing; the outcome is queried via
`--recording-status` / `--record-wait-json`.

### CLI Arguments

| Option | Description |
| :--- | :--- |
| `[file]` | **Positional**: Opens an existing local image in annotation mode instead of capturing the screen. |
| `-h`, `--help` | Displays help information and exits. |
| `-v`, `--version` | Displays version information and exits. |
| `--all-outputs` | Captures all screens on the virtual display environment instead of only the active one. |
| `--xdg-window` | Forces the use of a standard XDG fullscreen window (xdg-shell) instead of layer-shell. |
| `--fullscreen` | Skips region selection and opens annotation mode on the full screen frame directly. |
| `--tray` | Keeps DracoPho running in the system tray and registers global capture hotkeys when supported. |
| `--capture` | Forces one-shot capture when tray autostart is enabled in the config. |
| `--delay <seconds>` | Waits the given number of seconds with a fullscreen countdown overlay (Esc to cancel) before entering capture. |
| `--pin-image <path>` | Opens an existing local image directly as a pinned sticker window, skipping capture and region selection. |
| `--recording-status` | Prints the current recording status as JSON through the running instance. |
| `--stop-recording` | Requests the running instance to stop the active recording. |
| `--record-region <x,y,w,h>` | Records a screen region through the running instance; geometry as x,y,width,height. |
| `--record-display <id>` | Records a display by id (see `--list-displays` output: a raw screen name such as `DP-1`, the `output:DP-1` key, or `all`). |
| `--record-output <path>` | Output file path for the recording (required with the record flags). |
| `--record-duration <seconds>` | Recording duration in seconds; 0 records until `--stop-recording`. |
| `--record-fps <n>` | Frame rate for the recording (default 15). |
| `--record-format <mp4\|gif\|webp>` | Recording format: mp4 (default), gif or webp. |
| `--record-audio` | Include system audio in the recording. |
| `--record-wait-json` | Wait for the recording to finish, then print the final status as JSON. |
| `--default-tool <tool>` | Sets the annotation tool selected after region selection. Also seeds fullscreen mode unless `--fullscreen-default-tool` is set. |
| `--fullscreen-default-tool <tool>` | Sets the annotation tool selected in fullscreen annotation mode. |
| `--default-color <color>` | Sets the default annotation color. Supports `#RRGGBB` and `#RRGGBBAA`. |
| `--debug` | Enables debug logging for this run. |
| `--no-debug` | Disables debug logging for this run, overriding config and environment variables. |
| `--debug-log <path>` | Writes debug logs to the specified path and enables debug logging unless `--no-debug` is also set. |
| `--capture-to <path>` | Headless capture: writes a PNG to the given file or directory without opening the UI. Prints a JSON summary to stdout. |
| `--region <x,y,w,h>` | With `--capture-to`: capture only the logical screen region. |
| `--display <name>` | With `--capture-to`: capture a specific output by monitor name. May be repeated to capture several monitors at once (one PNG each). |
| `--include-cursor` | With `--capture-to`: draw the mouse cursor into the captured frame. |
| `--output-name <name>` | With `--capture-to`: base file name (without extension) used when the capture path is a directory. |
| `--list-displays` | Prints the available outputs as JSON and exits. |
| `--list-windows` | Lists the visible windows (id, title, class, pid, geometry) as JSON and exits. |
| `--window <selector>` | Captures the window(s) matching the selector. May be repeated; append `@x,y,w,h` to capture a component sub-region. |
| `--window-by <mode>` | How `--window` selectors are interpreted: `auto`, `id`, `title`, `class`, `index`, `pid` or `process`. |
| `--capture-destination <mode>` | Where captured window images go: `inline` (base64), `file`, `stage` or `clipboard`. |

### Compositor / Desktop Hotkey Integration

To bind `mark-shot` to a system screenshot shortcut, configure your compositor or desktop environment.

**Tray mode**:
```powershell
mark-shot --tray
```

Tray mode registers these global hotkeys by default:
- `Ctrl+Alt+S`: start region capture.

The tray menu also provides Capture, Fullscreen Capture, Start Recording, live recording status, Stop Recording, Settings, and Quit actions.

**niri** (`~/.config/niri/config.kdl`):
```kdl
binds {
    Mod+Shift+S { spawn "mark-shot"; }
}
```

**Hyprland** (`~/.config/hypr/hyprland.conf`):
```ini
# Bind Super+Shift+S to start mark-shot selection
bind = SUPER SHIFT, S, exec, mark-shot
# Bind Print key to start mark-shot selection
bind = , Print, exec, mark-shot
```

**Sway / i3** (`~/.config/sway/config` or `~/.config/i3/config`):
```ini
# Bind Super+Shift+S to start mark-shot selection
bindsym Mod4+Shift+S exec mark-shot
# Bind Print key to start mark-shot selection
bindsym Print exec mark-shot
```

**GNOME** (via custom keyboard shortcut in Settings → Keyboard → Keyboard Shortcuts → Custom Shortcuts).

### Extension Commands

The right-side action toolbar includes an **Extensions** button. It reads user-defined commands from `~/.config/mark-shot/extensions.json`. The file can be either a JSON array or an object with a `commands` array.

```json
{
  "commands": [
    {
      "name": "Long screenshot",
      "command": "./target/release/wayscrollshot {slurp}",
      "workingDirectory": "~/Desktop/projects/wayscrollshot",
      "closeOnStart": true
    },
    {
      "name": "OCR selection",
      "command": "ocr-tool {image}",
      "saveImage": true
    }
  ]
}
```

`command` is executed through `$SHELL -c` on Unix-like systems and `%COMSPEC% /C` on Windows, so shell features work. Use `{slurp}` to pass the current selection as `x,y widthxheight` geometry. Use `{image}` or `{imagePath}` to pass the current rendered selection as a temporary PNG path, or `{imageUrl}` for a `file://` URL. These placeholders are shell-quoted automatically. Set `saveImage` or `needsImage` to `true` to append the temporary PNG path when no image placeholder is present. `workingDirectory` and `cwd` are aliases. `closeOnStart` defaults to `true`, hiding and closing DracoPho before the command starts.

### Application Configuration

See [Configuration Reference](docs/configuration.md).

### User Guide

For everyday operation — the window hover-selection feature, annotation
tools, startup tools, pinned windows, scrolling capture, headless CLI, and a
feature testing checklist — see the
[User Guide](docs/user-guide.md) ([中文](docs/user-guide.zh-CN.md)).

Available in other languages:
[简体中文](docs/user-guide.zh-CN.md) · [繁體中文](docs/user-guide.zh-TW.md) ·
[日本語](docs/user-guide.ja.md) · [한국어](docs/user-guide.ko.md) ·
[Русский](docs/user-guide.ru.md) · [Italiano](docs/user-guide.it.md) ·
[العربية](docs/user-guide.ar.md) · [Français](docs/user-guide.fr.md) ·
[Deutsch](docs/user-guide.de.md) · [Español](docs/user-guide.es.md) ·
[Português](docs/user-guide.pt.md)

## Compilation & Installation

### Installation Guide

##### Arch Linux (AUR)
Arch Linux users can install directly from the AUR using helpers like `paru` or `yay`:
```bash
# Build from source
paru -S mark-shot
# or
yay -S mark-shot

# Install the prebuilt binary package instead
paru -S mark-shot-bin
# or
yay -S mark-shot-bin
```

`mark-shot` compiles from source; `mark-shot-bin` downloads the prebuilt pacman package from GitHub Releases.

##### NixOS
NixOS users can install mark-shot by adding it as a Flake input:
```nix
# flake.nix
mark-shot = {
  url = "github:tystudio-26020701/mark-shot-community";
  inputs.nixpkgs.follows = "nixpkgs";
};

# home-manager
home.packages = with pkgs; [
  # other user packages
  inputs.mark-shot.packages.${pkgs.stdenv.hostPlatform.system}.default
]
```

##### Other Distributions (Pre-built Packages)
For other distributions (such as Debian, Ubuntu, or Fedora), download the compiled package from the Releases page and install it via:
- **Debian / Ubuntu**:
  ```bash
  sudo apt install ./mark-shot_<version>_amd64.deb
  ```
- **Fedora**:
  ```bash
  sudo dnf install ./mark-shot-<version>-1.x86_64.rpm
  ```

The official `.deb` package is built on a Debian 12 compatibility baseline. It intentionally avoids linking the optional LayerShellQt plugin so that Deepin and other Debian-derived systems with Qt 6.8-era packages can install it without Ubuntu `t64` or newer GCC runtime dependencies.

> **Ubuntu 26.04 LTS**: DracoPho is verified and supported on Ubuntu 26.04 LTS
> ("Resolute"). Building from source on Ubuntu 26.04 uses the distro Qt 6.10
> packages directly (no `aqtinstall` step needed):
>
> ```bash
> sudo apt install build-essential cmake ninja-build pkg-config \
>   qt6-base-dev qt6-wayland libpipewire-0.3-dev libxcb-cursor0 \
>   xdg-desktop-portal pipewire xclip
> cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
> cmake --build build
> ```
>
> Headless capture (`--capture-to`), multi-display capture (repeatable
> `--display`), and the local MCP server all run on Ubuntu 26.04 under both
> Wayland (GNOME) and X11 sessions.

### Dependencies

#### Wayland (Arch Linux)

```bash
sudo pacman -S --needed base-devel cmake ninja pkgconf qt6-base qt6-wayland layer-shell-qt pipewire grim wl-clipboard
```

#### X11/GNOME (Ubuntu/Debian)

```bash
# Build essentials
sudo apt install build-essential cmake ninja-build pkg-config libpipewire-0.3-dev

# Portal and clipboard tools
sudo apt install xdg-desktop-portal pipewire xclip

# Qt 6 (if not available in system repos, install via aqtinstall)
pip install aqtinstall
aqt install-qt linux desktop 6.7.3 gcc_64 --outputdir ~/Qt
```

> **Note**: On Ubuntu 22.04 where the system ships Qt 5, installing Qt 6 to `~/Qt` keeps the system untouched. Pass `-DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64` when configuring.

#### fcitx5 Chinese Input Method (Qt 6 on X11)

Qt 6 does not ship a fcitx5 input context plugin. To enable Chinese input, build the plugin from source:

```bash
sudo apt install libfcitx5utils-dev libfcitx5config-dev libfcitx5core-dev libfcitx5-qt-dev extra-cmake-modules

git clone --depth 1 --branch 5.0.10 https://github.com/fcitx/fcitx5-qt.git /tmp/fcitx5-qt
cmake -B /tmp/fcitx5-qt/build -S /tmp/fcitx5-qt \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64 \
  -DENABLE_QT4=OFF -DENABLE_QT5=OFF -DENABLE_QT6=ON
cmake --build /tmp/fcitx5-qt/build

cp /tmp/fcitx5-qt/build/qt6/platforminputcontext/libfcitx5platforminputcontextplugin.so \
   ~/Qt/6.7.3/gcc_64/plugins/platforminputcontexts/
cp /tmp/fcitx5-qt/build/qt6/dbusaddons/libFcitx5Qt6DBusAddons.so* \
   ~/Qt/6.7.3/gcc_64/lib/
```

#### OCR Backend (Optional)

DracoPho delegates text recognition to the bundled `mark-shot-ocr` Python script. It supports **RapidOCR** (primary, based on PaddleOCR PP-OCR models) and **Tesseract** (fallback). On Linux the script is installed automatically; on Windows it must be configured manually.

<details>
<summary><b>Linux</b></summary>

```bash
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime
```

The installed `mark-shot-ocr` helper is discovered automatically—no config changes needed.

**Environment variables** (optional):

| Variable | Description | Default |
|----------|-------------|---------|
| `MARK_SHOT_OCR_VERSION` | PaddleOCR version (`PP-OCRv5`, `PP-OCRv4`, …) | `PP-OCRv5` |
| `MARK_SHOT_OCR_MODEL_TYPE` | Model size: `mobile` or `server` | `mobile` |
| `MARK_SHOT_OCR_MODEL_DIR` | Custom model storage directory | `~/.local/share/mark-shot/models` |
| `MARK_SHOT_OCR_NO_VENV` | Set to `1` to disable automatic venv re-exec | — |
| `MARK_SHOT_OCR_PYTHON` | Override the Python interpreter used for re-exec | `~/.local/share/mark-shot/ocr-venv/bin/python` |

</details>

<details>
<summary><b>Windows</b></summary>

The bundled helper scripts are not installed on Windows. Complete the following steps to enable OCR:

**1. Install Python 3**

Download and install Python 3.10 or later from [python.org](https://www.python.org/downloads/). Make sure to check **Add python.exe to PATH** during installation.

**2. Copy the OCR helper script**

Copy `scripts/mark-shot-ocr` from the [DracoPho repository](https://github.com/tystudio-26020701/mark-shot-community) to a local directory, for example `%LOCALAPPDATA%\mark-shot\mark-shot-ocr.py`.

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\mark-shot"
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/tystudio-26020701/mark-shot-community/main/scripts/mark-shot-ocr" `
  -OutFile "$env:LOCALAPPDATA\mark-shot\mark-shot-ocr.py"
```

**3. Create a virtual environment and install dependencies**

```powershell
python -m venv "$env:LOCALAPPDATA\mark-shot\ocr-venv"
& "$env:LOCALAPPDATA\mark-shot\ocr-venv\Scripts\pip.exe" install -U pip rapidocr onnxruntime
```

> `onnxruntime` provides CPU-based inference. If you have a compatible GPU, you can install `onnxruntime-directml` or `onnxruntime-gpu` instead for faster recognition.

**4. Configure `ocr.command` in `config.json`**

Open `%LOCALAPPDATA%\mark-shot\config.json` (create it if it does not exist) and set `ocr.command`:

```json
{
  "ocr": {
    "enabled": true,
    "backend": "rapidocr",
    "command": "\"%LOCALAPPDATA%\\mark-shot\\ocr-venv\\Scripts\\python.exe\" \"%LOCALAPPDATA%\\mark-shot\\mark-shot-ocr.py\" --format json --backend rapidocr {image}",
    "timeoutMs": 30000
  }
}
```

Replace `%LOCALAPPDATA%` with the actual expanded path (e.g. `C:\Users\YourName\AppData\Local`). The `{image}` placeholder is replaced with the temporary screenshot path at runtime; if omitted, DracoPho appends it automatically.

> **Tip**: Set the environment variable `MARK_SHOT_OCR_NO_VENV=1` to skip the script's built-in venv auto-detection, since the venv Python is already invoked directly.

</details>

#### Code Scan Backend (Optional)

```bash
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

The code scanner helper prefers `zxing-cpp` for QR Code, Data Matrix, Aztec, PDF417, EAN, UPC, Code 39, Code 93, Code 128, and other common formats. It can also fall back to `pyzbar` or OpenCV QR detection when those packages are available.

#### Image Upload Backend (optional)

The image upload feature uses the bundled `mark-shot-upload` Python script by default. It has no third-party dependencies (Python 3 standard library only) and is configured entirely through environment variables in `upload.env`. See the [Image Upload Configuration](#image-upload-configuration) section above for supported keys and provider examples.

For providers that return a plain-text URL instead of JSON (e.g. litterbox), set `upload.command` to a custom `curl` invocation—DracoPho auto-detects any stdout line starting with `http://` or `https://` as the upload result.

#### Windows

Install Qt 6 for your compiler toolchain, CMake, Ninja, and a C++17 compiler such as MSVC or MinGW. The Windows build does not require Qt DBus, PipeWire, X11/XCB, LayerShellQt, `grim`, `wl-copy`, or `xclip`.

```powershell
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
cmake --build build-windows
```

Windows support currently targets normal screenshots and image annotation. Scrolling capture, compositor-specific window detection, and Linux desktop entries are not available on Windows. The bundled Python helper scripts (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`) are not installed automatically—see the [OCR Backend](#ocr-backend-optional), [Code Scan Backend](#code-scan-backend-optional), and translation sections above for manual Windows setup instructions.

### Build Steps

```bash
# With system Qt 6
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# If Qt 6 is installed under the user directory, add CMAKE_PREFIX_PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64

# Build
cmake --build build
```

Or build with Nix:

```bash
nix build
```

LayerShellQt is detected automatically. When found, full Wayland layer-shell support is enabled. When absent, the build succeeds and falls back to standard fullscreen windows at runtime.

### Installation

```bash
cmake --install build --prefix "$HOME/.local"
```

This installs the binary, helper scripts (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`, `mark-shot-upload`), desktop entries, and icons.

### GNOME Wayland Scrolling Capture Extension

GNOME Wayland scrolling capture requires the **DracoPho Scroll Helper** extension. Without it, DracoPho cannot perform silent repeated area screenshots or display the GNOME-native scroll preview panel, causing the scrolling capture action to be disabled on GNOME Wayland.

The extension files are bundled in the project repository at `packaging/gnome-extension/mark-shot-scroll-helper@snemc.org`.

<details>
<summary><b>Expand/Collapse GNOME Wayland Scrolling Capture Extension Installation & Enable Guide</b></summary>

##### Method A: Installed from Distribution Package
If DracoPho was installed via a distribution package, the extension is already installed system-wide. Enable it with:
```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```
*If not found, log out and log back in, then retry.*

##### Method B: Installed from Repository Source
To manually install and enable the extension directly from the repository source folder:
```bash
# Define the extension UUID
UUID=mark-shot-scroll-helper@snemc.org

# Create the user GNOME extensions directory
mkdir -p "$HOME/.local/share/gnome-shell/extensions"

# Copy the extension files from the repository
cp -r "packaging/gnome-extension/$UUID" "$HOME/.local/share/gnome-shell/extensions/"

# Enable the extension (you may need to restart GNOME Shell or log out and back in)
gnome-extensions enable "$UUID"
```

Verify that the helper D-Bus interface is available:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
```

The expected result is `('4.2',)`. On GNOME Wayland, restart `mark-shot` after enabling the extension.

</details>

---

## Shortcuts & Interactive Gestures

### Tool Switching

| Hotkey | Tool | Description |
| :---: | :--- | :--- |
| **V** | Move / Pan | Moves and pans the image canvas (in local file mode). |
| **S** | Select | Selects, moves, scales, or deletes existing vector annotations. |
| **P** | Pen | Draws smooth freehand curves. |
| **L** | Line | Draws straight lines. |
| **H** | Highlighter | Semi-transparent highlight strokes. |
| **R** | Rectangle | Draws rectangular bounding boxes. |
| **E** | Ellipse | Draws elliptical bounding boxes. |
| **A** | Arrow | Draws classic pointy-tailed arrows. |
| **T** | Text | Types rich text (supports 1000px size and dual-gesture scale). |
| **N** | Number | Stamps sequential auto-incrementing numbered markers. |
| **M** | Mosaic | Covers sensitive data with acrylic frost blur. |
| **G** | Laser | Places temporary laser markings that dissolve automatically over time. |

### Startup Overlay Tools

| Hotkey | Tool | Description |
| :---: | :--- | :--- |
| **C** | Color Picker | Samples a screenshot pixel before selecting a region. Use the mouse wheel to resize the loupe, left click to open a color panel with copyable HEX, RGB, HSL, HSV, and Qt formats. Right click or Esc returns to normal selection. |
| **R** | Ruler | Measures coordinates before selecting a region. Hover reads the current pixel, and left-drag draws a measured rectangle with pixel ticks, width, height, diagonal, and area. Right click or Esc returns to normal selection. |
| **Q** | Code Scanner | Enters QR code and barcode scan mode. Select a region to decode codes inside it; the result opens in a copyable window. Right click or Esc returns to normal selection. |
| **D** | Display Capture | Instantly captures all outputs, crops the snapshot by display, and shows thumbnails with hover actions for copy, edit, and save. |

### Global Actions

| Hotkey | Action |
| :---: | :--- |
| **Esc** | Closes the screenshot/annotation window. |
| **Ctrl + C** | Confirms pending text edits and copies selection to system clipboard. |
| **Ctrl + S** or **Enter** | Confirms pending text edits and saves selection to a file. |
| **Ctrl + P** | Pins the current selection as a floating sticker window. |
| **Ctrl + U** | Uploads the current screenshot to the configured image host; the returned URL is copied to the clipboard. |
| **Ctrl + Z** | Undoes the last annotation. |
| **Ctrl + Y** or **Ctrl + Shift + Z** | Redoes the last undone annotation. |
| **Backspace** or **Delete** | Deletes the selected annotation object (under Select tool). |
| **F** | Toggles the active capture scope between selection and full screen. |

### Advanced Interaction Tips

- **Constrain Drawing**: Hold `Ctrl` while drawing Rectangles or Ellipses to constrain them to perfect squares or circles.
- **Quick Select Tool**: Right-click once on the canvas to switch to the **Select** tool instantly.
- **Quick Color Switch**: Double right-click on the canvas to open the radial color palette and quickly switch the active annotation color.
- **Scroll Wheel Regulation**: While a drawing tool is active, scroll the mouse wheel to dynamically adjust stroke width, text size, auto-increment number scale, or mosaic block size.
- **Canvas Zoom & Pan**: Under **Select** tool (or in local image mode), scroll the mouse wheel to zoom the canvas, and hold the middle mouse button to pan. Double-tap `Ctrl` to reset zoom and pan.

### Pinned Window Actions

| Gesture / Shortcut | Description |
| :--- | :--- |
| **Hold Left Click & Drag** | Repositions the floating window on your desktop. |
| **Scroll Wheel Up / Down** | Scales the floating window size proportionally. |
| **Double Left Click** | Closes the pinned window immediately. |
| **Right Click** | Opens the context menu (Rotate, Zoom, Always on Top, Copy Image Text, Translate, Save, Copy, Close). |
| **Esc Key** | Closes the currently active pinned window. |

---

## Release Notes

See [Release Notes](docs/releases.md).

## Feedback & Issues

If you encounter bugs or want to suggest new features, we recommend using GitHub CLI (`gh`) tool to submit an Issue. We provide templates and a script to automatically collect system information. For details, please refer to the [Issue Submission Guide](.doc/submit-issue-via-gh.md).

## License

This project is licensed under the **MIT License**. For details, please refer to the [LICENSE](LICENSE) file.

## Acknowledgements

DracoPho is built on the shoulders of the open-source community. We would like to express our sincere gratitude to:

- **The original upstream project [jswysnemc/mark-shot](https://github.com/jswysnemc/mark-shot) and its author and all contributors.** This community edition is developed on top of the original upstream project, whose outstanding design and sustained contributions made everything here possible. We sincerely thank them for their great work.
- **[serendipitywgy](https://github.com/serendipitywgy)** for contributions from `serendipitywgy/mark-shot`, including cross-desktop compatibility improvements, the OCR copy toolbar action, and smart rectangle preselection.
- **All the open-source projects that DracoPho depends on**, including Qt 6, PipeWire, xdg-desktop-portal, layer-shell-qt, wl-clipboard, xclip, grim, RapidOCR, onnxruntime, Tesseract, and ZXing-C++, among others.

This community edition is maintained by [Beijing Taiyin Zhaowu Technology Co., Ltd.](https://github.com/tystudio-26020701/mark-shot-community) and its contributors, under the **MIT License**.
