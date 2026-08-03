# Release Notes

### 26.8.1.1

> **Mark Shot Community Edition** — maintenance release fixing settings-window
> unsaved-changes reporting and multi-screen freeze behavior, with ten new
> localized README editions and documentation fixes.

#### Fixes

**Settings window unsaved-changes reporting**
- **Deterministic dirty-state tracking**: every settings control's value-change signal (combo-box selection, check-box toggles, spin-box values, text edits, key-sequence edits) drives the unsaved-changes indicator directly instead of relying on events landing inside the settings window. Combo-box popup selections previously skipped the refresh — changing a value did not mark the dialog dirty and reverting it still blocked closing. Both false negatives and false positives are now gone.
- **Color-picker refresh**: default annotation color and pinned-border color changes via the modal color dialog now refresh the dirty indicator after the dialog returns.
- **Precision-normalized baselines**: corner radius, shadow opacity, translation temperature and pinned border width are normalized to the widget precision, so hand-written fractional config values no longer show as unsaved right after opening Settings.

**Multi-screen freeze scope**
- **Frozen backdrop on other displays**: with the default "Freeze All Screens" scope, committing a selection on one monitor keeps the other displays frozen and fully non-interactive (mouse, keyboard, wheel and shortcuts are swallowed; toolbars are hidden) instead of closing those overlays and leaving the screens operable. The session ends when the active selection window closes; closing a frozen backdrop independently no longer interrupts the active edit.

**Documentation**
- README available in ten additional languages (Traditional Chinese, Japanese, Korean, Russian, Italian, Arabic, French, German, Spanish, Portuguese) in a `READMEs/` subfolder, with a language selector in the root README.
- Fixed the issue-submission-guide relative link in all twelve user-guide language editions.

### 26.8.1.0

> **Mark Shot Community Edition** — the first release under the new
> `年.月.版.微调` versioning scheme, built on top of the original upstream
> project `jswysnemc/mark-shot`. Release packages: Linux `.tar.gz` / `.deb` /
> `.rpm` / `.AppImage` / `.flatpak`, Windows `.zip`, and Arch packages.

#### New Features

**Headless Capture CLI**
- `--capture-to <path>` captures the screen without opening the annotation UI and writes a PNG with a compact JSON summary. It reuses all interactive capture backends (QScreen, xdg-desktop-portal, PipeWire, grim, KWin/GNOME helpers, Windows Graphics Capture).
- `--region x,y,w,h` captures a logical screen region; `--display <name>` selects a monitor and may be repeated to capture several monitors at once; `--include-cursor` draws the cursor; `--output-name` sets the generated file name; `--list-displays` prints outputs as JSON.
- `--list-windows` enumerates visible windows (index/id/title/class/instance/geometry, optional z-order) on Wayland (GNOME/KDE/Hyprland/niri) and X11; `--window <selector>` (repeatable, `auto/id/title/class/index` matching, with `<selector>@x,y,w,h` for window-internal component sub-regions) captures windows.
- `--capture-destination inline|file|stage|clipboard`: inline returns base64 without touching the filesystem or clipboard, file writes to `--capture-to`, stage writes to `$TMPDIR/mark-shot-staging`, clipboard enters the system clipboard only when explicitly requested. Defaults and the clipboard permission are configurable in Settings → Storage (passphrase-gated, off by default).
- Headless mode opens no window, shows no interactive portal dialog, and exits immediately; the window list is unchanged before and after capture.

**Annotation Text Control**
- The text font panel provides an exact point-size input (20–300 pt with floating-point precision), a font family list, and Bold / Italic toggles. They apply to new text, the inline editor and existing annotations, and persist across sessions.
- New annotations now default to a normal 20 pt instead of 63 pt.

**Settings & UI**
- **Multilingual interface**: 12 UI languages — English, Simplified/Traditional Chinese, Japanese, Korean, Russian, Italian, Arabic (RTL), French, German, Spanish, Portuguese — following the system locale or `MARK_SHOT_LANG`, switchable instantly from Settings → General.
- **Unsaved-changes protection**: categories show an unsaved-changes indicator, the footer and title warn about pending edits, and closing asks Save and Close / Discard and Close / Keep Editing (including Escape).
- **About page**: below Advanced, showing the software icon, version, community-edition repository link, company logo and an acknowledgment of the original upstream project and its contributors.
- **Settings scroll guard**: hover-scrolling over combo boxes, spin boxes and sliders scrolls the page instead of mutating values; a source-level wheel suppressor makes this robust regardless of focus.
- **Precise wheel scrolling**: fractional wheel deltas accumulate like Qt's native accumulator; Ctrl/Shift + wheel scrolls by page; momentum phases reset cleanly.
- **Per-section restore**: every Settings section and page has a Restore button; the Advanced page also offers a confirmed factory reset.
- **Shortcut input safeguards**: modifier-only and dangerous keys are rejected, global hotkeys require a modifier or function key, duplicates are flagged, and the default Escape cancel shortcut is preserved.

**System Integration**
- **Native X11 global shortcuts**: `xcb_grab_key` registration (NumLock/CapsLock variants, native event dispatch) on X11, portal backend on Wayland; modifier masks resolved from the live X11 map so swapped Alt/Super layouts and xmodmap work, with re-grab on mapping changes.
- **F25–F35 function keys** supported consistently across the UI, X11 keysym lookup and portal triggers.
- Capture overlays are excluded from the taskbar/Alt-Tab on Windows and X11.
- Region selection freezes all screens by default and supports cross-monitor selections on X11/Windows.

#### Bug Fixes

- **Full-screen flicker / temporary screen corruption after capturing on Linux**: the capture overlay no longer performs a delayed 1 px resize round-trip on Wayland (that delta made the compositor alternate between fullscreen scan-out and regular composition, flickering the whole desktop on dual-monitor / fractional-scaling setups); the overlay is placed at the exact screen geometry once.
- **Focus-fight flicker on Wayland**: `raise()`/`activateWindow()` right after showing the fullscreen overlay are skipped on Wayland and the overlay sets `WA_ShowWithoutActivating`, avoiding the "focus repeatedly granted/removed" loop with GNOME.
- **Overlay paint churn / drop-shadow artifacts**: the capture overlay sets `WA_OpaquePaintEvent` + `WA_NoSystemBackground` (no double-draw flashes on expose/resize) and `NoDropShadowWindowHint` (no shadow composition for a full-screen overlay); the scroll-capture overlay gets the same drop-shadow flag.
- Settings window wheel-scroll tampering of unfocused spin boxes, dropdowns and sliders.
- Settings wheel-guard segfault caused by re-entrant event dispatch (Qt 6.11 gesture manager).
- About page literal `%1` placeholders and a non-scrollable, truncated layout.
- `MARK_SHOT_LANG` session override no longer overridden when Settings opens.
- Default Escape cancel shortcut no longer silently cleared on reload.
- Text size mapping and default (63 pt → 20 pt).
- Window hover-selection z-order and compositor script selection on unknown Wayland sessions; custom detection commands are respected.
- Headless capture edge cases (output naming, geometry handling).
- Clipboard persistence hang in headless mode (detached owner process with redirected stdio; skip the Wayland `QClipboard::setImage` ownership round-trip).
- PipeWire buffer helpers compile without PipeWire headers (`HAVE_PIPEWIRE` guard).

#### Documentation

- User guide in 12 languages.
- Headless CLI chapters (screen/multi-monitor/window/component capture, destinations, clipboard policy) and window hover-selection guidance.
- Ubuntu 26.04 LTS support declared and multi-display capture documented.
- README now acknowledges the original upstream project, its contributors, and the open-source dependencies.

### 0.1.41

- **Windows Build Compatibility**: PipeWire SPA buffer helpers and their dedicated tests now compile only on Linux, restoring Windows builds and release packages while preserving Linux PipeWire capture behavior.

### 0.1.40

- **Runtime Capture Settings**: Cursor inclusion, freeze scope, and default annotation tools are now read for every capture, so settings changes apply without restarting Mark Shot.
- **Default Move Tool**: New installations now start annotation editing with the Move tool instead of Pen unless an explicit default is configured.
- **Draggable Toolbars**: The annotation toolbar and right-side action toolbar now include drag grips and retain their user-selected positions during editing.
- **Cursor Feedback**: The Move tool uses an arrow outside the selection, while toolbars, property panels, color controls, font lists, extension panels, and combo box popups no longer inherit drawing crosshairs.
- **Text Annotation Layout**: Text backgrounds include additional width padding to prevent premature wrapping in the editor.
- **KDE and PipeWire Capture**: Improved KWin own-window handling, KDE Wayland capture compatibility, and PipeWire buffer data-type processing with expanded tests.

### 0.1.39

- **Wayland Multi-Monitor Capture**: Fixed mixed-scale multi-monitor screenshots by capturing Wayland outputs independently, preventing half-screen selection and incorrectly scaled overlays.
- **Niri DMS Window Geometry**: The niri window detector now reads DMS bar, dock, frame, and frame-exclusion settings so tiled-window selection aligns with the visible window bounds.
- **Pinned Windows Across Outputs**: Pinned layer-shell windows now rebind to the target output while being dragged, so images remain visible after moving between monitors.
- **Image Frame Default**: The optional macOS-style export frame is now disabled by default. Existing configurations can still enable it with `export.imageFrame.enabled`.
- **Capture Window Visibility Setting**: `capture.hideOwnWindows` is now read at capture time and applied consistently to single-screen and multi-screen paths, so settings changes take effect without restarting.
- **Number Badge Rotation**: Number annotations now rotate around the badge center, with matching hit testing and selection geometry.
- **Standalone Plugin Assets**: Release builds now publish provider plugins as separate checksummed assets, with a stabilized Rapid OCR plugin build.
- **Packaging and Documentation**: Arch packages now depend on FFmpeg for recording support, all package versions are synchronized, and installed packages include the linked configuration and release documentation.


### 0.1.38

- **Plugin Ecosystem Foundation**: Added provider plugin registration, user-level plugin directories, provider preference configuration, and a Plugins settings page for OCR, translation, and code scanning extensions.
- **GitHub Plugin Marketplace**: Added the C++/Qt plugin index parser, download, SHA-256 verification, and install flow. The marketplace can be hosted entirely on GitHub Releases without requiring Python.
- **C++ Rapid OCR Plugin Upgrade**: Rapid OCR now emits word-level tokens, splits Chinese text and punctuation into selectable characters, splits Latin text by whitespace, and reuses existing RapidOCR model directories.
- **Pinned Text Selection Fixes**: Fixed half-width highlight backgrounds for full-width Chinese characters and avoided unintended spaces when copying adjacent Chinese OCR tokens.

### 0.1.37

- **Windows Recording Audio**: Added native WASAPI loopback capture for Windows video recording, replacing the PulseAudio-only path on Windows.
- **Windows Release Packaging**: Enabled FFmpeg-backed Windows packages with runtime DLL deployment and Authenticode signing support for executables and DLLs.
- **Windows CI Build Fixes**: Fixed WASAPI GUID and recording test linkage so Windows builds, tests, signing, packaging, and artifact upload complete successfully.

<details>
<summary>Previous versions</summary>

### 0.1.36

- **Older PipeWire Build Compatibility**: Fixed Debian 12 / older PipeWire header builds by probing `spa_video_info_raw::flags` at configure time while keeping explicit DMA-BUF modifier detection on newer PipeWire versions.

### 0.1.35

- **Qt 6.4 DMA-BUF Build Compatibility**: Fixed Debian 12 / Qt 6.4 builds by guarding Qt Wayland native display access while preserving the Wayland EGL display path on Qt 6.5 and newer.

### 0.1.34

- **Theme Setting**: Added `ui.theme` with System, Dark, and Light options, including a General settings selector and immediate settings-dialog theme application.
- **PipeWire Recording Backend**: Improved Wayland recording capture with shared-memory and DMA-BUF PipeWire frame handling, plus wlroots screencopy and polling fallbacks when portal capture is unusable.
- **Recording Timeline Accuracy**: Aligned the recording status timer with saved video timestamps so portal authorization and capture startup delay are not counted in the displayed duration.
- **Settings Polish**: Localized the theme setting controls and normalized combobox and spinbox styling across widget styles.

### 0.1.33

- **GIF and Video Recording**: Added GIF and MP4 recording with stepped frame rates, display or region capture, optional video audio input, and configurable output directories.
- **Tray and CLI Recording Controls**: Added tray Start Recording and Stop Recording actions, live tray status, `--recording-status`, and `--stop-recording`.
- **Recording-Aware Capture Overlay**: Active recordings now show status in the frozen-frame overlay and can be stopped with `S` or the overlay button without blocking normal screenshots.
- **Save and Recording Notifications**: Added desktop notifications for recording start/save/failure and screenshot save completion.
- **Recording Dialog Updates**: The recording dialog now switches between GIF and video modes, defaults to the current display, and updates frame rate, audio, and output path controls as the mode changes.
- **Wayland and Text Selection Fixes**: Improved mixed-DPI Wayland capture placement and fixed right-click context menus so editable text selections are preserved.

### 0.1.32

- **Startup Shortcut Hint Panel**: Replaced the centered startup hint pill with a PixPin-style vertical shortcut panel that defaults to the left-bottom corner and moves to the left-top corner when the pointer approaches it.
- **Input Device Hints**: Added keyboard, mouse, and mouse-wheel glyphs to the startup shortcut panel so shortcut rows communicate the expected input method more clearly.
- **Window Z-Order Selection**: Improved window ordering across GNOME, KDE Plasma, Hyprland, X11, and Windows so region selection prefers the visually topmost matching window.
- **Wayland Fcitx5 Candidate Support**: Adjusted layer-shell cursor-rectangle handling so fcitx5 candidate windows appear correctly under Wayland capture overlays.
- **Settings Gear Icons**: Redrew the settings toolbar and General settings navigation icons as clearer gear glyphs instead of sun-like radial icons.
- **Tray Mode Compatibility**: Fixed startup behavior when Mark Shot is launched directly into tray mode on environments without an immediately available system tray.
- **Wayland Text Editor Width**: Prevented the annotation text editor from shrinking unexpectedly on fractional-scale Wayland displays.

### 0.1.31

- **CLI Image Pinning**: Added `--pin-image <path>` to open an existing local image directly as a pinned sticker window, skipping capture and selection.
- **Color Picker History**: The startup Color Picker now remembers recently picked colors, persisted in `config.json` under `colorPicker.history` (capped at 7 `#RRGGBBAA` entries) and shown as swatches in the color panel.
- **Interface Language Setting**: Added a configurable `ui.language` option (`system` / `english` / `chinese`) selectable from the General settings page; supersedes the legacy root-level `language` key.
- **Desktop-Aware Window Detection**: Mark Shot now auto-selects the matching window detection script at runtime (GNOME, KDE Plasma, Hyprland, Niri), falling back to the niri script on other Wayland sessions and native X11 detection on X11. Mismatched configured commands are corrected in memory without touching `config.json`.
- **GNOME Occluded Window Filtering**: The GNOME Shell scroll helper extension now filters fully occluded windows from detection results.
- **Prebuilt AUR Package**: Added a `mark-shot-bin` AUR package installing prebuilt pacman packages from GitHub Releases, alongside the source-based `mark-shot` package.
- **GNOME Adwaita Palette Fix**: Overrode the application palette at the `qApp` level so the dark palette fully replaces the libqtk3 base palette under GNOME Adwaita.
- **AUR Optional Dependencies**: Added `python-rapidocr`, `python-pillow`, and `python-zxing-cpp` as preferred OCR/code-scan optdepends.

### 0.1.30

- **Settings Configuration Dialog**: Added a dedicated settings window with pages for General, Capture, Annotation, Pinned, Scroll, Shortcuts, Storage, Integrations, and Advanced. Every previously file-only option is now editable in one place, backed by the same `config.json` store, with shared design tokens and a custom navigation sidebar.
- **Launch on Startup**: Added a `Launch on Startup` switch on the General page. Linux writes an XDG `autostart` desktop entry; Windows writes the current user's `Run` registry key. The switch disables itself on unsupported platforms.
- **Portal Global Shortcut Support**: Added an `xdg-desktop-portal` `GlobalShortcuts` backend so global capture hotkeys work on Wayland without X11.
- **Pinned Text Selection Toggle**: The pinned image window now exposes a configurable text-selection toggle, with `pinned_window_config` split into its own module.
- **Settings Entry During Capture**: Opening settings from the toolbar or shortcut now closes the frozen capture session first and defers the dialog to the next event-loop tick, avoiding conflicts with the layer-shell capture window.
- **Pinned Window Placement on Wayland**: Extracted layer-shell geometry computation and improved the resize controller, fixing off-screen and multi-monitor placement of pinned windows.

### 0.1.29

- **Independent Magnifier Frame Resize**: The magnifier annotation now exposes resize handles on both the inner source viewfinder and the outer lens. Rectangle lenses get 8 corner/edge handles per frame, circular lenses get 4. Resizing either frame keeps `magnifierScale` constant by scaling the other frame proportionally, so the loupe ratio stays consistent regardless of which side the user grabs.
- **Rectangle Highlight & Invert Styles**: The rectangle tool gains a style selector with three modes—`Stroke` (existing outlined / filled rectangle with optional rounded corners), `Highlight` (marker-pen overlay using `CompositionMode_Multiply` with semi-transparent fill), and `Invert` (inverts the RGB pixels covered by the rectangle while keeping the outline as a visual cue). Fill toggle and corner radius are hidden for `Highlight` and `Invert`.
- **Persistent Tool Defaults**: Annotation tool defaults (color, opacity, per-tool widths, rectangle fill / corner radius / style, magnifier scale and shape, arrow / highlighter / number style, text font, text background color) now survive across sessions through a dedicated `annotation-state.json` file. Writes are atomic via `QSaveFile` and triggered immediately after every default-changing entry point.

### 0.1.28

- **Configurable Clipboard Image Policy**: Added `clipboard.image.mode` with `image/png`, `url`, and `threshold` modes. The default now keeps direct `image/png` clipboard data for better compatibility with office suites and browser input fields, while `thresholdM` can still switch large images to file URL mode.
- **Default Runtime Config Creation**: Ensured runtime startup creates a default `config.json` when the file is missing, including the new clipboard defaults.
- **Shift-Constrained Line Drawing**: Holding `Shift` while drawing Line, Arrow, or straight Highlighter annotations now snaps the stroke to horizontal, vertical, or 45-degree directions.

### 0.1.27

- **Multi-point Line/Arrow Skeleton Editing**: Introduced support for adding, dragging, and deleting multiple skeleton (control) points on line and arrow annotations. Paths are smoothed using continuous quadratic Bezier curves, ensuring endpoints precisely target endpoints.
- **Shortcut & Interaction Improvements**: Enhanced keyboard and scroll interactions (e.g. using Backspace/Delete to remove selected skeleton points), and refactored input shortcut processing logic.

### 0.1.26

- **Custom Save Path & Placeholders**: Introduced flexible screenshot save templates (`save.pathTemplate` and `save.directoryTemplate`), supporting 30+ dynamic placeholders like `{pictures}`, `{datetime}`, and custom formatting like `{datetime:yyyy-MM-dd}` for versatile directory structures and naming schemes.
- **KDE KWin Screenshot Control Switch**: Added the `capture.wayland.kde.kwinScreenshot.enabled` option to enable or disable using KWin's restricted `org.kde.KWin.ScreenShot2` D-Bus interface, facilitating fallback debug routines.
- **Document Layout Optimization & Details Collapsing**: Refactored the user guide to collapse long KDE DBus setup details and application configuration parameters, improving overall readability.

</details>

---
