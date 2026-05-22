# Mark Shot

[中文说明](README.zh-CN.md)

<video src="https://github.com/user-attachments/assets/c2298867-06b4-404d-87bc-62ab8d81088b" width="100%" controls></video>

`mark-shot` is a high-performance, rapid Wayland screenshot and annotation tool built with Qt 6, deeply optimized at a native level for modern Wayland compositors such as `niri`.

It captures lossless screen frames instantly via `grim` and opens an adaptive fullscreen overlay layer, providing users with seamless area crop, rich annotation, clipboard copying, saving, and premium Snip-Pinning services.

---

## Core Capabilities & Innovations

### Comprehensive & High-Fidelity Annotation Toolset
- **Pen & Highlighter**: Supports smooth freehand line drawings and semi-transparent overlay highlighting.
- **Geometric Vectors**: High-precision Line, Rectangle, and Ellipse shapes.
- **Streamlined Acute Arrow**: A mathematically remodeled 6-vertex classic arrow path. The tail tapers down to a sleek point while the nose angle is made sharper and more acute, featuring high-quality anti-aliased path rendering.
- **Dual-Gesture Advanced Text**:
  * **1000px Scale Bound**: Font size bounds are fully expanded to `1000px`, adjustable via scroll wheel or property sliders.
  * **Unexpected Wrap Prevention**: Incorporates a `5%` safe physical width expansion algorithm that compensates for non-linear font Hinting and Kerning across extreme scales, keeping text single-row and cropping back to perfect pixel bounds.
  * **Dual-Gesture Handles**: Dragging **diagonal corners** scales the font size and the bounding box proportionally; dragging **side borders** only resizes the wrapping width to re-arrange layout lines without altering the font size.
- **Laser Pointer**: A dedicated teaching tool whose trace dissolves smoothly over time, perfect for presentations or tutorials.
- **Auto-Increment Number**: Click to stamp sequential numbered markers for multi-step instructions.
- **Area Mosaic**: Applies high-fidelity acrylic frost-glass blur to conceal sensitive information.

### Snip-Pinning Experience (Floating Overlay Sticker)
- **Local Pin-to-Desktop**: Instantly pins any screenshot or cropped annotation as an independent, frameless, and top-level floating window (`PinnedImageWindow`).
- **Flexible Pin Controls**:
  * **Drag & Move**: Hold the left mouse button to drag the sticker anywhere on the desktop.
  * **Seamless Scaling**: Scroll the mouse wheel to scale the pinned window, or double-tap the `Ctrl` key to reset it to original size.
  * **Instant Dismiss**: Double-click the left mouse button or press the `Esc` key to close the pinned sticker.
  * **Context Menu**: Rotate, adjust opacity (0.2 to 1.0), save as, copy to clipboard, or close.
- **Accidental Click Filter**: In Qt 6 environments, the right-click menu might accidentally trigger options during pop-up. We designed and installed a custom `LeftClickMenuFilter` that blocks non-left-click triggers within the menu area, providing refined interaction ergonomics.

### Wayland Native & System Integration
- **Wayland Overlay Layer**: Native `layer-shell-qt` overlay layout by default for seamless fullscreen coverage; falls back gracefully to a generic XDG fullscreen window via `--xdg-window`.
- **Dual Desktop Entries & SVG Icons**:
  * `mark-shot.desktop`: Configured as a system-wide utility, triggerable by user custom hotkeys.
  * `mark-shot-edit.desktop`: Configured as a "Mark Shot Image Editor". Seamlessly registers in file manager context menus (such as Dolphin or Nautilus) to directly open and annotate existing local files.
  * Packaged with high-resolution `mark-shot.svg` and `mark-shot-edit.svg` vector icons.

---

## Command Line Interface (CLI)

`mark-shot` provides flexible command-line arguments to suit different usage environments:

| Option | Description |
| :--- | :--- |
| `[file]` | **Positional Argument**: Opens an existing local image file in annotation mode instead of capturing the screen. Accepts only one file at a time. |
| `-h`, `--help` | Displays help information and exits. |
| `-v`, `--version` | Displays version information and exits. |
| `--all-outputs` | Captures all screens on the virtual display environment instead of only the current Qt active screen. |
| `--xdg-window` | Forces the use of a standard XDG fullscreen window (xdg-shell) instead of the default Wayland layer-shell. |
| `--fullscreen` | Skips the region crop step and opens annotation mode on the full captured screen frame directly. |

---

## Compilation & Installation

### System Dependencies (Arch Linux)

Install the necessary dependencies before building the project:

```bash
sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-wayland layer-shell-qt grim wl-clipboard
```

### Build Steps

Build the binary using CMake and Ninja:

```bash
# Configure the project outputting to the build directory
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build the executable
cmake --build build
```

Once successful, the binary will be located at `./build/mark-shot` and can be run directly.

### System Integration & Installation

To install the application and automatically register desktop launchers and system mime associations:

```bash
cmake --install build --prefix "$HOME/.local"
```

Once installed, **Mark Shot Image Editor** will be visible in your desktop launcher menus and file manager "Open With" context dialogs.

---

## Shortcuts & Interactive Gestures

Use the following keyboard shortcuts in annotation mode for maximum efficiency:

### Tool Switching Keys

| Hotkey | Targeted Tool | Description |
| :---: | :--- | :--- |
| **V** | Move / Pan | Moves and pans the image canvas (in local file mode). Double-tap Ctrl to reset zoom. |
| **S** | Select | Selects, moves, scales, or deletes existing vector annotations. Right-click area to pin. |
| **P** | Pen | Draws smooth freehand curves. |
| **L** | Line | Draws straight lines. |
| **H** | Highlighter | Semi-transparent highlight strokes. |
| **R** | Rectangle | Draws hollow rectangular bounding boxes. |
| **E** | Ellipse | Draws hollow elliptical bounding boxes. |
| **A** | Arrow | Draws the refined classic pointy-tailed arrow. |
| **T** | Text | Types and shapes rich text (supports 1000px size and dual-gesture scale). |
| **N** | Number | Stamps sequential auto-incrementing numbered markers. |
| **M** | Mosaic | Covers sensitive data with acrylic frost blur. |
| **G** | Laser | Places temporary laser markings that dissolve automatically over time. |

### Global Action Keys

| Hotkey | Action |
| :---: | :--- |
| **Esc** | Instantly closes the screenshot/annotation window. |
| **Ctrl + C** | Confirms all pending text and copies the screenshot/selection to the Wayland system clipboard. |
| **Ctrl + S** or **Enter / Return** | Confirms all pending text and saves the screenshot. |
| **Ctrl + Z** | Undoes the last annotation edit. |
| **Ctrl + Y** or **Ctrl + Shift + Z** | Redoes the last undone annotation. |
| **Backspace** or **Delete** | Deletes the selected annotation (when in **Select** tool mode and an object is active). |
| **F** | Toggles the active capture scope. |

### Pinned Window Actions (`PinnedImageWindow`)

| Gesture / Shortcut | Description |
| :--- | :--- |
| **Hold Left Click & Drag** | Freely moves and places the floating sticker on your desktop. |
| **Scroll Wheel Up / Down** | Scales the floating sticker window proportionally and seamlessly. |
| **Double Left Click** | Closes the pinned window immediately. |
| **Double Tap Ctrl Key** | Resets the pinned window size back to its original physical aspect ratio. |
| **Right Click** | Opens the feature context menu (Rotate, Opacity, Save, Copy, Close). |
| **Esc Key** | Closes the currently focused pinned sticker window. |

---

## License

This project is open-source and licensed under the **MIT License**. For details, please refer to the [LICENSE](LICENSE) file.
