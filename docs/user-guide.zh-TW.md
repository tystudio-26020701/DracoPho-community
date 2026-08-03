# Mark Shot 使用指南

本手冊涵蓋 Mark Shot 的日常操作，重點在於**視窗／元件懸停選取**功能（移動滑鼠時會自動追蹤並反白游標下方的視窗；按一下即可選取它）、註記工作流程、無頭擷取以及設定。

> 本儲存庫中的文件是在社群分支中撰寫，並鏡像到上游與企業版儲存庫。企業版會為其本機 MCP 伺服器額外增加一節內容。

---

## 1. 快速開始

### 1.1 啟動

開始一次區域擷取工作階段：

```bash
mark-shot
```

按下桌面快速鍵（見第 8 節）或從終端機執行。凍結的全螢幕疊加層會在聚焦的顯示器上開啟。移動滑鼠繪製選取矩形，然後放開按鍵進入註記編輯器。

### 1.2 可攜式建置

如果您使用可攜式套件（`mark-shot-upstream`、`mark-shot-community`、`mark-shot-enterprise`），請使用隨附的啟動器啟動，這樣才能找到隨附的 Qt 函式庫、外掛程式與輔助腳本：

```bash
portable/mark-shot-community/bin/run-mark-shot.sh
```

啟動器會將其 `bin/` 目錄加入 `PATH` 前端，這是視窗偵測輔助腳本（`mark-shot-window-detection-*`）以及 OCR／上傳輔助程式所必需的。

---

## 2. 視窗／元件懸停選取

Mark Shot 可以在您選取區域之前偵測目前桌面的視窗。當選取疊加層開啟時，**移動滑鼠會以青色框反白游標下方的視窗**。**直接左鍵按一下（不拖曳）會選取整個視窗**作為擷取區域；接著您可以直接進行註記、複製、釘選或儲存。

反白的視窗來自於在疊加層出現之前執行的、依合成器而定的偵測腳本：

| 桌面 | 偵測來源 | 備註 |
| :--- | :--- | :--- |
| GNOME Wayland | 透過 D-Bus 的隨附 `mark-shot-scroll-helper@snemc.org` Shell 擴充功能 | 需要啟用該擴充功能（見第 2.1 節） |
| KDE Plasma Wayland | 透過 `qdbus6`／`qdbus` + journalctl 的一次性 KWin 腳本 | 需要 KWin 工作階段 |
| Hyprland | `hyprctl -j clients` | |
| niri | `niri msg -j windows` + 設定檔解析 | |
| X11 | 程序內 XCB 列舉 `_NET_CLIENT_LIST_STACKING` | 不需要腳本 |
| Windows | 程序內 `EnumWindows` | 不需要腳本 |

只會追蹤**頂層視窗**。視窗內的個別小元件（「component」）不會被 Wayland 合成器公開，因此在所有平台上懸停選取都以整個視窗為目標。

### 2.1 GNOME Wayland：啟用輔助擴充功能

```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```

確認 D-Bus 輔助程式會回應：

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
# -> ('5',)
```

如果呼叫失敗，請登出後重新登入（或在 X11 上重新啟動 GNOME Shell）再重試。沒有該擴充功能時，GNOME 輔助腳本會以錯誤結束，懸停選取會保持關閉（正常的拖曳選取仍然有效）。

### 2.2 如何使用

1. 觸發一次擷取（`mark-shot` 或桌面快速鍵）。
2. 在未按下任何滑鼠按鍵的狀態下，將游標移到某個視窗上。一個青色框會框出將被選取的視窗。
3. **按一下**（按下並放開，且移動不超過幾像素）即可選取該視窗。如果視窗重疊，游標處最上方的視窗獲勝（考量 z 軸順序）。
4. 放開後會進入註記編輯器，視窗正好被框住。
5. 若要改用**手動**區域，照常拖曳矩形即可——一旦拖曳超過按一下的門檻值，懸停框就會被忽略。

當色彩選擇器（`C`）或標尺（`R`）啟動工具啟用時，懸停反白會被停用，並且在條碼掃描（`Q`）、顯示器擷取（`D`）以及 GIF／影片錄製啟動模式中仍可使用。

### 2.3 在正確的顯示器上選取視窗

視窗偵測會針對每個擷取目標執行。在多顯示器環境中，每個凍結的視窗只會收到與其自身幾何區域相交的視窗，因此懸停框會與您在該顯示器上看到的內容一致。

### 2.4 啟用／停用

此功能預設為啟用（`windowDetection.enabled = true`）。您可以在**設定 → 進階 → 啟用視窗偵測**中切換，或編輯 `~/.config/mark-shot/config.json`：

```json
{
  "windowDetection": {
    "enabled": true,
    "command": "mark-shot-window-detection-gnome",
    "timeoutMs": 1000,
    "env": {}
  }
}
```

- `command`：偵測腳本。在 GNOME／KDE／Hyprland／niri Wayland 上，會自動選擇符合您工作階段的隨附 `mark-shot-window-detection-*` 腳本；在 X11 與 Windows 上則是在程序內列舉平台，`command` 可以留空。**使用者提供的自訂命令（例如絕對路徑）一律會被採用。**
- `timeoutMs`：等待腳本的最長時間（100–30000 毫秒，預設 1000）。
- `env`：傳遞給腳本的額外環境變數。依合成器而定的調整（偏移量）記載於腳本標頭中。

### 2.5 疑難排解

| 症狀 | 檢查 |
| :--- | :--- |
| GNOME Wayland 上沒有青色框 | 擴充功能有啟用嗎？上述 `gdbus` 呼叫必須回傳版本 |
| X11／Windows 上沒有青色框 | 不需要——平台列舉內建其中；請確認擷取工作階段未使用啟動指標工具 |
| 懸停框選到錯誤的（下層）視窗 | 自訂偵測腳本缺少 z 軸順序資料；沒有 `zOrder` 的視窗會被排在底層 |
| 擷取開始很慢 | 偵測腳本在疊加層之前執行；只有在桌面很慢時才調高 `timeoutMs`，或設定 `enabled:false` 加以略過 |
| 查看診斷資訊 | 執行 `mark-shot --debug --debug-log /tmp/mark-shot.log`；尋找 `window-detection` 行 |

---

## 3. 區域選取與啟動工具

在提交區域之前，您可以使用啟動疊加層工具：

| 快速鍵 | 工具 | 行為 |
| :---: | :--- | :--- |
| `C` | 色彩選擇器 | 取樣像素；滾輪調整放大鏡大小；左鍵按一下開啟色彩面板（HEX／RGB／HSL／HSV／Qt 格式）；右鍵按一下或 `Esc` 結束 |
| `R` | 標尺 | 懸停讀取像素座標；左鍵拖曳測量一個矩形，顯示寬度、高度、對角線與面積；右鍵按一下或 `Esc` 結束 |
| `Q` | 條碼掃描 | 在 QR 碼／條碼周圍拖曳一個區域；解碼結果會開啟在可複製的視窗中 |
| `D` | 顯示器擷取 | 擷取所有輸出，依顯示器裁切，顯示可懸停的縮圖（複製／編輯／儲存） |
| `S` | 停止進行中的 GIF／影片錄製 | 停止疊加層中顯示的錄製 |

`Esc` 會取消工作階段；右鍵按一下（未啟用啟動工具時）同樣會取消。

#### 3.1 多顯示器凍結行為

使用預設的 **Freeze All Screens** 擷取範圍時，在選取區域期間所有已連接的顯示器都會被凍結。一旦您在某台顯示器上提交選取，其他顯示器會繼續將凍結的畫面顯示為非互動式背景：滑鼠、鍵盤、滾輪與快速鍵輸入都會被吞掉，疊加層也不會顯示工具列，因此在擷取工作階段結束前，虛擬桌面的其餘部分都會保持凍結。若您改用 **Cursor Screen** 範圍（設定 → 擷取 → Freeze Scope），則只有游標下方的顯示器會被凍結，其他螢幕仍可完全正常使用。

---

## 4. 註記工具

選取區域後（或開啟本機影像後），編輯器會隨註記工具列一同開啟。工具可透過數字鍵或工具列切換：

| 快速鍵 | 工具 | 說明 |
| :---: | :--- | :--- |
| `V` | 移動／平移 | 移動整個選取範圍，平移本機影像畫布 |
| `S` | 選取 | 選取、移動、縮放、旋轉、刪除既有的註記 |
| `P` | 畫筆 | 流暢的手繪筆觸 |
| `L` | 直線 | 直線 |
| `H` | 螢光筆 | 半透明標記；手繪或直線樣式 |
| `R` | 矩形 | 具有「描邊」／「反白」／「反相」樣式、圓角的方框 |
| `E` | 橢圓 | 橢圓／圓形 |
| `A` | 箭頭 | 經典箭頭（羽狀、KDE、雙向） |
| `T` | 文字 | 富文字；滾輪或滑桿調整大小；對角控點等比縮放，側邊控點調整換行；字型面板提供精確的 pt 大小、字型家族、粗體／斜體 |
| `N` | 編號 | 依序編號的標記（阿拉伯數字、字母、羅馬數字、中文……） |
| `M` | 馬賽克 | 壓克力霧化模糊，用以隱藏敏感內容 |
| `G` | 雷射 | 會自動消散的暫時筆觸 |

繪圖提示：

- 繪製矩形／橢圓時按住 `Ctrl` 可強制為正方形／正圓形。
- 工具啟用時滾動滾輪可調整筆觸寬度、文字大小、編號比例或馬賽克區塊大小（即時預覽）。
- 在「選取」工具下，滾動可縮放畫布，按住中鍵可平移；連點兩下 `Ctrl` 可重設。

### 4.1 編輯既有的註記

切換到**選取**（`S`）。按一下註記即可顯示其控點：

- 在內部拖曳可移動；
- 拖曳角落／邊緣控點可調整大小；
- 拖曳頂緣上方的圓形控點可旋轉；
- 按下 `Delete`／`Backspace` 可移除；
- 連點兩下文字即可就地編輯。

屬性面板（右側）可編輯選取的註記：顏色、寬度、樣式、文字字型家族／大小／粗體／斜體。在「選取」工具下拖曳一個選取框可以選取多個註記；之後整個群組可以一起移動、調整大小、旋轉與刪除。

### 4.2 動作

| 快捷鍵 | 動作 |
| :--- | :--- |
| `Ctrl+C` | 複製到剪貼簿 |
| `Ctrl+S`／`Enter` | 儲存（依設定中的路徑範本） |
| `Ctrl+P` | 釘選為浮動貼紙視窗 |
| `Ctrl+U` | 上傳到設定的圖片主機；會複製 URL |
| `Ctrl+Z`／`Ctrl+Y` | 復原／重做 |
| `F` | 切換擷取範圍（選取範圍 ↔ 全螢幕） |

### 4.3 匯出外框

啟用**設定 → 匯出 → Mac 風格外框**，即可為儲存／複製／上傳的圖片加入透明留白、圓角與柔和陰影。

---

## 5. 釘選視窗貼紙

| 手勢／快捷鍵 | 行為 |
| :--- | :--- |
| 左鍵拖曳 | 重新定位貼紙 |
| 滾輪 | 等比縮放 |
| 連點兩下左鍵／`Esc` | 關閉 |
| 右鍵按一下 | 內容選單（旋轉、縮放、保持置頂、複製文字、翻譯、儲存、複製、關閉） |

釘選視窗內的 OCR 文字可以選取並複製（`Ctrl+C`／內容選單）。翻譯（OpenAI 相容端點）會將翻譯後文字以原始版面位置渲染回影像上。

---

## 6. 捲動截圖

1. 選取一個區域（或對非常大的區域使用浮動拖曳控點）。
2. 疊加層會捲動目標視窗；擷取的畫面會拼接成一張長圖。
3. GNOME Wayland 需要 Mark Shot Scroll Helper 擴充功能（第 2.1 節）。

捲動擷取在 niri 及類似的 wlroots/Wayland 合成器上已可正式使用；在 KDE、X11 等其他堆疊上則屬於測試功能。如果失敗，請使用一般截圖或自訂擴充功能命令。

---

## 7. 無頭擷取（CLI）

非互動式擷取會寫入 PNG 並輸出 JSON：

```bash
# primary screen
mark-shot --capture-to /tmp/shot.png

# directory (timestamped file name)
mark-shot --capture-to /tmp/shots/

# region
mark-shot --capture-to /tmp/r.png --region 0,0,1280,720

# a specific display, with cursor
mark-shot --capture-to /tmp/w.png --display DP-1 --include-cursor

# several displays at once (one PNG each)
mark-shot --capture-to /tmp/shots/ --display DP-1 --display DP-2

# list outputs
mark-shot --list-displays
```

所有無頭選項都與位置參數的影像檔互斥。完整的參數表請參閱 README。

### 7.1 無頭視窗／元件擷取

Mark Shot 可以**不開啟任何使用者介面**，就從腳本、建置管線或代理程式擷取**特定視窗——或視窗內的元件（子區域）**。程序只要寫入或回傳影像後就會結束，而且絕不建立視窗、絕不跳出對話方塊、絕不搶佔焦點，因此工具擷取桌面時使用者可以繼續工作。

首先列出視窗以查看有哪些可用：

```bash
mark-shot --list-windows
```

範例輸出（GNOME Wayland）：

```json
{"count":2,"platform":"wayland","source":"compositor-script","windows":[
  {"index":0,"id":"0x3c00007","title":"Mark Shot - VSCodium","class":"codium","instance":"codium","x":1920,"y":0,"width":1680,"height":1050,"zOrder":1},
  {"index":1,"title":"Terminal","class":"org.gnome.Terminal","x":67,"y":32,"width":800,"height":600}
]}
```

每個條目都帶有選擇器會比對的欄位：`index`、`id`（X11 視窗 id／後端提供的 id）、`title`、`class` 與 `instance`，以及 `x`／`y`／`width`／`height` 與選填的 `zOrder`。

#### 7.1.1 選取視窗（單一或多個）

`--window` 可以重複使用，以便在**一次呼叫中擷取任意數量的視窗**。每個選擇器都會自動解讀（`--window-by auto`）：

| 選擇器值               | 比對項目                                         |
| :---                      | :---                                                |
| `0`, `1`, …               | 清單 `index`                                        |
| `0x3c00007`               | 視窗 `id`                                         |
| `VSCodium`                | `class` 或 `instance`，然後 `title`（先精確，再子字串） |
| `Mark Shot - VSCodium`    | `title`                                             |

使用 `--window-by id|title|class|index` 強制套用單一比對規則。比對到多個視窗的選擇器會擷取**全部**這些視窗。

在選擇器後加上 `@x,y,width,height` 即可擷取元件（視窗內的子區域）——偏移量是相對於視窗左上角，並會被限制在視窗範圍內：

```bash
# the top 100px strip of window 0
mark-shot --window "0@0,0,1680,100" --capture-destination file --capture-to /tmp/shots/
```

#### 7.1.2 選擇影像的去處

`--capture-destination` 決定輸出方式；它可以搭配任意數量的 `--window` 選擇器以及一個元件子區域使用：

| 目標 | 行為 |
| :--- | :--- |
| `inline`（預設） | 以 JSON 輸出內嵌 Base64 PNG。**不會寫入任何檔案，也絕不碰觸剪貼簿。** 對於只需要像素的代理程式而言是最安全的選擇。 |
| `file` | 將 PNG 檔案寫入 `--capture-to <directory>`；需要該選項。 |
| `stage` | 將 PNG 檔案寫入暫存目錄（`$TMPDIR/mark-shot-staging`）。適合「先留著稍後使用」的工作流程。 |
| `clipboard` | 將影像複製到系統剪貼簿；有多張影像時以**最後一張**為準。內容在 CLI 結束後仍然保留（會產生一個持續存在的 `wl-copy`／`xclip` 持有者）。 |

範例：

```bash
# several windows, saved to a directory (one PNG per window)
mark-shot --window VSCodium --window Terminal --capture-destination file --capture-to /tmp/shots/

# a window plus a component of another window, staged for later
mark-shot --window "VSCodium@0,0,400,300" --window 1 --capture-destination stage

# multi-select, returned as base64 without touching files or clipboard
mark-shot --window 0 --window "Terminal" --capture-destination inline

# copy a window to the clipboard
mark-shot --window 0 --capture-destination clipboard
```

**剪貼簿政策。** 互動式編輯器刻意將您的選取範圍放到系統剪貼簿上（「複製」動作／`Ctrl+C`），因為這是截圖工具的主要工作流程。無頭模式（CLI 與企業版 MCP 伺服器）則遵循相反的規則：**除非明確選擇 `clipboard` 作為目標，且在「設定 → 儲存 → 無頭模式」中啟用了剪貼簿寫入，否則絕不修改剪貼簿**——`inline`（預設）與 `stage` 會保留使用者目前的剪貼簿內容，因此排程或代理程式驅動的擷取不會覆寫使用者正在其他地方使用的文字或影像。當 `clipboard` 請求因無頭剪貼簿寫入被停用而被拒絕時，擷取會退回設定的無頭預設目標，JSON 輸出（`"warning"`）與 stderr 會告知您，程序並以非零代碼結束，好讓自動化可以偵測。在設定中啟用無頭剪貼簿寫入需要輸入確認通行片語。

輸出是一個 JSON 物件 `{"captures":[...]}`，每個已擷取的視窗對應一個條目；每個條目都會重複選擇器、視窗身分與最終擷取矩形，並帶有 `path`（file/stage）或 `data`（inline），或兩者皆無（clipboard）。只有當每個選擇器都比對到且每次擷取都成功時，結束代碼才是 `0`；比對不到或擷取失敗會以 `"error"` 欄位及結束代碼 `1` 結束，而不是默默成功。

相同的擷取管線可以程式化地產生已註記的輸出——請參閱企業版的 MCP 伺服器章節，或將儲存的 PNG 與互動式編輯器搭配使用。

#### 7.1.3 無視窗干擾保證

每一種無頭模式都保證不可見且不具破壞性：

- **絕不建立任何視窗**——包括註記編輯器、擷取疊加層與系統匣；擷取會重用無頭擷取路徑；
- **絕不顯示任何對話方塊**——包括錯誤對話方塊：錯誤會送往 stderr；即使是格式錯誤的命令列（例如 `--window-by` 沒有搭配 `--window`、未知的 `--capture-destination`，或額外的位置參數檔案）也會立即以非零代碼與 stderr 訊息結束，而不是跳出 `QMessageBox` 或落到互動式使用者介面；
- 不會出現互動式 portal 提示（`allowInteractivePortal` 已停用）；
- 寫入輸出後程序立即結束；
- 無頭操作前後擷取的視窗清單完全一致；
- 除非明確要求 `clipboard`**且**在「設定 → 儲存 → 無頭模式」中啟用了剪貼簿寫入，否則無頭模式絕不碰觸系統剪貼簿。

如果沒有偵測到任何視窗（例如合成器輔助程式被停用，或 X11 工作階段沒有視窗列舉），命令會在 stderr 上輸出清楚的錯誤訊息並以代碼 `1` 結束，而不是默默擷取空白內容。

---

## 8. 桌面快速鍵與系統匣

系統匣模式（`mark-shot --tray`）會註冊 `Ctrl+Alt+S` 用於區域擷取，並提供擷取／錄製／設定／結束的選單項目。桌面快速鍵：

- **GNOME**：設定 → 鍵盤 → 快捷鍵 → 自訂快捷鍵 → 綁定到 `mark-shot`。
- **KDE**：綁定到 `mark-shot` 的自訂快捷鍵（加上用於精確 KDE 擷取的 KWin ScreenShot2 權限，請參閱 README）。
- **Hyprland**：`bind = SUPER SHIFT, S, exec, mark-shot` 與 `bind = , Print, exec, mark-shot`。
- **niri**：`binds { Mod+Shift+S { spawn "mark-shot"; } }`。
- **Sway / i3**：`bindsym Mod4+Shift+S exec mark-shot`。

---

## 9. 設定與後端

- 設定檔：`~/.config/mark-shot/config.json`（Linux），於首次執行時建立。
- 完整參考：[Configuration](configuration.md)。
- 後端：Wayland（PipeWire portal／grim／wlroots screencopy）、X11（`QScreen::grabWindow`）、Windows（原生 WGC）。錄製偏好 PipeWire portal，並會自動退回。
- 設定視窗會以確定性的方式追蹤未儲存的變更：每個控制項（下拉式選單、開關、微調方塊、文字欄位、快速鍵欄位、色彩選擇器）都會立即更新未儲存變更指示器，包括從下拉式清單快顯與強制回應色彩對話方塊中選取的值。還原變更會清除指示器，因此視窗只會在關閉時詢問真正待處理的編輯。

選用輔助程式：

```bash
# OCR (RapidOCR / Tesseract)
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime

# Code scan (zxing-cpp)
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

---

## 10. 功能測試檢查清單

使用此清單端對端驗證一個建置：

1. **啟動**——`run-mark-shot.sh` 開啟凍結的疊加層。
2. **視窗懸停**——將滑鼠移到視窗上：青色框跟隨移動；按一下選取視窗；重疊視窗會選取最上方的那個。
3. **手動區域**——拖曳一個矩形；放開；編輯器開啟。
4. **註記**——用每種工具繪製（畫筆、直線、矩形、橢圓、箭頭、螢光筆、文字、編號、馬賽克、放大鏡、雷射）；復原／重做；用「選取」移動／調整大小／旋轉／刪除；連點兩下文字進行編輯。
5. **複製／儲存／釘選／上傳**——`Ctrl+C`、`Ctrl+S`、`Ctrl+P`、`Ctrl+U`。
6. **啟動工具**——`C` 色彩選擇器、`R` 標尺、`Q` 條碼掃描、`D` 顯示器擷取。
7. **無頭**——`--capture-to`、`--region`、`--display`、`--list-displays`。
8. **無頭視窗擷取**——`--list-windows` 列出桌面；重複 `--window` 擷取多個視窗；在全部四種模式（inline、file、stage、clipboard）下測試 `--capture-destination`；驗證元件選擇器（`--window "0@0,0,400,300"`）；確認前後的視窗清單不變（無視窗干擾）。
9. **系統匣＋快速鍵**——`mark-shot --tray`，按下 `Ctrl+Alt+S`。
10. **可攜式細節**——套件能找到自己的 Qt 函式庫／外掛程式／腳本。

---

## 11. 意見回饋

請使用隨附的 [issue submission guide](../.doc/submit-issue-via-gh.md)，以 `gh issue create` 回報問題。並附上用 `mark-shot --debug --debug-log /tmp/mark-shot.log` 擷取的偵錯日誌。
