<div align="center">
  <img src="../data/icons/hicolor/scalable/apps/mark-shot.svg" alt="Mark Shot Logo" width="128" />
  <h1>Mark Shot</h1>
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

[English README](../README.md)

其他語言版本：
[简体中文](../README.zh-CN.md) · [繁體中文](./README.zh-TW.md) · [日本語](./README.ja.md) · [한국어](./README.ko.md) · [Русский](./README.ru.md) · [Italiano](./README.it.md) · [العربية](./README.ar.md) · [Français](./README.fr.md) · [Deutsch](./README.de.md) · [Español](./README.es.md) · [Português](./README.pt.md)

**標籤**：`C++` / `Qt 6` / `螢幕截圖` / `圖像標註` / `桌面貼圖` / `OCR 辨識` / `捲動長截圖` / `Wayland` / `Windows`


<details>
<summary>示範影片</summary>
<p align="center">
  <video src="https://github.com/user-attachments/assets/4f86fcee-fef9-409e-98ba-1491ecee06c7" width="100%" controls></video>
</p>
</details>

`mark-shot` 是一款基於 Qt 6 開發的高效能截圖標註工具。專案最初針對 `niri` 等 Wayland 視窗管理員設計，目前支援在 Linux（X11、GNOME、wlroots/Wayland 桌面）以及 Windows 環境中完成一般截圖與標註工作流程。

它可以瞬間擷取螢幕畫面，並開啟自適應全螢幕標註覆蓋層，為使用者提供區域裁切、標註、複製到剪貼簿、儲存以及桌面貼圖等功能。

---

## 核心特色

### 標註工具箱
- **畫筆與螢光筆**：支援平滑的自由線條繪製與半透明高亮疊色。
- **幾何向量工具**：高精度的直線、矩形與橢圓路徑。其中矩形支援三種風格切換：
  - `描邊`：原有的描邊或填滿矩形，可選圓角。
  - `高亮`：以 `CompositionMode_Multiply` 與半透明填滿實現的螢光筆式覆蓋效果。
  - `反色`：對矩形覆蓋區域內的像素做 RGB 反相，同時保留外輪廓作為視覺提示。
- **最佳化箭頭**：採用六頂點經典箭頭路徑，邊緣平滑且支援抗鋸齒渲染。
- **雙重聯動文字**：
  - 支援超大字級無段調整，可透過滑鼠滾輪或屬性滑桿平滑縮放。
  - 導入物理寬度緩衝區設計，避免文字在極高縮放比例下因渲染抖動而產生意外的自動換行。
  - **對角控制點**可實現字級與文字框的等比例聯動縮放；**左右邊控制線**則僅調整排版邊界寬度。
- **雷射示範筆**：適用於簡報或教學，筆跡會隨時間平滑融解消失。
- **自動遞增步驟序號**：點擊即可放置依序遞增的數字步驟標記。
- **馬賽克**：支援對敏感資訊執行毛玻璃區域模糊化。
- **雙框獨立調整的放大鏡**：放大鏡的內層取景框與外層透鏡各自帶有 resize 把手，矩形透鏡每框 8 個角/邊把手，圓形透鏡每框 4 個上下左右把手。調整任一框時會依放大倍率聯動另一框，倍率始終保持不變；平移單框時另一框保持原位。
- **啟動階段掃碼**：選取前按 `Q` 進入掃碼模式，框選 QR Code 或條碼區域後，會開啟可複製的辨識結果視窗。
- **快速擷取顯示器**：選取前按 `D` 會立刻擷取全部輸出螢幕，再按顯示器裁切成縮圖；將滑鼠懸停到縮圖上可複製、編輯或儲存該顯示器的截圖。
- **GIF、MP4 與動畫 WebP 錄製**：透過啟動階段錄製快捷鍵或系統匣選單，可以把指定顯示器或自訂區域錄製為 GIF、MP4 或動畫 WebP。錄製具備當機安全設計：MP4 會先寫入暫存的 MKV，並在結束時重新封裝，因此中斷的錄製仍然可以復原。進行中的錄製會在系統匣與凍結幀中顯示狀態，可用 `S`、覆蓋層按鈕、系統匣選單或 `--stop-recording` 停止，並在開始和儲存時發送桌面通知。在 Wayland 上，錄製優先使用 PipeWire portal 後端；當 portal 擷取不可用時，可回退到 wlroots screencopy 或輪詢採集。
- **圖床上傳**：選取後按 `Ctrl+U` 或點擊工具列上傳按鈕，將目前截圖上傳到自訂圖床（如 ImgURL、sm.ms、imgbb、litterbox 等），上傳成功後 URL 自動複製到剪貼簿。支援透過 `upload.env` 設定圖床參數，或透過 `upload.command` 接入任意自訂上傳腳本。
- **Mac 風格匯出外框**：為儲存、複製、上傳、開啟方式與擴充指令圖片新增透明邊距、圓角和柔和陰影。

### 貼圖懸浮固定（Pin）
- 支援將截圖或標註區域作為一個獨立、無邊框且置頂的懸浮貼圖視窗固定在螢幕上。
- 支援在貼圖視窗中直接選取 OCR 辨識出的文字，使用 `Ctrl + C` 或右鍵選單複製圖片文字。
- 支援透過 OpenAI 相容介面呼叫 LLM 翻譯 OCR 文字，並將譯文依原圖位置覆蓋渲染到貼圖上。
- **便捷互動**：
  - 滑鼠左鍵拖曳可自由平移貼圖位置。
  - 滾動滑鼠滾輪可等比縮放貼圖。
  - 雙擊滑鼠左鍵或按下 `Esc` 鍵即可關閉貼圖。
  - 右鍵點擊喚出選單，支援多角度旋轉、複製圖片文字、翻譯、另存新檔、複製或關閉。

### 捲動截圖
- 透過 PipeWire screencast、互動式捲動覆蓋層和影像拼接器，擷取長頁面或長區域截圖。
- 此功能主要面向 `niri` 以及行為相近的 Wayland 環境；這些環境的輸出幾何、擷取時序和視窗位置更容易保持穩定。
- **大選區懸浮把手**：當選擇的截圖區域過大，以致螢幕剩餘空間不足以展示捲動預覽面板時，預覽面板會自動隱藏，並在選區邊緣顯示一個**懸浮拖曳把手**（帶方向箭頭的懸浮按鈕）。
  - **拖曳調整選區**：可按住並拖曳該懸浮把手，將截圖選區沿捲動軸方向平移，以擷取超出初始螢幕範圍的內容；
  - **點擊切換軸向**：在尚未開始擷取前，點擊懸浮把手可直接切換捲動方向（垂直/水平）。
- **相容性說明**：KDE、GNOME、X11 以及其他非 `niri` 環境中的捲動截圖仍是測試功能，尚未完善。這些桌面棧的 portal 後端策略、Shell 或視窗管理員行為、視窗幾何回饋、幀時序和捲動事件處理存在差異。
- 如果捲動截圖無法使用，請使用一般截圖流程，或者透過 Mark Shot 擴充指令接入外部長截圖工具。
- 如果需要提交捲動截圖問題，請先執行 `mark-shot --debug --debug-log /path/to/mark-shot.log` 並重現問題，然後把日誌附到 GitHub issue 中。也可以在 `config.json` 中透過 `debug.enabled` 與 `debug.logPath` 開啟；`DEBUG=1` 與 `MARK_SHOT_DEBUG_LOG=/path/to/log` 仍然可用。

### 跨顯示伺服器支援
- **Wayland**：使用 PipeWire portal screencast 支援錄製與實驗性捲動截圖，並處理共享記憶體與 DMA-BUF 兩類幀路徑；使用 `grim` 支援 wlroots 截圖，使用 `layer-shell-qt` 建立原生覆蓋層，使用 `wl-copy` 持久化剪貼簿。
- **X11**：使用 `QScreen::grabWindow` 截圖、全螢幕置頂視窗作為覆蓋層、`xclip` 持久化剪貼簿。
- **Windows**：使用 Qt 原生截圖與剪貼簿 API 支援基礎截圖、標註、複製、儲存和貼圖流程。PipeWire、xdg-desktop-portal、`grim`、XCB 視窗偵測、LayerShellQt、GNOME Shell helper 等 Linux 專用後端會在編譯期關閉。
- Linux 顯示伺服器後端會在執行階段透過 `$XDG_SESSION_TYPE` 自動偵測；Windows 使用 Qt 原生平台後端。
- **Multi-monitor freeze scope**：預設情況下，區域選取會凍結所有已連接的顯示器（在 X11/Windows 上 DPR 相符時，顯示為單一虛擬桌面視窗）；在一個顯示器上確定選取後，其他顯示器會保持凍結且無法互動，直到工作階段結束。**Cursor Screen** 範圍只會凍結滑鼠游標所在的顯示器。

### 桌面整合
- **可配置的啟動行為**：啟動 Mark Shot 不再預設彈出截圖覆蓋層。在 設定 → 通用 → **啟動行為** 中可任意組合 **托盤圖示**、**懸浮球**、**設定視窗**、**直接截圖** 四種模式。全新安裝預設托盤圖示 + 懸浮球，直接截圖為可選項。
- **懸浮球**：置頂小圓形控制項（預設螢幕右下角），提供截圖 / 全屏截圖 / 錄製 / 設定的快捷入口。單擊彈出選單，雙擊直接截圖，拖動可移動（位置會被記住），截圖進行中自動隱藏。
- **桌面捷徑**：
  - `mark-shot.desktop`：設定為系統全域截圖工具，支援系統快捷鍵直接呼叫。
  - `mark-shot-edit.desktop`：註冊為獨立的影像編輯器，可整合到檔案管理員（如 Dolphin、Nautilus）的右鍵「開啟方式」選單中。
- 附帶高解析度的 `mark-shot.svg` 與 `mark-shot-edit.svg` 系統向量圖示。

### KDE KWin ScreenShot2 授權

在 KDE Wayland 中，Mark Shot 可以使用 KWin 的 `org.kde.KWin.ScreenShot2` 介面執行精確區域截圖。KWin 將該介面視為受限 D-Bus 介面，因此應用程式對應的桌面檔案必須宣告授權欄位。

<details>
<summary>KDE KWin ScreenShot2 授權與桌面檔案設定說明 (點擊展開)</summary>

宣告授權欄位：
```ini
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

發行版安裝套件和 `cmake --install` 會自動安裝所需的桌面檔案。如果直接執行本機建置產物而未安裝專案，請建立或更新 `~/.local/share/applications/mark-shot.desktop`：

```ini
[Desktop Entry]
Type=Application
Name=Mark Shot
Comment=Wayland screenshot selection and annotation tool
Exec=/absolute/path/to/mark-shot
Icon=mark-shot
Terminal=false
Categories=Graphics;Utility;
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

如果是透過 KDE 的命令快捷鍵服務繫結 Mark Shot，還需要建立 `~/.local/share/applications/net.local.mark-shot.desktop`：

```ini
[Desktop Entry]
Type=Application
Name=Mark Shot Shortcut Service
Exec=/absolute/path/to/mark-shot
Icon=mark-shot
Terminal=false
NoDisplay=true
StartupNotify=false
Categories=Utility;
X-KDE-GlobalAccel-CommandShortcut=true
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

修改桌面檔案後，建議登出並重新登入，讓 KDE 重新讀取桌面檔案快取。如果目前 KDE 工作階段仍回傳 `NoAuthorized`，請重新啟動 KWin 或重新啟動系統一次。
</details>


---

## 產品功能對比

Mark Shot 社群版是一款**開源（MIT）、跨平台（Linux X11/Wayland 原生 + Windows）、可完全離線**的截圖·標註·貼圖·OCR·翻譯·螢幕錄製一體化工具。下表按各產品官方文件與官網整理（截至 **2026 年 8 月**），如實標註能力有無：**✅ 內建支援**；**⭕ 部分支援或依賴外部工具/服務**；**❌ 不支援**。能力一律按「開箱即用」口徑統計，不與付費會員、雲端服務或實驗性分支掛鉤。

### 核心能力總覽

| 能力 | Mark Shot 社群版 | Flameshot | ShareX | PixPin | Snipaste | Spectacle | Snagit | CleanShot X | Shottr |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 區域截圖 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 全螢幕截圖 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 視窗截圖 | ⭕ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 捲動長截圖 | ✅ | ❌ | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ✅ |
| 標註工具集 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 序號 / 放大鏡標註 | ✅ | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ |
| 貼圖 Pin 到螢幕 | ✅ | ❌ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ |
| OCR 文字辨識 | ✅ | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ |
| 翻譯 | ✅ | ❌ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ |
| 二維碼 / 條碼辨識 | ✅ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ |
| 螢幕錄製 | ✅ | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ⭕ |
| 動畫 WebP 輸出 | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| 圖床上傳 | ✅ | ⭕ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ✅ | ⭕ |
| 無頭 CLI / 腳本化 | ✅ | ✅ | ✅ | ⭕ | ❌ | ✅ | ❌ | ⭕ | ⭕ |
| 懸浮球快捷入口 | ✅ | ❌ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Linux Wayland 原生 | ✅ | ⭕ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| 定時 / 延遲截圖 | ❌ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ |
| 開源 / 免費 | ✅ | ✅ | ✅ | ⭕ | ⭕ | ✅ | ❌ | ❌ | ✅ |

### 功能詳解

**截圖品質與覆蓋面**（決定「能不能截到想要的東西」）

- **區域 / 全螢幕 / 顯示器截圖** — Mark Shot 全支援，多顯示器下正確處理負座標與獨立縮放（HiDPI）；主流競品均已涵蓋此基礎能力。
- **視窗截圖** — Mark Shot 在無頭模式（`--window`）完整支援按 id / 標題 / 類別 / PID / 行程名稱選窗；X11 下即使視窗被遮擋或已最小化，也能從合成器保留的視窗緩衝直接讀取真實內容（不彈起、不搶焦點）。互動模式為全螢幕選區覆蓋層，未單獨提供「活動視窗」一鍵模式（故標 ⭕）。
- **捲動長截圖** — Mark Shot 在 niri 與 GNOME Wayland 原生可用（GNOME 需官方擴充套件），KDE/X11 為實驗特性。Windows 的 ShareX、Snagit 與 macOS 的 CleanShot X / Shottr 亦內建；開源陣營中 Flameshot、Spectacle、Snipaste 均不支援。
- **定時 / 延遲截圖** — Mark Shot 暫未提供（❌）；多數主流工具支援，屬後續可補齊項目。

**標註、辨識與隱私**（「截完之後能不能高效率地使用」）

- **標註工具集** — Mark Shot 內建 12+ 工具：畫筆、螢光筆、直線、矩形、橢圓、箭頭、文字、步驟序號、馬賽克、雙框放大鏡、雷射筆、取色器與尺子；與 ksnip、Spectacle、ShareX 處於同一水準。
- **貼圖 Pin** — Mark Shot 支援無邊框置頂貼圖、縮放、旋轉、OCR 取詞、LLM 原位翻譯；Snipaste、ShareX、CleanShot X、Shottr 亦內建。
- **OCR 文字辨識** — Mark Shot 內建 RapidOCR（PP-OCR 模型，離線）+ Tesseract 回退，開箱即用；Snipaste、Flameshot、Snagit 無（或需外掛），Spectacle 依賴外部 Tesseract。
- **翻譯** — Mark Shot 內建 OpenAI 相容介面的 LLM 截圖翻譯（可離線/自建服務）；**競品中無一內建通用截圖翻譯**（PixPin 為會員功能，微信/QQ 翻譯綁定其用戶端）。這是目前最獨特的差異化能力之一。
- **二維碼 / 條碼辨識** — Mark Shot 內建掃碼（QR / 一維碼 / PDF417 等），截圖階段按 `Q` 鍵直達；ShareX、Shottr 支援，多數工具沒有。
- **一鍵 AI 打碼** — 微信截圖 2026 年新增的自動打碼能力 Mark Shot 暫未內建（❌），屬可跟進方向；Mark Shot 現有馬賽克/毛玻璃為手動標註。

**分享、協作與自動化**（「截完能不能順手分發/沉澱」）

- **圖床上傳** — Mark Shot 內建 ImgURL / sm.ms / imgbb / litterbox / 自訂指令，`Ctrl+U` 一鍵上傳並複製連結；ShareX 上傳目標最齊全，Flameshot 僅 Imgur，Snipaste / PixPin 無圖床。
- **無頭 CLI / 腳本化** — Mark Shot 提供完整無頭鏈路：`--capture-to` 區域/顯示器截圖、`--list-windows` + `--window` 視窗/元件截圖、`--record-region/--record-display` 無人值守錄製、`--record-wait-json` 等待並輸出結構化狀態；全程無視窗、無彈窗、無焦點搶佔，且**可指定 PID / 行程名稱**並截取被遮擋/最小化視窗。商業版 MCP server 複用同一鏈路。競品中僅 ShareX（僅 Windows）與 Flameshot 有較強 CLI；PixPin 僅有 JS 腳本動作引擎。
- **懸浮球快捷入口** — Mark Shot 內建可拖曳、可吸附螢幕邊緣、閒置自動半透明的懸浮球；同類僅 PixPin（會員）提供。

**平台、授權與生態**

- **Linux Wayland 原生** — Mark Shot 原生支援 PipeWire portal、grim、layer-shell、KDE KWin ScreenShot2 與 GNOME 擴充套件；開源陣營中僅 Spectacle（KDE）達到同等原生水準，Flameshot 的 Wayland 為實驗性。PixPin / Snipaste（Pro）無 Linux 版。
- **開源 / 免費** — Mark Shot 社群版為 MIT 開源、完全免費、無廣告、無帳號、無需連網；Flameshot、ShareX、Spectacle、Shottr 同為免費開源/免費，PixPin / Snipaste 為閉源商業（會員/授權），Snagit / CleanShot X 為付費商業軟體。
- **外掛生態** — Mark Shot 提供 Qt 外掛體系與 GitHub 外掛市集（OCR / 翻譯 / 掃碼 provider 可擴充）；ShareX 有動作/上傳外掛，ksnip 有 OCR 外掛，Flameshot / Spectacle 無外掛體系。

> 說明：以上對比基於各產品官網與官方文件（2026-08）整理；功能口徑隨版本演進會變化，請以各產品最新文件為準。部分工具（FastStone Capture、Nimbus、搜狗截圖等）未列入總覽表，其能力詳見上文調查口徑。

---

## 命令列介面 (CLI)

### 常用使用範例

```bash
# 擷取螢幕並進入區域裁剪與標註模式
mark-shot

# 在多顯示器環境下擷取所有輸出螢幕
mark-shot --all-outputs

# 跳過選區步驟，直接對擷取的完整螢幕截圖進行標註
mark-shot --fullscreen

# 選區完成後預設使用移動工具，全螢幕標註預設使用雷射筆，並設定紅色預設顏色
mark-shot --default-tool move --fullscreen-default-tool laser --default-color '#FF4D4D'

# 開啟一個已有的本機圖片檔案並直接進入標註模式
mark-shot path/to/image.png

# 直接將本機圖片作為貼圖視窗開啟
mark-shot --pin-image path/to/image.png

# 強制使用標準的 XDG 全螢幕一般視窗執行（而非 Wayland layer-shell）
mark-shot --xdg-window
```

#### 無介面（非互動）截圖

腳本、CI 自動化或其它程式可呼叫 `mark-shot` 完成截圖而無需開啟標註介面。
擷取的幀會寫入 PNG，並向標準輸出列印一行精簡的 JSON 摘要：

```bash
# 擷取主螢幕並寫入 PNG
mark-shot --capture-to /tmp/shot.png

# 寫入目錄（自動產生帶時間戳記的檔名）
mark-shot --capture-to /tmp/shots/

# 擷取邏輯螢幕區域（x,y,寬度,高度）
mark-shot --capture-to /tmp/region.png --region 0,0,1280,720

# 依顯示器名稱擷取指定螢幕，並包含滑鼠游標
mark-shot --capture-to /tmp/window.png --display DP-1 --include-cursor

# 同時擷取多個顯示器（可重複 --display，每個顯示器一張 PNG）
mark-shot --capture-to /tmp/shots/ --display DP-1 --display DP-2

# 以 JSON 輸出目前所有顯示器資訊並退出
mark-shot --list-displays
```

單一顯示器 `--capture-to` 的 JSON 輸出範例：

```json
{"path":"/tmp/shot.png","width":2560,"height":1440,"output":"DP-1","error":null}
```

當指定多個 `--display` 時，輸出變為每個螢幕一個擷取的陣列：

```json
{"captures":[{"path":"/tmp/shots/mark-shot-DP-1-20260801-000000.png","width":2560,"height":1440,"output":"DP-1","error":null},
             {"path":"/tmp/shots/mark-shot-DP-2-20260801-000000.png","width":1920,"height":1080,"output":"DP-2","error":null}]}
```

每個選中的顯示器使用各自的來源幾何進行擷取，因此 portal 類後端會精確回傳
該顯示器而不是整個虛擬桌面。

無介面截圖重用與互動介面相同的全部擷取後端（QScreen、
xdg-desktop-portal、PipeWire、grim、KWin/GNOME 輔助、Windows Graphics Capture），
因此影像品質與區域裁剪行為完全一致。所有無介面參數與位置圖片檔案參數互斥。

### CLI 參數說明

| 參數選項 | 功能說明 |
| :--- | :--- |
| `[file]` | **位置參數**：開啟一個已有的本機圖片檔案進入標註模式，而不是擷取目前螢幕。 |
| `-h`, `--help` | 顯示說明資訊並退出。 |
| `-v`, `--version` | 顯示目前版本資訊並退出。 |
| `--all-outputs` | 擷取虛擬顯示桌面的所有輸出螢幕，而不是僅擷取目前的作用中螢幕。 |
| `--xdg-window` | 強制使用標準的 XDG 全螢幕一般視窗（xdg-shell）替代預設的 Wayland 覆蓋層（layer-shell）。 |
| `--fullscreen` | 跳過選區步驟，直接對擷取的完整螢幕截圖進行標註。 |
| `--default-tool <tool>` | 指定一般選區完成後的預設標註工具；未設定 `--fullscreen-default-tool` 時也作為全螢幕模式預設工具。 |
| `--fullscreen-default-tool <tool>` | 指定全螢幕標註模式的預設工具。 |
| `--default-color <color>` | 指定預設標註顏色。支援 `#RRGGBB` 與 `#RRGGBBAA`。 |
| `--tray` | 將 Mark Shot 保持執行在系統匣中，並在平台支援時註冊全域截圖快捷鍵。 |
| `--capture` | 當設定中啟用系統匣自動啟動時，強制觸發單次截圖。 |
| `--pin-image <path>` | 直接將本機圖片作為貼圖視窗開啟，跳過截圖與選區流程。 |
| `--recording-status` | 透過執行中的實例輸出目前錄製狀態 JSON。 |
| `--stop-recording` | 請求執行中的實例停止目前進行的錄製。 |
| `--record-region <x,y,w,h>` | 透過執行中的實例錄製螢幕區域；幾何格式為 x,y,寬度,高度。 |
| `--record-display <id>` | 依 ID 錄製顯示器（參見 `--list-displays` 輸出：原始螢幕名稱如 `DP-1`、`output:DP-1` 鍵或 `all`）。 |
| `--record-output <path>` | 錄製的輸出檔案路徑（使用錄製旗標時為必填）。 |
| `--record-duration <seconds>` | 錄製持續時間（秒）；0 表示錄製到 `--stop-recording` 為止。 |
| `--record-fps <n>` | 錄製的影格率（預設 15）。 |
| `--record-format <mp4\|gif\|webp>` | 錄製格式：mp4（預設）、gif 或 webp。 |
| `--record-audio` | 在錄製中包含系統音訊。 |
| `--record-wait-json` | 等待錄製完成，然後以 JSON 列印最終狀態。 |
| `--debug` | 為本次執行啟用除錯日誌。 |
| `--no-debug` | 為本次執行停用除錯日誌，並覆蓋設定檔與環境變數。 |
| `--debug-log <path>` | 將除錯日誌寫入指定路徑；除非同時設定 `--no-debug`，否則會啟用除錯日誌。 |
| `--capture-to <path>` | 無介面截圖：將 PNG 寫入指定檔案或目錄，不開啟介面；向標準輸出列印 JSON 摘要。 |
| `--region <x,y,w,h>` | 配合 `--capture-to` 使用：只擷取指定邏輯螢幕區域。 |
| `--display <name>` | 配合 `--capture-to` 使用：依顯示器名稱擷取指定輸出螢幕。可重複指定以一次擷取多個顯示器（每個螢幕一張 PNG）。 |
| `--include-cursor` | 配合 `--capture-to` 使用：將滑鼠游標繪製進擷取幀。 |
| `--output-name <name>` | 配合 `--capture-to` 使用：當擷取路徑為目錄時使用的基準檔案名稱（不含副檔名）。 |
| `--list-displays` | 以 JSON 輸出目前所有顯示器資訊並退出。 |

### 快捷鍵繫結

將 `mark-shot` 繫結為系統截圖快捷鍵：

**niri**（修改 `~/.config/niri/config.kdl`）：
```kdl
binds {
    Mod+Shift+S { spawn "mark-shot"; }
}
```

**Hyprland**（修改 `~/.config/hypr/hyprland.conf`）：
```ini
# 繫結 Super+Shift+S 啟動 mark-shot 選區截圖
bind = SUPER SHIFT, S, exec, mark-shot
# 繫結 Print 按鍵啟動 mark-shot 選區截圖
bind = , Print, exec, mark-shot
```

**Sway / i3**（修改 `~/.config/sway/config` 或 `~/.config/i3/config`）：
```ini
# 繫結 Super+Shift+S 啟動 mark-shot 選區截圖
bindsym Mod4+Shift+S exec mark-shot
# 繫結 Print 按鍵啟動 mark-shot 選區截圖
bindsym Print exec mark-shot
```

**GNOME**：在系統設定 → 鍵盤 → 鍵盤快捷鍵 → 自訂快捷鍵中新增。

**系統匣模式**：
```powershell
mark-shot --tray
```

系統匣模式預設註冊以下全域快捷鍵：
- `Ctrl+Alt+S`：啟動區域截圖。

系統匣選單還提供截圖、全螢幕截圖、開始錄製、錄製狀態、停止錄製、設定和退出等操作。


### 擴充指令

右側動作工具列提供 **Extensions** 按鈕，程式會從 `~/.config/mark-shot/extensions.json` 讀取使用者自訂指令。設定檔可以是 JSON 陣列，也可以是包含 `commands` 陣列的 JSON 物件。

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

`command` 在類 Unix 系統上透過 `$SHELL -c` 執行，在 Windows 上透過 `%COMSPEC% /C` 執行，因此支援 shell 運算式。使用 `{slurp}` 可把目前選區作為 `x,y widthxheight` 幾何字串傳入指令。使用 `{image}` 或 `{imagePath}` 可把目前已渲染選區作為臨時 PNG 路徑傳入指令，使用 `{imageUrl}` 可傳入 `file://` URL。這些佔位符會自動進行 shell 引用跳脫，設定中不要再額外加引號。若未使用圖片佔位符，可設定 `saveImage` 或 `needsImage` 為 `true`，程式會自動把臨時 PNG 路徑附加到指令結尾。`workingDirectory` 與 `cwd` 等價。`closeOnStart` 預設值為 `true`，指令啟動前會先隱藏並關閉 Mark Shot。

### 應用程式設定檔

參見[設定參考](../docs/configuration.zh-CN.md)。

### 使用者操作手冊

日常操作（視窗懸停框選、標註工具、啟動工具、貼圖視窗、長截圖、headless CLI
以及功能自測清單）請參見[使用者操作手冊](../docs/user-guide.zh-CN.md)
（[English](../docs/user-guide.md)）。

其他語言版本：
[简体中文](../docs/user-guide.zh-CN.md) · [繁體中文](../docs/user-guide.zh-TW.md) ·
[日本語](../docs/user-guide.ja.md) · [한국어](../docs/user-guide.ko.md) ·
[Русский](../docs/user-guide.ru.md) · [Italiano](../docs/user-guide.it.md) ·
[العربية](../docs/user-guide.ar.md) · [Français](../docs/user-guide.fr.md) ·
[Deutsch](../docs/user-guide.de.md) · [Español](../docs/user-guide.es.md) ·
[Português](../docs/user-guide.pt.md)

## 編譯與安裝

### 安裝指南

##### Arch Linux (AUR)
Arch Linux 使用者可以直接透過 AUR 輔助工具進行安裝：
```bash
# 從原始碼編譯安裝
paru -S mark-shot
# 或
yay -S mark-shot

# 安裝預編譯二進位套件
paru -S mark-shot-bin
# 或
yay -S mark-shot-bin
```

`mark-shot` 從原始碼編譯；`mark-shot-bin` 從 GitHub Releases 下載預編譯 pacman 套件安裝。

##### NixOS
NixOS 使用者可以透過新增 Flake input 來進行安裝
```nix
# flake.nix
mark-shot = {
  url = "github:tystudio-26020701/mark-shot-community";
  inputs.nixpkgs.follows = "nixpkgs";
};

# home-manager
home.packages = with pkgs; [
  # 其他使用者應用程式
  inputs.mark-shot.packages.${pkgs.stdenv.hostPlatform.system}.default
]
```

##### 其他發行版 (預編譯安裝套件)
對於其他發行版（如 Ubuntu, Debian, Fedora），請在 Releases 頁面下載編譯好的安裝套件並執行以下命令安裝：
- **Debian / Ubuntu**:
  ```bash
  sudo apt install ./mark-shot_<version>_amd64.deb
  ```
- **Fedora**:
  ```bash
  sudo dnf install ./mark-shot-<version>-1.x86_64.rpm
  ```

> **Ubuntu 26.04 LTS**：Mark Shot 已在 Ubuntu 26.04 LTS（Resolute）上驗證並支援。
> 在 Ubuntu 26.04 上從原始碼建置可直接使用發行版自帶的 Qt 6.10 軟體套件
> （無需 `aqtinstall` 步驟）：
>
> ```bash
> sudo apt install build-essential cmake ninja-build pkg-config \
>   qt6-base-dev qt6-wayland libpipewire-0.3-dev libxcb-cursor0 \
>   xdg-desktop-portal pipewire xclip
> cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
> cmake --build build
> ```
>
> 無頭截圖（`--capture-to`）、多顯示器截圖（可重複的 `--display`）以及本機
> MCP 服務均可在 Ubuntu 26.04 的 Wayland（GNOME）與 X11 工作階段下執行。

### 系統依賴

#### Wayland (Arch Linux)

```bash
sudo pacman -S --needed base-devel cmake ninja pkgconf qt6-base qt6-wayland layer-shell-qt pipewire grim wl-clipboard
```

#### X11/GNOME (Ubuntu/Debian)

```bash
# 建置工具
sudo apt install build-essential cmake ninja-build pkg-config libpipewire-0.3-dev

# Portal 與剪貼簿工具
sudo apt install xdg-desktop-portal pipewire xclip

# Qt 6（若系統套件庫無 Qt 6，可透過 aqtinstall 安裝到使用者目錄）
pip install aqtinstall
aqt install-qt linux desktop 6.7.3 gcc_64 --outputdir ~/Qt
```

> **說明**：在 Ubuntu 22.04 等系統自帶 Qt 5 的環境下，將 Qt 6 安裝到 `~/Qt` 不會影響系統。編譯時傳入 `-DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64` 即可。

#### fcitx5 中文輸入支援（X11 環境下的 Qt 6）

Qt 6 未自帶 fcitx5 輸入法外掛。若需在 X11 環境下使用 fcitx5 中文輸入，需從原始碼編譯該外掛：

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

#### OCR 後端（可選）

Mark Shot 的文字辨識功能依賴內建的 `mark-shot-ocr` Python 腳本。該腳本支援 **RapidOCR**（首選，基於 PaddleOCR PP-OCR 模型）和 **Tesseract**（回退）。Linux 上會自動安裝該腳本；Windows 上需要手動設定。

<details>
<summary><b>Linux</b></summary>

```bash
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime
```

安裝完成後 `mark-shot-ocr` 會被自動偵測，無需額外設定。

**環境變數**（可選）：

| 變數 | 說明 | 預設值 |
|------|------|--------|
| `MARK_SHOT_OCR_VERSION` | PaddleOCR 版本（`PP-OCRv5`、`PP-OCRv4` 等） | `PP-OCRv5` |
| `MARK_SHOT_OCR_MODEL_TYPE` | 模型大小：`mobile` 或 `server` | `mobile` |
| `MARK_SHOT_OCR_MODEL_DIR` | 自訂模型儲存目錄 | `~/.local/share/mark-shot/models` |
| `MARK_SHOT_OCR_NO_VENV` | 設為 `1` 停用自動切換虛擬環境 | — |
| `MARK_SHOT_OCR_PYTHON` | 指定用於 re-exec 的 Python 解譯器路徑 | `~/.local/share/mark-shot/ocr-venv/bin/python` |

</details>

<details>
<summary><b>Windows</b></summary>

內建的輔助腳本不會在 Windows 上自動安裝，需要手動完成以下步驟：

**1. 安裝 Python 3**

從 [python.org](https://www.python.org/downloads/) 下載安裝 Python 3.10 或更高版本。安裝時請勾選 **Add python.exe to PATH**。

**2. 複製 OCR 輔助腳本**

將 [Mark Shot 儲存庫](https://github.com/tystudio-26020701/mark-shot-community) 中的 `../scripts/mark-shot-ocr` 複製到本機目錄，例如 `%LOCALAPPDATA%\mark-shot\mark-shot-ocr.py`。

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\mark-shot"
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/tystudio-26020701/mark-shot-community/main/scripts/mark-shot-ocr" `
  -OutFile "$env:LOCALAPPDATA\mark-shot\mark-shot-ocr.py"
```

**3. 建立虛擬環境並安裝依賴**

```powershell
python -m venv "$env:LOCALAPPDATA\mark-shot\ocr-venv"
& "$env:LOCALAPPDATA\mark-shot\ocr-venv\Scripts\pip.exe" install -U pip rapidocr onnxruntime
```

> `onnxruntime` 提供 CPU 推論。如果有相容的 GPU，可以安裝 `onnxruntime-directml` 或 `onnxruntime-gpu` 以加速辨識。

**4. 在 `config.json` 中設定 `ocr.command`**

開啟 `%LOCALAPPDATA%\mark-shot\config.json`（不存在則新建），設定 `ocr.command`：

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

將 `%LOCALAPPDATA%` 替換為實際展開後的路徑（如 `C:\Users\你的使用者名稱\AppData\Local`）。`{image}` 佔位符在執行階段會被替換為臨時截圖路徑；如果省略，Mark Shot 會自動附加。

> **提示**：設定環境變數 `MARK_SHOT_OCR_NO_VENV=1` 可以跳過腳本內建的虛擬環境自動偵測，因為已經直接使用了虛擬環境中的 Python。

</details>

#### 掃碼後端（可選）

```bash
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

掃碼 helper 優先使用 `zxing-cpp`，支援 QR Code、Data Matrix、Aztec、PDF417、EAN、UPC、Code 39、Code 93、Code 128 等常見格式。如果安裝了 `pyzbar` 或 OpenCV，也會作為回退後端使用。

#### 圖床上傳後端（可選）

圖床上傳功能預設使用內建的 `mark-shot-upload` Python 腳本，無需額外安裝依賴（僅使用 Python 3 標準函式庫）。該腳本透過環境變數設定圖床參數，支援任意相容 multipart/form-data 上傳協定的圖床服務。

<details>
<summary>內建 helper 支援的環境變數</summary>

| 環境變數 | 說明 | 預設值 |
|---------|------|--------|
| `MARK_SHOT_UPLOAD_URL` | **必填**，圖床上傳介面 endpoint | — |
| `MARK_SHOT_UPLOAD_FIELD` | 檔案欄位名稱 | `image` |
| `MARK_SHOT_UPLOAD_API_KEY` | API Key / Token | — |
| `MARK_SHOT_UPLOAD_AUTH_HEADER` | 認證標頭名稱 | `Authorization` |
| `MARK_SHOT_UPLOAD_AUTH_SCHEME` | 認證方案（如 `Bearer`），留空則直接用 API Key | `Bearer` |
| `MARK_SHOT_UPLOAD_URL_PATH` | URL 在 JSON 回應中的點分隔路徑（如 `data.url`） | 自動探測 |
| `MARK_SHOT_UPLOAD_DELETE_URL_PATH` | 刪除 URL 路徑 | 自動探測 |
| `MARK_SHOT_UPLOAD_HEADER_xxx` | 自訂請求標頭（如 `MARK_SHOT_UPLOAD_HEADER_X-Custom=foo`） | — |
| `MARK_SHOT_UPLOAD_FIELD_xxx` | 額外表單欄位（如 `MARK_SHOT_UPLOAD_FIELD_album=123`） | — |

</details>

<details>
<summary>設定範例：ImgURL V3</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://www.imgurl.org/api/v3/upload",
    "MARK_SHOT_UPLOAD_FIELD": "file",
    "MARK_SHOT_UPLOAD_API_KEY": "sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

ImgURL V3 使用 `Authorization: Bearer <token>` 認證（`AUTH_SCHEME` 預設 `Bearer`，無需修改）。

</details>

<details>
<summary>設定範例：sm.ms</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://sm.ms/api/v2/upload",
    "MARK_SHOT_UPLOAD_FIELD": "smfile",
    "MARK_SHOT_UPLOAD_API_KEY": "你的Token",
    "MARK_SHOT_UPLOAD_AUTH_SCHEME": "",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

sm.ms 直接用 Token 作為 Authorization 值，因此 `AUTH_SCHEME` 設為空字串。

</details>

<details>
<summary>設定範例：imgbb</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://api.imgbb.com/1/upload?key=你的API_KEY",
    "MARK_SHOT_UPLOAD_FIELD": "image",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

imgbb 透過 URL 查詢參數傳遞 API Key，無需設定 `API_KEY`。

</details>

<details>
<summary>設定範例：litterbox（臨時圖床，無需 API Key）</summary>

```json
"upload": {
  "command": "curl -sf --max-time 30 -A 'Mozilla/5.0' -F reqtype=fileupload -F time=72h -F fileToUpload=@{image} https://litterbox.catbox.moe/resources/internals/api.php",
  "timeoutMs": 35000
}
```

litterbox 回應為純文字 URL（非 JSON），Mark Shot 會自動辨識 `http://`/`https://` 開頭的輸出作為上傳結果。

</details>

<details>
<summary>自訂上傳指令</summary>

若內建 helper 無法滿足需求，可透過 `upload.command` 接入任意自訂上傳腳本。指令需滿足：

1. **退出碼**：成功時退出碼為 0，非零視為失敗
2. **輸出格式**（二選一）：
   - **JSON**：`{"url":"https://...","deleteUrl":"https://...","errors":[]}`（`url` 必填，其他可選）
   - **純文字 URL**：stdout 第一行非空內容以 `http://` 或 `https://` 開頭
3. **佔位符**：支援 `{image}`、`{imagePath}`、`{imageUrl}`；若指令中未包含佔位符，Mark Shot 會自動在指令結尾附加臨時圖片路徑

```json
"upload": {
  "command": "/path/to/your-uploader.sh --file {image} --json",
  "timeoutMs": 30000,
  "env": {
    "UPLOADER_API_KEY": "xxx"
  }
}
```

`upload.env` 中的環境變數會同時傳遞給自訂指令，便於重複使用設定。

</details>

#### Windows

安裝與目前編譯器匹配的 Qt 6、CMake、Ninja，以及支援 C++17 的編譯器，例如 MSVC 或 MinGW。Windows 建置不需要 Qt DBus、PipeWire、X11/XCB、LayerShellQt、`grim`、`wl-copy` 或 `xclip`。

```powershell
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
cmake --build build-windows
```

目前 Windows 支援範圍是一般截圖與圖片標註。捲動截圖、合成器專用視窗偵測和 Linux 桌面捷徑在 Windows 上不可用。內建的 Python 輔助腳本（`mark-shot-ocr`、`mark-shot-code-scan`、`mark-shot-translate`）不會自動安裝，請參考上方的 [OCR 後端](#ocr-後端可選)、[掃碼後端](#掃碼後端可選)和翻譯章節進行手動設定。

### 建置與編譯

```bash
# 使用系統 Qt 6
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 如果 Qt 6 安裝在使用者目錄，額外指定 CMAKE_PREFIX_PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64

# 執行編譯
cmake --build build
```

或者使用 nix

```bash
nix build
```

LayerShellQt 會被自動偵測。找到時啟用完整 Wayland layer-shell 支援；未找到時編譯照常成功，執行階段自動降級為標準全螢幕視窗。

### 安裝與整合

```bash
cmake --install build --prefix "$HOME/.local"
```

此命令會安裝可執行檔、輔助腳本（`mark-shot-ocr`、`mark-shot-code-scan`、`mark-shot-translate`、`mark-shot-upload`）、桌面捷徑和圖示。

### GNOME Wayland 捲動截圖擴充套件

GNOME Wayland 的捲動截圖必須啟用 **Mark Shot Scroll Helper** 擴充套件。沒有該擴充套件時，Mark Shot 無法靜默連續截取選定區域，也無法繪製 GNOME 原生捲動預覽面板，因此會在 GNOME Wayland 上停用捲動截圖按鈕。

擴充套件檔案位於專案儲存庫的 `../packaging/gnome-extension/mark-shot-scroll-helper@snemc.org` 路徑。

<details>
<summary><b>展開/收合 GNOME Wayland 捲動截圖擴充套件安裝與啟用指南</b></summary>

##### 方式 A：透過發行版套件安裝
如果您是透過發行版套件（如 `.deb` 或 `.rpm`）安裝的 Mark Shot，該擴充套件已隨系統預設安裝。可執行以下命令為目前使用者啟用該擴充套件：
```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```
*如果提示找不到該擴充套件，請登出並重新登入系統後再次嘗試。*

##### 方式 B：從儲存庫原始碼目錄安裝
如果您是從原始碼或本機手動建置的，需要先將該擴充套件複製到使用者的 GNOME 擴充套件路徑下：
```bash
# 定義擴充套件的 UUID
UUID=mark-shot-scroll-helper@snemc.org

# 建立使用者層級擴充套件目錄
mkdir -p "$HOME/.local/share/gnome-shell/extensions"

# 從專案儲存庫中複製擴充套件檔案
cp -r "packaging/gnome-extension/$UUID" "$HOME/.local/share/gnome-shell/extensions/"

# 啟用該擴充套件（您可能需要重新啟動 GNOME Shell 或登出並重新登入系統使該擴充套件生效）
gnome-extensions enable "$UUID"
```

驗證 helper D-Bus 介面是否可用：

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
```

預期結果為 `('4.2',)`。啟用擴充套件後，請重新啟動 `mark-shot`。

</details>

---

## 互動快捷鍵與手勢指南

### 工具切換快捷鍵

| 快捷鍵 | 切換的目標工具 | 對應功能說明 |
| :---: | :--- | :--- |
| **V** | 移動 / 導覽 (Move / Pan) | 在已有影像模式下，用於平移和拖曳影像畫布。 |
| **S** | 選擇 (Select) | 選取並移動、縮放或刪除已繪製的向量標註。 |
| **P** | 畫筆 (Pen) | 自由曲線繪製。 |
| **L** | 直線 (Line) | 繪製筆直的向量線條。 |
| **H** | 螢光筆 (Highlighter) | 半透明的高亮覆蓋，適合標記重點。 |
| **R** | 矩形 (Rectangle) | 繪製矩形線框。 |
| **E** | 橢圓 (Ellipse) | 繪製橢圓形線框。 |
| **A** | 箭頭 (Arrow) | 繪製經典的六頂點尖細長銳角箭頭。 |
| **T** | 文字 (Text) | 輸入並編排富文字，支援 1000px 字級及拖曳聯動。 |
| **N** | 序號 (Number) | 自動遞增步驟序號標貼。 |
| **M** | 馬賽克 (Mosaic) | 進行毛玻璃敏感區域虛化。 |
| **G** | 雷射筆 (Laser) | 教學或簡報使用的暫時筆跡，會自動平滑消融。 |

### 啟動介面輔助工具

| 快捷鍵 | 工具 | 功能說明 |
| :---: | :--- | :--- |
| **C** | 取色器 (Color Picker) | 在選擇截圖區域之前取樣截圖像素。滾動滑鼠滾輪可調整放大鏡大小，左鍵點擊會開啟顏色面板，可複製 HEX、RGB、HSL、HSV 和 Qt 等格式。右鍵或 Esc 返回一般選區。 |
| **R** | 尺子 (Ruler) | 在選擇截圖區域之前測量座標。懸停顯示目前像素，左鍵拖曳繪製帶像素刻度的測量矩形，並顯示寬度、高度、對角線和面積。右鍵或 Esc 返回一般選區。 |
| **Q** | 掃碼 (Code Scanner) | 進入 QR Code 與條碼掃碼模式。框選區域後會辨識其中的碼內容，並在可複製視窗中展示結果。右鍵或 Esc 返回一般選區。 |
| **D** | 顯示器擷取 (Display Capture) | 立即擷取全部輸出螢幕，按顯示器裁切並顯示縮圖；將滑鼠懸停到縮圖上可複製、編輯或儲存。 |

### 全域操作快捷鍵

| 快捷鍵 | 觸發動作 |
| :---: | :--- |
| **Esc** | 立即退出並關閉標註視窗。 |
| **Ctrl + C** | 確認所有文字編輯，並將目前截圖/已標註選區複製到系統剪貼簿。 |
| **Ctrl + S** 或 **Enter / Return** | 確認所有文字編輯，並儲存目前截圖。 |
| **Ctrl + P** | 將目前選區固定為懸浮貼圖視窗。 |
| **Ctrl + U** | 將目前截圖上傳到自訂圖床，上傳成功後 URL 自動複製到剪貼簿。 |
| **Ctrl + Z** | 復原上一步標註操作。 |
| **Ctrl + Y** 或 **Ctrl + Shift + Z** | 重做已被復原的標註操作。 |
| **Backspace** 或 **Delete** | 在 **選擇 (Select)** 工具啟動且選取了某標註時，刪除被選取的標註。 |
| **F** | 切換目前截圖覆蓋範圍（選區模式與全螢幕模式切換）。 |

### 進階互動操作技巧

- **繪製圖形約束**：在繪製 **矩形（Rectangle）** 或 **橢圓（Ellipse）** 時，按住 `Ctrl` 鍵可強制約束為正方形或正圓形。
- **快速切換至選擇工具**：在標註過程中，在畫布空白處點擊滑鼠右鍵可立即切換到 **選擇（Select）** 工具。
- **雙擊右鍵快速切換顏色**：在畫布空白處雙擊滑鼠右鍵，可開啟環形調色盤，快速切換目前標註工具的顏色。
- **滾輪無段調整**：在對應標註工具啟動狀態下，滾動滑鼠滾輪可即時調整目前工具的線寬、字級大小、序號標貼尺寸或馬賽克格網尺寸。
- **畫布平移與縮放**：在 **選擇（Select）** 工具模式下，或在編輯本機檔案時，滾動滑鼠滾輪可進行畫布無縫縮放，按住滑鼠中鍵拖曳可平移畫布。雙擊 `Ctrl` 鍵重設縮放與平移。

### 貼圖視窗專屬互動

| 互動手勢 / 快捷鍵 | 動作效果 |
| :--- | :--- |
| **滑鼠左鍵按住並拖曳** | 自由平移和放置桌面貼圖位置。 |
| **滑鼠滾輪向上/向下** | 貼圖視窗等比無段放大/縮小。 |
| **雙擊滑鼠左鍵** | 極速關閉該貼圖視窗。 |
| **滑鼠右鍵點擊** | 彈出功能選單（包括旋轉、複製圖片文字、翻譯、儲存、複製、關閉等）。 |
| **Esc 鍵** | 關閉目前取得焦點的貼圖視窗。 |

---

## 發版說明

參見[發版說明](../docs/releases.zh-CN.md)。

## 意見回饋與交流

### 提交 Issue
若您在執行中遇到問題或有新功能建議，我們推薦使用 GitHub CLI (`gh`) 命令列工具提交 Issue。我們提供了一鍵收集環境資訊並自動產生的腳本，詳情請參閱 [Issue 提交指南](../.doc/submit-issue-via-gh.md)。

---

## 授權許可說明

本專案基於 **MIT 授權許可** 開源，詳情請參閱 [LICENSE](../LICENSE) 檔案。

## 致謝

Mark Shot 站在開源社群眾人的肩膀上，我們在此致以誠摯的謝意：

- **原上游專案 [jswysnemc/mark-shot](https://github.com/jswysnemc/mark-shot) 及其作者與全部貢獻者。** 本社群版基於原上游專案開發，其卓越的設計與持續的貢獻是這一切的基礎，我們由衷感謝他們的出色工作。
- **[serendipitywgy](https://github.com/serendipitywgy)**：感謝透過 `serendipitywgy/mark-shot` 貢獻跨桌面相容性改進、OCR 複製工具列動作和智慧矩形框預選功能。
- **Mark Shot 所依賴的全部開源專案**，包括 Qt 6、PipeWire、xdg-desktop-portal、layer-shell-qt、wl-clipboard、xclip、grim、RapidOCR、onnxruntime、Tesseract、ZXing-C++ 等。

本社群版由 [北京太殷造物科技有限公司](https://github.com/tystudio-26020701/mark-shot-community) 及貢獻者維護，基於 **MIT 授權許可** 開源。
