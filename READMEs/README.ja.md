<div align="center">
  <img src="../data/icons/DracoPho-logo.png" alt="DracoPho Logo" width="128" />
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

[English README](../README.md)

他の言語で読む：
[简体中文](../README.zh-CN.md) · [繁體中文](./README.zh-TW.md) ·
[日本語](./README.ja.md) · [한국어](./README.ko.md) ·
[Русский](./README.ru.md) · [Italiano](./README.it.md) ·
[العربية](./README.ar.md) · [Français](./README.fr.md) ·
[Deutsch](./README.de.md) · [Español](./README.es.md) ·
[Português](./README.pt.md)

**タグ**: `C++` / `Qt 6` / `屏幕截图` / `图像标注` / `桌面贴图` / `OCR 识别` / `滚动长截图` / `Wayland` / `Windows`


<details>
<summary>デモ動画</summary>
<p align="center">
  <video src="https://github.com/user-attachments/assets/4f86fcee-fef9-409e-98ba-1491ecee06c7" width="100%" controls></video>
</p>
</details>

`mark-shot` は Qt 6 ベースで開発された高性能スクリーンショット・注釈ツールです。もともと `niri` などの Wayland ウィンドウマネージャ向けに設計されましたが、現在は Linux（X11、GNOME、wlroots/Wayland デスクトップ）および Windows 環境で、一般的なスクリーンショットと注釈のワークフローをサポートしています。

画面を瞬時にキャプチャし、自動調整される全画面注釈オーバーレイを開いて、領域の切り抜き、注釈、クリップボードへのコピー、保存、デスクトップピンなどの機能を提供します。

---

## 主な機能

### 注釈ツールボックス
- **ペンと蛍光ペン**: なめらかな自由線の描画と半透明のハイライト重ね塗りに対応。
- **幾何ベクトルツール**: 高精度の直線・矩形・楕円パスを描画。矩形は次の 3 スタイルを切り替え可能:
  - `描边`: 従来の枠線または塗りつぶしの矩形で、角丸も選択可能。
  - `高亮`: `CompositionMode_Multiply` と半透明の塗りつぶしによる蛍光ペン風のオーバーレイ効果。
  - `反色`: 矩形が覆う領域のピクセルに対して RGB 反転を行い、外枠は視覚的なヒントとして残します。
- **最適化された矢印**: 6 頂点のクラシックな矢印パスを採用し、エッジが滑らかでアンチエイリアス描画に対応。
- **二重連動テキスト**:
  - 超特大のフォントサイズを無段階に調整可能。マウスホイールまたはプロパティスライダーで滑らかに拡大縮小できます。
  - 物理的な幅バッファの設計を採用し、極端に高いズーム率でもレンダリングの振動による意図しない折り返しを防止します。
  - **対角のコントロールポイント**でフォントサイズとテキストボックスを比率連動で拡大縮小できます。**左右のコントロールライン**はレイアウト境界の幅のみを調整します。
- **レーザー演示ペン**: プレゼンテーションや授業に最適で、筆跡は時間とともに滑らかに溶けて消えます。
- **自動採番のステップ番号**: クリックするだけで順に増える数字のステップマーカーを配置できます。
- **モザイク**: 機密情報に対して毛ガラス風の領域ぼかしを適用できます。
- **2 フレームを個別調整できる拡大鏡**: 拡大鏡の内側のビューファインダー枠と外側のレンズにはそれぞれリサイズハンドルが付いています。矩形レンズは各フレームに 8 個の角/辺ハンドル、円形レンズは各フレームに上下左右 4 個のハンドルがあります。どちらかのフレームを調整すると、拡大倍率に応じて他方のフレームも連動し、倍率は常に一定に保たれます。片方のフレームだけを移動しても、もう一方のフレームは元の位置に留まります。
- **起動時のコードスキャン**: 領域選択の前に `Q` を押してコードスキャンモードに入ると、QR コードまたはバーコードの領域を枠で選択した後、コピー可能な認識結果ウィンドウが開きます。
- **ディスプレイのクイックキャプチャ**: 領域選択の前に `D` を押すと、すべての出力画面を即座にキャプチャし、ディスプレイごとに切り出してサムネイルを表示します。サムネイルにカーソルを合わせると、そのディスプレイのスクリーンショットをコピー・編集・保存できます。
- **GIF・MP4・アニメーション WebP の録画**: 起動時の録画ショートカットまたはトレイメニューから、指定したディスプレイやカスタム領域を GIF、MP4、またはアニメーション WebP として録画できます。録画はクラッシュセーフです: MP4 は一時的な MKV に書き出され、終了時にリマックスされるため、中断された録画も復旧可能です。録画中はトレイと凍結フレームにステータスが表示され、`S`、オーバーレイのボタン、トレイメニュー、または `--stop-recording` で停止できます。開始時と保存時にはデスクトップ通知が送信されます。Wayland では録画は PipeWire portal バックエンドを優先して使用し、portal のキャプチャが利用できない場合は wlroots screencopy またはポーリングキャプチャにフォールバックします。
- **画像ホストへのアップロード**: 領域選択後に `Ctrl+U` を押すか、ツールバーのアップロードボタンをクリックすると、現在のスクリーンショットをカスタム画像ホスト（ImgURL、sm.ms、imgbb、litterbox など）にアップロードし、成功すると URL が自動的にクリップボードへコピーされます。`upload.env` による画像ホストのパラメータ設定、または `upload.command` による任意のカスタムアップロードスクリプトの組み込みに対応しています。
- **Mac スタイルの書き出し外枠**: 保存・コピー・アップロード・開く方法・拡張コマンドの画像に、透明な余白、角丸、柔らかな影を追加します。

### デスクトップピン（Pin）
- スクリーンショットまたは注釈領域を、独立した枠なしの常時最前面のピンウィンドウとして画面に固定できます。
- ピンウィンドウ内で OCR 認識されたテキストを直接選択し、`Ctrl + C` または右クリックメニューで画像内の文字をコピーできます。
- OpenAI 互換の API を介して LLM を呼び出し、OCR テキストを翻訳して、原文の位置に合わせてピン画像上に重ねて表示できます。
- **便利な操作**:
  - マウス左ボタンのドラッグでピンの位置を自由に移動できます。
  - マウスホイールのスクロールでピンを等比率で拡大縮小できます。
  - マウス左ボタンをダブルクリックするか、`Esc` キーを押すとピンを閉じます。
  - 右クリックでメニューが開き、多角度の回転、画像内文字のコピー、翻訳、名前を付けて保存、コピー、閉じるなどができます。

### スクロールスクリーンショット
- PipeWire screencast、インタラクティブなスクロールオーバーレイ、画像スティッチャーにより、長いページや広い領域のスクリーンショットをキャプチャします。
- この機能は主に `niri` やそれに近い動作をする Wayland 環境を対象としています。これらの環境では出力ジオメトリ、キャプチャのタイミング、ウィンドウ位置を安定に保ちやすいためです。
- **大きな選択領域のフローティングハンドル**: 選択したスクリーンショット領域が大きすぎて、スクロールプレビューパネルを表示するのに十分な画面領域が残っていない場合、プレビューパネルは自動的に非表示になり、選択領域の端に**フローティングドラッグハンドル**（方向矢印付きのフローティングボタン）が表示されます。
  - **ドラッグで選択領域を調整**: フローティングハンドルを長押ししてドラッグすると、スクロール軸に沿ってスクリーンショットの選択領域を移動し、最初の画面範囲を超えた内容をキャプチャできます。
  - **クリックで軸を切り替え**: キャプチャを開始する前に、フローティングハンドルをクリックするとスクロール方向（垂直/水平）を直接切り替えられます。
- **互換性の注意**: KDE、GNOME、X11、その他の `niri` 以外の環境でのスクロールスクリーンショットはまだ試験的な機能であり、未完成です。これらのデスクトップスタックでは、portal バックエンドのポリシー、Shell やウィンドウマネージャの動作、ウィンドウジオメトリのフィードバック、フレームのタイミング、スクロールイベントの処理に違いがあります。
- スクロールスクリーンショットが使えない場合は、通常のスクリーンショットの流れを使用するか、DracoPho の拡張コマンドで外部の長尺スクリーンショットツールを利用してください。
- スクロールスクリーンショットの問題を報告する場合は、まず `mark-shot --debug --debug-log /path/to/mark-shot.log` を実行して問題を再現し、ログを GitHub issue に添付してください。`config.json` の `debug.enabled` と `debug.logPath` でも有効にできます。`DEBUG=1` と `MARK_SHOT_DEBUG_LOG=/path/to/log` も引き続き利用できます。

### ディスプレイサーバー横断対応
- **Wayland**: PipeWire portal screencast による録画と実験的なスクロールスクリーンショットをサポートし、共有メモリと DMA-BUF の 2 種類のフレームパスを処理します。`grim` で wlroots のスクリーンショット、`layer-shell-qt` でネイティブオーバーレイの作成、`wl-copy` でクリップボードの保持を行います。
- **X11**: `QScreen::grabWindow` でスクリーンショット、全画面の最前面ウィンドウをオーバーレイとして使用し、`xclip` でクリップボードを保持します。
- **Windows**: Qt ネイティブのスクリーンショットとクリップボード API を使用して、基本的なスクリーンショット、注釈、コピー、保存、ピンのワークフローをサポートします。PipeWire、xdg-desktop-portal、`grim`、XCB ウィンドウ検出、LayerShellQt、GNOME Shell helper などの Linux 専用バックエンドはコンパイル時に無効化されます。
- Linux のディスプレイサーバーバックエンドは、実行時に `$XDG_SESSION_TYPE` によって自動検出されます。Windows では Qt ネイティブのプラットフォームバックエンドを使用します。
- **Multi-monitor freeze scope**: 既定では、領域選択時に接続されているすべてのディスプレイがフリーズされます（X11/Windows で DPR が一致する場合は単一の仮想デスクトップウィンドウとして表示）。1 つのモニターで選択を確定すると、他のディスプレイはセッションが終了するまでフリーズされたまま操作できません。**Cursor Screen** スコープはカーソルがあるモニターだけをフリーズします。

### デスクトップ統合
- **起動時の動作を設定可能**: DracoPho の起動時にスクリーンショットオーバーレイを自動で開かなくなりました。設定 → 一般 → **起動時の動作** で、**トレイアイコン**・**フローティングボール**・**設定ウィンドウ**・**直接スクリーンショット** の4モードを自由に組み合わせられます。新規インストールの既定はトレイアイコン + フローティングボールで、直接スクリーンショットはオプトインです。
- **フローティングボール**: 小さな常時最前面の丸いボール（既定は画面右下）で、スクリーンショット・全画面スクリーンショット・録画・設定へのクイックアクセスを提供します。シングルクリックでメニュー、ダブルクリックで直接スクリーンショット、ドラッグで移動（位置は記憶されます）。スクリーンショット中は自動的に非表示になります。
  画面端の近くにドラッグすると自動的に吸着し、**端側の半分が画面外に隠れます**（ホバーすると表示され、離すと再び非表示になります。X11/Windows/macOS で有効。ネイティブ Wayland では位置をコンポジタが管理するため、プロトコル上の制約によりボールは自由浮遊のまま）。数秒間アイドル状態が続くと半透明にフェードアウトし、ホバーするとすぐに不透明度が戻ります。
- **デスクトップショートカット**:
  - `mark-shot.desktop`: システム全体のスクリーンショットツールとして設定され、システムショートカットから直接呼び出せます。
  - `mark-shot-edit.desktop`: 独立した画像エディタとして登録され、ファイルマネージャ（Dolphin、Nautilus など）の右クリックメニューの「アプリで開く」に統合できます。
- 高解像度のシステムベクターアイコン `dracoPho.svg` と `mark-shot-edit.svg` が同梱されています。

### KDE KWin ScreenShot2 の認可

KDE Wayland では、DracoPho は KWin の `org.kde.KWin.ScreenShot2` インターフェースを使用して正確な領域スクリーンショットを実行できます。KWin はこのインターフェースを制限付き D-Bus インターフェースとして扱うため、対応するアプリのデスクトップファイルで認可フィールドを宣言する必要があります。

<details>
<summary>KDE KWin ScreenShot2 の認可とデスクトップファイル設定の説明 (クリックで展開)</summary>

認可フィールドの宣言:
```ini
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

ディストリビューションのパッケージと `cmake --install` は必要なデスクトップファイルを自動的にインストールします。プロジェクトをインストールせずにローカルのビルド成果物を直接実行する場合は、`~/.local/share/applications/mark-shot.desktop` を作成または更新してください:

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

KDE のコマンドショートカットサービスで DracoPho をバインドする場合は、`~/.local/share/applications/net.local.mark-shot.desktop` も作成する必要があります:

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

デスクトップファイルを変更したら、KDE がデスクトップファイルのキャッシュを再読み込みするよう、一度ログアウトして再ログインすることをお勧めします。現在の KDE セッションで依然として `NoAuthorized` が返る場合は、KWin を再起動するか、システムを一度再起動してください。
</details>


---

## 機能比較

DracoPho コミュニティ版は、**オープンソース（MIT）、クロスプラットフォーム（Linux X11/Wayland ネイティブ + Windows）、完全オフライン**のスクリーンショット・注釈・ピン・OCR・翻訳・録画を一体化したツールです。下表は各製品の公式ドキュメントと公式サイト（**2026 年 8 月**時点）をもとに、主要なデスクトッププラットフォーム全体で最も人気のあるスクリーンショットツール（オープンソース・商用を問わず）を網羅しています。機能の有無は正直に表記しています：**✅ 内蔵対応**；**⭕ 部分対応（有料プラン・プラットフォーム・外部ツール/サービスの制限を受ける）**；**❌ 非対応**。集計はすべて「初期状態ですぐ使える（アウトオブザボックス）」基準で行っており、有料プラン・クラウドサービス・実験的なブランチには連動していません。個々の ⭕ の具体的な制限は下記の詳細注記で説明しています。

### コア機能の概要

#### I. 撮影（「必要なものを撮れるかどうか」）

| 機能 | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 領域スクリーンショット | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 全画面 / マルチモニタースクリーンショット | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| ウィンドウスクリーンショット | ⭕ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 遮蔽 / 最小化されたウィンドウの内容 | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| スクロール / 長尺スクリーンショット | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ⭕ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |
| タイマー / 遅延スクリーンショット | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⭕ | ❌ | ✅ |

#### II. 注釈とインテリジェンス（「撮影後、効率よく使えるか」）

| 機能 | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 注釈ツールセット | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| デスクトップピン | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ |
| OCR 文字認識 | ✅ | ✅ | ✅ | ⭕ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 翻訳 | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| QR コード / バーコード認識 | ✅ | ✅ | ✅ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ⭕ |
| カラーピッカー | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ✅ | ✅ | ✅ | ✅ |

#### III. 出力と自動化（「撮影後、そのまま共有・蓄積できるか」）

| 機能 | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 画面録画 | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ |
| アニメーション GIF / WebP 出力 | ✅ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ |
| 画像アップロード / クラウド | ✅ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ |
| ヘッドレス CLI / スクリプト化 | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| フローティングボールランチャー | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| キャプチャ履歴 | ❌ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ | ⭕ | ❌ | ✅ | ✅ | ⭕ | ❌ | ⭕ |

#### IV. プラットフォームとエコシステム

| 機能 | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Linux Wayland ネイティブ | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| クロスプラットフォーム（2 以上のデスクトップ OS） | ⭕ | ❌ | ⭕ | ✅ | ✅ | ✅ | ❌ | ⭕ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ |
| プラグイン / 拡張メカニズム | ✅ | ⭕ | ❌ | ❌ | ⭕ | ✅ | ⭕ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| オープンソース / 無料 | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ✅ | ⭕ | ✅ | ❌ | ❌ | ✅ | ⭕ | ⭕ |

### 機能の詳細

**I. 撮影**（「必要なものを撮れるかどうか」）

- **領域 / 全画面 / マルチモニタースクリーンショット** — 掲載 15 ツールすべてに内蔵されています。DracoPho はさらに、負の座標のディスプレイとディスプレイごとの HiDPI スケーリングを正しく処理し、マルチモニターのスクリーンショットを結合してもゴーストが発生しません。
- **ウィンドウスクリーンショット** — DracoPho の対話モードは全画面の選択オーバーレイを使用します（専用の「アクティブウィンドウ」ワンクリックモードがないため ⭕）。ヘッドレスモード（`--window`）では id・タイトル・クラス・**PID またはプロセス名**でウィンドウを正確に選択できます。Flameshot にはウィンドウモードがまったくなく、その他のツールはすべて備えています。
- **遮蔽 / 最小化されたウィンドウの内容** — X11 では、DracoPho は XComposite 経由でウィンドウ自身のコンポジットバッファを読み取り、ウィンドウが完全に遮蔽されたり最小化されたりしていても実際の内容をキャプチャできます（前面へのポップアップやフォーカスの奪取はありません）。同種のツールでは scrot の `--stack` だけがこれを実現できます。Wayland のプロトコルでは、どのツールも最小化されたウィンドウの内容を読み取れません。
- **スクロール / 長尺スクリーンショット** — DracoPho は niri と GNOME Wayland（公式拡張機能）でネイティブに動作し、KDE/X11 では実験的対応です。ShareX・PixPin・PicPick・Snagit・CleanShot X・Shottr・Xnip・iShot は内蔵、Greenshot はレガシー IE のシナリオのみ、Snipaste・Flameshot・ksnip・Spectacle・Windows Snipping Tool は非対応です。
- **タイマー / 遅延スクリーンショット** — DracoPho は設定した秒数だけ待ってからキャプチャを開始します（全画面カウントダウンオーバーレイ、Esc でキャンセル）。`--delay`・トレイ / フローティングボールの「遅延スクリーンショット」サブメニュー・キャプチャ設定ページから利用できます。Flameshot・ksnip・Spectacle などは単純な遅延起動のみ、ShareX はタイマーキャプチャに対応し、PixPin・Snipaste・Xnip は非対応です。

**II. 注釈とインテリジェンス**（「撮影後、効率よく使えるか」）

- **注釈ツールセット** — DracoPho は 12 種類以上のツールを内蔵しています：ペン、蛍光ペン、直線、矩形、楕円、矢印、テキスト、ステップ番号、モザイク、2 フレーム拡大鏡、レーザーポインター、カラーピッカー、ルーラー。全 15 ツールが程度の差はあれ注釈を内蔵しています。
- **デスクトップピン** — DracoPho は枠なしの常時最前面スティッカー（ピン）を提供し、拡大縮小、回転、OCR 文字選択、ピン上での LLM 翻訳に対応します。Snipaste・PixPin・ShareX・ksnip・CleanShot X・Shottr・Xnip・iShot にもあります。Flameshot・Spectacle・Greenshot・PicPick・Snipping Tool・Snagit にはありません。
- **OCR 文字認識** — DracoPho は RapidOCR（オフラインの PP-OCR モデル）と Tesseract フォールバックを同梱しており、初期状態ですぐ使えます。Windows Snipping Tool（ローカル Text Actions）・ShareX・Snagit・CleanShot X・Shottr・Xnip・iShot は内蔵、Snipaste は有料プラン限定、ksnip はプラグイン、Spectacle は外部の Tesseract インストールが必要です。Flameshot・Greenshot・PicPick にはありません。
- **翻訳** — DracoPho は OpenAI 互換の LLM によるスクリーンショット翻訳を同梱しています（オフライン / 自前サーバー対応）。スクリーンショット翻訳を内蔵しているのは iShot だけで、PixPin は会員限定、その他のツールにはありません。これは最も強い差別化要因の 1 つであり続けています。
- **QR コード / バーコード認識** — DracoPho は撮影中に `Q` キーで QR・一次元コード・PDF417 をスキャンします。ShareX と PixPin は対応、Snipaste（有料）と CleanShot X / Shottr / iShot は OCR エンジン経由で QR のみ読み取れます。
- **カラーピッカー** — DracoPho は履歴パレット付きの画面カラーピッカーを内蔵しています。Spectacle と Snagit を除くほとんどのツールにあります。Windows Snipping Tool は Copilot+ AI PC のみ対応です。
- **ワンクリック AI ぼかし** — WeChat の 2026 年自動ぼかしはまだ内蔵されていません（❌）。DracoPho のモザイク / ぼかしは手動です。Snagit（AI Smart Redact）と Snipping Tool のテキストぼかしが同等の実装で、自動ぼかしは今後の対応候補です。

**III. 出力と自動化**（「撮影後、そのまま共有・蓄積できるか」）

- **画面録画** — DracoPho は MP4 / GIF / アニメーション WebP を録画でき、静かな無人録画（領域 / ディスプレイ）に対応します（ウィンドウなし・ダイアログなし・フォーカス奪取なし）。ShareX・PixPin・Spectacle（Plasma 6）・PicPick・Snipping Tool・Snagit・CleanShot X・iShot は内蔵、Snipaste・Flameshot・ksnip・Greenshot・Shottr・Xnip にはありません。
- **アニメーション GIF / WebP 出力** — DracoPho は**アニメーション WebP**（ファイルサイズが小さく、アルファチャンネル対応）をネイティブに録画できます。競合でアニメーション WebP をネイティブに出力できる製品はありません。PixPin と iShot は GIF 録画、ShareX は GIF 録画（WebP は静止画保存のみ）、Snagit と CleanShot X は GIF 書き出しに対応し、その他にはアニメーション出力がありません。
- **画像アップロード / クラウド** — DracoPho は ImgURL / sm.ms / imgbb / litterbox / カスタムコマンドによるアップロード（`Ctrl+U`）を内蔵しています。アップロード先は ShareX が最多で、Greenshot・PicPick・Snagit・CleanShot X はクラウドアップロードを内蔵、Flameshot は Imgur のみ、ksnip は Imgur/FTP/スクリプト、Shottr はアクティベーション後に S3 に対応します。Snipaste・PixPin・Snipping Tool・Xnip・iShot にはありません。
- **ヘッドレス CLI / スクリプト化** — DracoPho は完全なヘッドレスパイプラインを提供します：`--capture-to` による領域 / ディスプレイのスクリーンショット、`--list-windows` + `--window` によるウィンドウ / コンポーネントのキャプチャ、`--record-region` / `--record-display` による無人録画、`--record-wait-json` による構造化ステータス出力。すべてウィンドウ・ダイアログなし・フォーカス奪取なしで動作し、**PID / プロセス名の指定と遮蔽・最小化されたウィンドウのキャプチャ**に対応します。商用版の MCP サーバーは同じパイプラインを再利用しています。ShareX（Windows のみ）・Flameshot・ksnip・Spectacle は堅牢な CLI を備え、Snipaste の CLI は有料プラン機能、PixPin は JS アクションエンジンのみです。
- **フローティングボールランチャー** — DracoPho は画面端に吸着するドラッグ可能なボールを備え（X11/Windows/macOS。Wayland はプロトコル制約のため自由浮動）、アイドル時にはフェードアウトします。同様の機能を持つのは PixPin だけです。
- **キャプチャ履歴** — DracoPho には専用の履歴パネルがまだありません。ShareX・Snipaste・Flameshot・Snagit・CleanShot X にはあり、ksnip / PicPick / Shottr / iShot は軽量な代替機能（タブ / ギャラリー / ピン留めストック）があり、その他にはありません。

**IV. プラットフォームとエコシステム**

- **Linux Wayland ネイティブ** — DracoPho は PipeWire portal、grim、layer-shell、KDE KWin ScreenShot2、GNOME 拡張機能をネイティブにサポートします。オープンソースでは Spectacle（KDE）だけが同等の深さを持ち、Flameshot / ksnip は実験的または portal 依存です。
- **クロスプラットフォーム** — Snipaste・Flameshot・ksnip・Snagit は主要 3 デスクトップ OS（Windows + macOS + Linux）すべてをカバーします。DracoPho は Linux + Windows（macOS は計画中）。ShareX・PicPick・Snipping Tool・Spectacle・CleanShot X・Shottr・Xnip・iShot は単一プラットフォームです。
- **プラグイン / 拡張メカニズム** — DracoPho は GitHub プラグインマーケット付きの Qt プラグインシステムを提供します（OCR / 翻訳 / コードスキャンの provider を拡張可能）。ksnip と Greenshot にはプラグイン API があり、ShareX・Spectacle・Snagit・CleanShot X・Shottr はカスタムアクション / 統合で代替しています。
- **オープンソース / 無料** — DracoPho コミュニティ版は MIT ライセンスで、完全無料・広告なし・アカウント不要・ネットワーク接続任意です。ShareX・Flameshot・ksnip・Spectacle・Greenshot（Windows）も無料のオープンソース、Shottr と Snipping Tool は無料、PixPin / Snipaste / PicPick / Xnip / iShot はクローズドソースのフリーミアム、Snagit / CleanShot X は有料の商用ソフトウェアです。

> 注記：各製品の公式サイトと公式ドキュメント（2026-08）をもとに作成しています。機能はリリースごとに変化するため、各製品の最新ドキュメントを参照してください。FastStone Capture・Nimbus・Lightshot・Sogou スクリーンショット・WeChat / QQ スクリーンショットは上記の比較表に含まれていません（WeChat の 2026 年ワンクリック AI ぼかしがこの分野をリードしています）。

---

## コマンドラインインターフェース (CLI)

### よく使う使用例

```bash
# 捕获屏幕并进入区域裁剪与标注模式
mark-shot

# 在多显示器环境下捕获所有输出屏幕
mark-shot --all-outputs

# 跳过选区步骤，直接对捕获的完整屏幕截图进行标注
mark-shot --fullscreen

# 选区完成后默认使用移动工具，全屏标注默认使用激光笔，并设置红色默认颜色
mark-shot --default-tool move --fullscreen-default-tool laser --default-color '#FF4D4D'

# 打开一个已有的本地图片文件并直接进入标注模式
mark-shot path/to/image.png

# 直接将本地图片作为贴图窗口打开
mark-shot --pin-image path/to/image.png

# 强制使用标准的 XDG 全屏普通窗口运行（而非 Wayland layer-shell）
mark-shot --xdg-window
```

#### ヘッドレス（非対話）スクリーンショット

スクリプト、CI 自動化、その他のプログラムは、注釈 UI を開かずに `mark-shot` を呼び出してスクリーンショットを取得できます。キャプチャされたフレームは PNG として書き出され、1 行のコンパクトな JSON サマリーが標準出力に出力されます:

```bash
# 捕获主屏并写入 PNG
mark-shot --capture-to /tmp/shot.png

# 写入目录（自动生成带时间戳的文件名）
mark-shot --capture-to /tmp/shots/

# 捕获逻辑屏幕区域（x,y,宽度,高度）
mark-shot --capture-to /tmp/region.png --region 0,0,1280,720

# 按显示器名称捕获指定屏幕，并包含鼠标指针
mark-shot --capture-to /tmp/window.png --display DP-1 --include-cursor

# 同时捕获多个显示器（可重复 --display，每个显示器一张 PNG）
mark-shot --capture-to /tmp/shots/ --display DP-1 --display DP-2

# 以 JSON 输出当前所有显示器信息并退出
mark-shot --list-displays
```

単一ディスプレイの `--capture-to` における JSON 出力例:

```json
{"path":"/tmp/shot.png","width":2560,"height":1440,"output":"DP-1","error":null}
```

複数の `--display` を指定した場合、出力は各ディスプレイのキャプチャを含む配列になります:

```json
{"captures":[{"path":"/tmp/shots/mark-shot-DP-1-20260801-000000.png","width":2560,"height":1440,"output":"DP-1","error":null},
             {"path":"/tmp/shots/mark-shot-DP-2-20260801-000000.png","width":1920,"height":1080,"output":"DP-2","error":null}]}
```

選択した各ディスプレイはそれぞれのソースジオメトリでキャプチャされるため、portal 系バックエンドは仮想デスクトップ全体ではなく、そのディスプレイを正確に返します。

ヘッドレススクリーンショットは、対話 UI と同じすべてのキャプチャバックエンド（QScreen、xdg-desktop-portal、PipeWire、grim、KWin/GNOME ヘルパー、Windows Graphics Capture）を再利用するため、画質と領域切り抜きの挙動は完全に一致します。すべてのヘッドレスパラメータと位置指定の画像ファイルパラメータは互いに排他的です。

### CLI パラメータの説明

| パラメータ | 機能説明 |
| :--- | :--- |
| `[file]` | **位置パラメータ**: 現在の画面をキャプチャする代わりに、既存のローカル画像ファイルを開いて注釈モードに入ります。 |
| `-h`, `--help` | ヘルプ情報を表示して終了します。 |
| `-v`, `--version` | 現在のバージョン情報を表示して終了します。 |
| `--all-outputs` | 現在のアクティブな画面だけでなく、仮想ディスプレイデスクトップのすべての出力画面をキャプチャします。 |
| `--xdg-window` | デフォルトの Wayland オーバーレイ（layer-shell）の代わりに、標準の XDG 全画面通常ウィンドウ（xdg-shell）を強制使用します。 |
| `--fullscreen` | 領域選択のステップをスキップし、キャプチャした全画面スクリーンショットに直接注釈を付けます。 |
| `--default-tool <tool>` | 通常の領域選択完了後のデフォルト注釈ツールを指定します。`--fullscreen-default-tool` が未設定の場合、全画面モードのデフォルトツールにもなります。 |
| `--fullscreen-default-tool <tool>` | 全画面注釈モードのデフォルトツールを指定します。 |
| `--default-color <color>` | デフォルトの注釈色を指定します。`#RRGGBB` と `#RRGGBBAA` に対応しています。 |
| `--tray` | DracoPho をシステムトレイで常駐させ、プラットフォームが対応していればグローバルなスクリーンショットショートカットを登録します。 |
| `--capture` | 設定でトレイ自動起動が有効な場合、スクリーンショットを 1 回強制的に実行します。 |
| `--pin-image <path>` | ローカル画像を直接ピンウィンドウとして開き、スクリーンショットと領域選択のフローをスキップします。 |
| `--recording-status` | 実行中のインスタンスから現在の録画ステータス JSON を出力します。 |
| `--stop-recording` | 実行中のインスタンスに現在のアクティブな録画の停止を要求します。 |
| `--record-region <x,y,w,h>` | 実行中のインスタンスを介して画面領域を録画します。ジオメトリは x,y,幅,高さ です。 |
| `--record-display <id>` | ID でディスプレイを録画します（`--list-displays` の出力を参照: `DP-1` などの生の画面名、`output:DP-1` キー、または `all`）。 |
| `--record-output <path>` | 録画の出力ファイルパス（録画フラグ使用時に必須）。 |
| `--record-duration <seconds>` | 録画時間（秒）。0 は `--stop-recording` まで録画します。 |
| `--record-fps <n>` | 録画のフレームレート（デフォルト 15）。 |
| `--record-format <mp4\|gif\|webp>` | 録画フォーマット: mp4（デフォルト）、gif、または webp。 |
| `--record-audio` | 録画にシステムオーディオを含めます。 |
| `--record-wait-json` | 録画が終了するまで待機し、最終ステータスを JSON で出力します。 |
| `--debug` | この実行でデバッグログを有効にします。 |
| `--no-debug` | この実行でデバッグログを無効にし、設定ファイルと環境変数を上書きします。 |
| `--debug-log <path>` | デバッグログを指定したパスに書き込みます。`--no-debug` も同時に設定しない限り、デバッグログが有効になります。 |
| `--capture-to <path>` | ヘッドレススクリーンショット: UI を開かずに PNG を指定したファイルまたはディレクトリに書き込み、標準出力に JSON サマリーを出力します。 |
| `--region <x,y,w,h>` | `--capture-to` と併用します: 指定した論理画面領域のみをキャプチャします。 |
| `--display <name>` | `--capture-to` と併用します: ディスプレイ名で指定した出力画面をキャプチャします。繰り返し指定すると、複数のディスプレイを一度にキャプチャできます（各画面につき PNG 1 枚）。 |
| `--include-cursor` | `--capture-to` と併用します: マウスポインターをキャプチャフレームに描画します。 |
| `--output-name <name>` | `--capture-to` と併用します: キャプチャパスがディレクトリの場合に使用されるベースファイル名（拡張子なし）です。 |
| `--list-displays` | 現在のすべてのディスプレイ情報を JSON で出力して終了します。 |

### ショートカットキーのバインド

`mark-shot` をシステムのスクリーンショットショートカットとしてバインドするには:

**niri**（`~/.config/niri/config.kdl` を編集）:
```kdl
binds {
    Mod+Shift+S { spawn "mark-shot"; }
}
```

**Hyprland**（`~/.config/hypr/hyprland.conf` を編集）:
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bind = SUPER SHIFT, S, exec, mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bind = , Print, exec, mark-shot
```

**Sway / i3**（`~/.config/sway/config` または `~/.config/i3/config` を編集）:
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bindsym Mod4+Shift+S exec mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bindsym Print exec mark-shot
```

**GNOME**: システム設定 → キーボード → キーボードショートカット → カスタムショートカットで追加します。

**トレイモード**:
```powershell
mark-shot --tray
```

トレイモードでは、デフォルトで以下のグローバルショートカットが登録されます:
- `Ctrl+Alt+S`: 領域スクリーンショットを開始します。

トレイメニューでは、スクリーンショット、全画面スクリーンショット、録画開始、録画ステータス、録画停止、設定、終了などの操作も提供されます。


### 拡張コマンド

右側のアクションツールバーには **Extensions** ボタンがあり、プログラムは `~/.config/mark-shot/extensions.json` からユーザー定義のコマンドを読み込みます。設定ファイルは JSON 配列でも、`commands` 配列を含む JSON オブジェクトでも構いません。

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

`command` は Unix 系システムでは `$SHELL -c`、Windows では `%COMSPEC% /C` で実行されるため、シェル式を利用できます。`{slurp}` を使うと、現在の選択領域を `x,y widthxheight` のジオメトリ文字列としてコマンドに渡せます。`{image}` または `{imagePath}` を使うと、レンダリング済みの現在の選択領域を一時 PNG のパスとしてコマンドに渡せ、`{imageUrl}` では `file://` URL を渡せます。これらのプレースホルダーは自動的にシェルクォート用のエスケープが行われるため、設定内でさらに引用符を追加する必要はありません。画像プレースホルダーを使用しない場合は、`saveImage` または `needsImage` を `true` に設定すると、プログラムが一時 PNG パスをコマンドの末尾に自動的に追加します。`workingDirectory` は `cwd` と同等です。`closeOnStart` のデフォルト値は `true` で、コマンド起動前に DracoPho を非表示にして閉じます。

### アプリケーション設定ファイル

[設定リファレンス](../docs/configuration.zh-CN.md) を参照してください。

### ユーザー操作マニュアル

日常の操作（ウィンドウホバーでの領域選択、注釈ツール、起動時ツール、ピンウィンドウ、スクロールスクリーンショット、ヘッドレス CLI、機能のセルフテスト一覧）については、[ユーザー操作マニュアル](../docs/user-guide.zh-CN.md)（[English](../docs/user-guide.md)）を参照してください。

他の言語で読む：
[简体中文](../docs/user-guide.zh-CN.md) · [繁體中文](../docs/user-guide.zh-TW.md) ·
[日本語](../docs/user-guide.ja.md) · [한국어](../docs/user-guide.ko.md) ·
[Русский](../docs/user-guide.ru.md) · [Italiano](../docs/user-guide.it.md) ·
[العربية](../docs/user-guide.ar.md) · [Français](../docs/user-guide.fr.md) ·
[Deutsch](../docs/user-guide.de.md) · [Español](../docs/user-guide.es.md) ·
[Português](../docs/user-guide.pt.md)

## ビルドとインストール

### インストールガイド

##### Arch Linux (AUR)
Arch Linux ユーザーは AUR ヘルパーを使って直接インストールできます:
```bash
# 从源码编译安装
paru -S mark-shot
# 或
yay -S mark-shot

# 安装预编译二进制包
paru -S mark-shot-bin
# 或
yay -S mark-shot-bin
```

`mark-shot` はソースからコンパイルされ、`mark-shot-bin` は GitHub Releases からプリコンパイルされた pacman パッケージをダウンロードしてインストールします。

##### NixOS
NixOS ユーザーは Flake input を追加してインストールできます
```nix
# flake.nix
mark-shot = {
  url = "github:tystudio-26020701/mark-shot-community";
  inputs.nixpkgs.follows = "nixpkgs";
};

# home-manager
home.packages = with pkgs; [
  # 其他用户应用
  inputs.mark-shot.packages.${pkgs.stdenv.hostPlatform.system}.default
]
```

##### その他のディストリビューション (プリコンパイル済みパッケージ)
その他のディストリビューション（Ubuntu、Debian、Fedora など）では、Releases ページからビルド済みのインストーラをダウンロードし、以下のコマンドでインストールします:
- **Debian / Ubuntu**:
  ```bash
  sudo apt install ./mark-shot_<version>_amd64.deb
  ```
- **Fedora**:
  ```bash
  sudo dnf install ./mark-shot-<version>-1.x86_64.rpm
  ```

> **Ubuntu 26.04 LTS**: DracoPho は Ubuntu 26.04 LTS（Resolute）で検証済み・サポート対象です。
> Ubuntu 26.04 では、ディストリビューション付属の Qt 6.10 パッケージをそのまま使ってソースからビルドできます
> （`aqtinstall` の手順は不要）:
>
> ```bash
> sudo apt install build-essential cmake ninja-build pkg-config \
>   qt6-base-dev qt6-wayland libpipewire-0.3-dev libxcb-cursor0 \
>   xdg-desktop-portal pipewire xclip
> cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
> cmake --build build
> ```
>
> ヘッドレススクリーンショット（`--capture-to`）、複数ディスプレイのスクリーンショット（繰り返し指定可能な `--display`）、およびローカルの
> MCP サービスは、Ubuntu 26.04 の Wayland（GNOME）と X11 の両セッションで実行できます。

### システム依存パッケージ

#### Wayland (Arch Linux)

```bash
sudo pacman -S --needed base-devel cmake ninja pkgconf qt6-base qt6-wayland layer-shell-qt pipewire grim wl-clipboard
```

#### X11/GNOME (Ubuntu/Debian)

```bash
# 构建工具
sudo apt install build-essential cmake ninja-build pkg-config libpipewire-0.3-dev

# Portal 与剪贴板工具
sudo apt install xdg-desktop-portal pipewire xclip

# Qt 6（若系统仓库无 Qt 6，可通过 aqtinstall 安装到用户目录）
pip install aqtinstall
aqt install-qt linux desktop 6.7.3 gcc_64 --outputdir ~/Qt
```

> **注意**: Ubuntu 22.04 などシステムに Qt 5 が含まれる環境では、`~/Qt` に Qt 6 をインストールしてもシステムに影響しません。ビルド時に `-DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64` を渡すだけで問題ありません。

#### fcitx5 中国語入力のサポート（X11 環境の Qt 6）

Qt 6 には fcitx5 のインプットメソッドプラグインが同梱されていません。X11 環境で fcitx5 による中国語入力を使用するには、このプラグインをソースからコンパイルする必要があります:

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

#### OCR バックエンド（オプション）

DracoPho の文字認識機能は、内蔵の `mark-shot-ocr` Python スクリプトに依存しています。このスクリプトは **RapidOCR**（推奨、PaddleOCR PP-OCR モデルベース）と **Tesseract**（フォールバック）をサポートしています。Linux ではこのスクリプトが自動的にインストールされますが、Windows では手動での設定が必要です。

<details>
<summary><b>Linux</b></summary>

```bash
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime
```

インストール後、`mark-shot-ocr` は自動的に検出されるため、追加の設定は不要です。

**環境変数**（オプション）:

| 変数 | 説明 | デフォルト値 |
|------|------|--------|
| `MARK_SHOT_OCR_VERSION` | PaddleOCR のバージョン（`PP-OCRv5`、`PP-OCRv4` など） | `PP-OCRv5` |
| `MARK_SHOT_OCR_MODEL_TYPE` | モデルサイズ: `mobile` または `server` | `mobile` |
| `MARK_SHOT_OCR_MODEL_DIR` | カスタムモデルの保存ディレクトリ | `~/.local/share/mark-shot/models` |
| `MARK_SHOT_OCR_NO_VENV` | `1` に設定すると仮想環境の自動切り替えを無効化 | — |
| `MARK_SHOT_OCR_PYTHON` | re-exec に使用する Python インタープリターのパスを指定 | `~/.local/share/mark-shot/ocr-venv/bin/python` |

</details>

<details>
<summary><b>Windows</b></summary>

内蔵のヘルパースクリプトは Windows 上では自動インストールされないため、以下の手順を手動で行う必要があります:

**1. Python 3 のインストール**

[python.org](https://www.python.org/downloads/) から Python 3.10 以降をダウンロードしてインストールします。インストール時に **Add python.exe to PATH** にチェックを入れてください。

**2. OCR ヘルパースクリプトのコピー**

[DracoPho リポジトリ](https://github.com/tystudio-26020701/mark-shot-community) 内の `scripts/mark-shot-ocr` を、ローカルディレクトリ（例: `%LOCALAPPDATA%\mark-shot\mark-shot-ocr.py`）にコピーします。

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\mark-shot"
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/tystudio-26020701/mark-shot-community/main/scripts/mark-shot-ocr" `
  -OutFile "$env:LOCALAPPDATA\mark-shot\mark-shot-ocr.py"
```

**3. 仮想環境の作成と依存パッケージのインストール**

```powershell
python -m venv "$env:LOCALAPPDATA\mark-shot\ocr-venv"
& "$env:LOCALAPPDATA\mark-shot\ocr-venv\Scripts\pip.exe" install -U pip rapidocr onnxruntime
```

> `onnxruntime` は CPU 推論を提供します。互換性のある GPU がある場合は、`onnxruntime-directml` または `onnxruntime-gpu` をインストールすると認識を高速化できます。

**4. `config.json` で `ocr.command` を設定**

`%LOCALAPPDATA%\mark-shot\config.json` を開き（存在しない場合は新規作成）、`ocr.command` を設定します:

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

`%LOCALAPPDATA%` を実際に展開されたパス（例: `C:\Users\あなたのユーザー名\AppData\Local`）に置き換えてください。`{image}` プレースホルダーは実行時に一時スクリーンショットのパスに置き換えられます。省略した場合、DracoPho が自動的に追加します。

> **ヒント**: すでに仮想環境の Python を直接使用しているため、環境変数 `MARK_SHOT_OCR_NO_VENV=1` を設定すると、スクリプト内蔵の仮想環境自動検出をスキップできます。

</details>

#### コードスキャンバックエンド（オプション）

```bash
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

コードスキャンヘルパーは `zxing-cpp` を優先して使用し、QR Code、Data Matrix、Aztec、PDF417、EAN、UPC、Code 39、Code 93、Code 128 などの一般的な形式をサポートしています。`pyzbar` や OpenCV がインストールされている場合は、フォールバックバックエンドとしても使用されます。

#### 画像ホストアップロードバックエンド（オプション）

画像ホストへのアップロード機能は、デフォルトで内蔵の `mark-shot-upload` Python スクリプトを使用し、追加の依存パッケージは不要です（Python 3 の標準ライブラリのみを使用）。このスクリプトは環境変数で画像ホストのパラメータを設定し、multipart/form-data アップロードプロトコルに対応する任意の画像ホストサービスをサポートします。

<details>
<summary>内蔵ヘルパーがサポートする環境変数</summary>

| 環境変数 | 説明 | デフォルト値 |
|---------|------|--------|
| `MARK_SHOT_UPLOAD_URL` | **必須**、画像ホストのアップロード API エンドポイント | — |
| `MARK_SHOT_UPLOAD_FIELD` | ファイルフィールド名 | `image` |
| `MARK_SHOT_UPLOAD_API_KEY` | API Key / Token | — |
| `MARK_SHOT_UPLOAD_AUTH_HEADER` | 認証ヘッダー名 | `Authorization` |
| `MARK_SHOT_UPLOAD_AUTH_SCHEME` | 認証方式（例: `Bearer`）。空の場合は API Key をそのまま使用 | `Bearer` |
| `MARK_SHOT_UPLOAD_URL_PATH` | JSON レスポンス内の URL のドット区切りパス（例: `data.url`） | 自動検出 |
| `MARK_SHOT_UPLOAD_DELETE_URL_PATH` | 削除 URL のパス | 自動検出 |
| `MARK_SHOT_UPLOAD_HEADER_xxx` | カスタムリクエストヘッダー（例: `MARK_SHOT_UPLOAD_HEADER_X-Custom=foo`） | — |
| `MARK_SHOT_UPLOAD_FIELD_xxx` | 追加のフォームフィールド（例: `MARK_SHOT_UPLOAD_FIELD_album=123`） | — |

</details>

<details>
<summary>設定例: ImgURL V3</summary>

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

ImgURL V3 は `Authorization: Bearer <token>` 認証を使用します（`AUTH_SCHEME` のデフォルトは `Bearer` なので変更不要です）。

</details>

<details>
<summary>設定例: sm.ms</summary>

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

sm.ms は Token をそのまま Authorization の値として使用するため、`AUTH_SCHEME` を空文字列に設定します。

</details>

<details>
<summary>設定例: imgbb</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://api.imgbb.com/1/upload?key=你的API_KEY",
    "MARK_SHOT_UPLOAD_FIELD": "image",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

imgbb は URL のクエリパラメータで API Key を渡すため、`API_KEY` の設定は不要です。

</details>

<details>
<summary>設定例: litterbox（一時画像ホスト、API Key 不要）</summary>

```json
"upload": {
  "command": "curl -sf --max-time 30 -A 'Mozilla/5.0' -F reqtype=fileupload -F time=72h -F fileToUpload=@{image} https://litterbox.catbox.moe/resources/internals/api.php",
  "timeoutMs": 35000
}
```

litterbox のレスポンスはプレーンテキストの URL（JSON ではありません）です。DracoPho は `http://`/`https://` で始まる出力を自動的にアップロード結果として認識します。

</details>

<details>
<summary>カスタムアップロードコマンド</summary>

内蔵ヘルパーで要件を満たせない場合は、`upload.command` で任意のカスタムアップロードスクリプトを組み込めます。コマンドは以下の要件を満たす必要があります:

1. **終了コード**: 成功時は 0、それ以外は失敗とみなされます
2. **出力形式**（どちらか一方）:
   - **JSON**: `{"url":"https://...","deleteUrl":"https://...","errors":[]}`（`url` は必須、その他は任意）
   - **プレーンテキストの URL**: stdout の最初の空でない行が `http://` または `https://` で始まる
3. **プレースホルダー**: `{image}`、`{imagePath}`、`{imageUrl}` をサポート。コマンドにプレースホルダーが含まれない場合、DracoPho は一時画像パスをコマンド末尾に自動的に追加します

```json
"upload": {
  "command": "/path/to/your-uploader.sh --file {image} --json",
  "timeoutMs": 30000,
  "env": {
    "UPLOADER_API_KEY": "xxx"
  }
}
```

`upload.env` 内の環境変数はカスタムコマンドにも渡されるため、設定を再利用しやすくなっています。

</details>

#### Windows

使用するコンパイラと一致する Qt 6、CMake、Ninja、および C++17 対応のコンパイラ（例: MSVC または MinGW）をインストールします。Windows ビルドでは Qt DBus、PipeWire、X11/XCB、LayerShellQt、`grim`、`wl-copy`、`xclip` は不要です。

```powershell
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
cmake --build build-windows
```

現在の Windows での対応範囲は、通常のスクリーンショットと画像注釈です。スクロールスクリーンショット、コンポジタ専用のウィンドウ検出、Linux デスクトップショートカットは Windows では利用できません。内蔵の Python ヘルパースクリプト（`mark-shot-ocr`、`mark-shot-code-scan`、`mark-shot-translate`）は自動インストールされないため、上記の [OCR バックエンド](#ocr-后端可选)、[コードスキャンバックエンド](#扫码后端可选)、および翻訳の章を参照して手動で設定してください。

### ビルドとコンパイル

```bash
# 使用系统 Qt 6
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 如果 Qt 6 安装在用户目录，额外指定 CMAKE_PREFIX_PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64

# 执行编译
cmake --build build
```

または nix を使用

```bash
nix build
```

LayerShellQt は自動検出されます。見つかった場合は完全な Wayland layer-shell サポートが有効になり、見つからなくてもビルドは通常どおり成功し、実行時に標準の全画面ウィンドウへ自動的にフォールバックします。

### インストールと統合

```bash
cmake --install build --prefix "$HOME/.local"
```

このコマンドは、実行ファイル、ヘルパースクリプト（`mark-shot-ocr`、`mark-shot-code-scan`、`mark-shot-translate`、`mark-shot-upload`）、デスクトップショートカット、アイコンをインストールします。

### GNOME Wayland スクロールスクリーンショット拡張機能

GNOME Wayland のスクロールスクリーンショットでは、**DracoPho Scroll Helper** 拡張機能の有効化が必須です。この拡張機能がないと、DracoPho は選択した領域を静かに連続キャプチャできず、GNOME ネイティブのスクロールプレビューパネルも描画できないため、GNOME Wayland ではスクロールスクリーンショットのボタンが無効化されます。

拡張機能のファイルは、プロジェクトリポジトリの `../packaging/gnome-extension/mark-shot-scroll-helper@snemc.org` にあります。

<details>
<summary><b>GNOME Wayland スクロールスクリーンショット拡張機能のインストールと有効化ガイドを展開/折りたたみ</b></summary>

##### 方法 A: ディストリビューションパッケージでインストール
ディストリビューションパッケージ（例: `.deb` や `.rpm`）で DracoPho をインストールした場合、この拡張機能はすでにシステムにデフォルトインストールされています。以下のコマンドで現在のユーザーに対して拡張機能を有効にできます:
```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```
*拡張機能が見つからないと表示された場合は、一度ログアウトして再ログインした後に再度お試しください。*

##### 方法 B: リポジトリのソースディレクトリからインストール
ソースからビルドした場合や、ローカルで手動ビルドした場合は、まずこの拡張機能をユーザーの GNOME 拡張機能ディレクトリにコピーする必要があります:
```bash
# 定义扩展的 UUID
UUID=mark-shot-scroll-helper@snemc.org

# 创建用户级扩展目录
mkdir -p "$HOME/.local/share/gnome-shell/extensions"

# 从项目仓库中拷贝扩展文件
cp -r "packaging/gnome-extension/$UUID" "$HOME/.local/share/gnome-shell/extensions/"

# 启用该扩展（您可能需要重启 GNOME Shell 或注销并重新登录系统使该扩展生效）
gnome-extensions enable "$UUID"
```

helper の D-Bus インターフェースが利用可能かどうかを確認します:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
```

期待される結果は `('4.2',)` です。拡張機能を有効にした後、`mark-shot` を再起動してください。

</details>

---

## ショートカットキーとジェスチャーガイド

### ツール切り替えショートカット

| ショートカット | 切り替え対象ツール | 機能説明 |
| :---: | :--- | :--- |
| **V** | 移動 / ナビゲーション (Move / Pan) | 既存の画像モードで、画像キャンバスのパンとドラッグに使用します。 |
| **S** | 選択 (Select) | 描画済みのベクター注釈を選択して移動、拡大縮小、または削除します。 |
| **P** | ペン (Pen) | 自由曲線を描画します。 |
| **L** | 直線 (Line) | まっすぐなベクター線を描画します。 |
| **H** | 蛍光ペン (Highlighter) | 半透明のハイライトオーバーレイで、要点を強調するのに適しています。 |
| **R** | 矩形 (Rectangle) | 矩形の線枠を描画します。 |
| **E** | 楕円 (Ellipse) | 楕円形の線枠を描画します。 |
| **A** | 矢印 (Arrow) | クラシックな 6 頂点の細く鋭い矢印を描画します。 |
| **T** | テキスト (Text) | リッチテキストを入力・レイアウトします。1000px のフォントサイズとドラッグ連動に対応。 |
| **N** | 番号 (Number) | 自動で増えるステップ番号ラベルを配置します。 |
| **M** | モザイク (Mosaic) | 毛ガラス風の機密領域ぼかしを適用します。 |
| **G** | レーザーポインター (Laser) | 授業やプレゼンテーション用の一時的な筆跡で、自動的に滑らかに消えます。 |

### 起動画面の補助ツール

| ショートカット | ツール | 機能説明 |
| :---: | :--- | :--- |
| **C** | カラーピッカー (Color Picker) | スクリーンショット領域を選択する前に画素をサンプリングします。マウスホイールで拡大鏡のサイズを調整でき、左クリックでカラーパネルが開き、HEX、RGB、HSL、HSV、Qt などの形式でコピーできます。右クリックまたは Esc で通常の領域選択に戻ります。 |
| **R** | ルーラー (Ruler) | スクリーンショット領域を選択する前に座標を測定します。ホバーで現在の画素を表示し、左ドラッグで画素目盛り付きの測定矩形を描画して、幅、高さ、対角線、面積を表示します。右クリックまたは Esc で通常の領域選択に戻ります。 |
| **Q** | コードスキャン (Code Scanner) | QR コードとバーコードのスキャンモードに入ります。領域を枠で選択すると内のコード内容を認識し、コピー可能なウィンドウに結果を表示します。右クリックまたは Esc で通常の領域選択に戻ります。 |
| **D** | ディスプレイキャプチャ (Display Capture) | すべての出力画面を即座にキャプチャし、ディスプレイごとに切り出してサムネイルを表示します。サムネイルにカーソルを合わせるとコピー、編集、保存ができます。 |

### グローバル操作ショートカット

| ショートカット | 実行される操作 |
| :---: | :--- |
| **Esc** | 即座に終了し、注釈ウィンドウを閉じます。 |
| **Ctrl + C** | すべてのテキスト編集を確定し、現在のスクリーンショット/注釈付き領域をシステムクリップボードにコピーします。 |
| **Ctrl + S** または **Enter / Return** | すべてのテキスト編集を確定し、現在のスクリーンショットを保存します。 |
| **Ctrl + P** | 現在の領域をフローティングピンウィンドウとして固定します。 |
| **Ctrl + U** | 現在のスクリーンショットをカスタム画像ホストにアップロードします。成功すると URL が自動的にクリップボードにコピーされます。 |
| **Ctrl + Z** | 直前の注釈操作を元に戻します。 |
| **Ctrl + Y** または **Ctrl + Shift + Z** | 元に戻した注釈操作をやり直します。 |
| **Backspace** または **Delete** | **選択 (Select)** ツールがアクティブで注釈が選択されている場合、選択中の注釈を削除します。 |
| **F** | 現在のスクリーンショットの対象範囲を切り替えます（領域選択モードと全画面モードの切り替え）。 |

### 上級インタラクション操作のテクニック

- **図形の制約**: **矩形（Rectangle）** または **楕円（Ellipse）** を描画するとき、`Ctrl` キーを押し続けると正方形または正円に制約されます。
- **選択ツールへのクイック切り替え**: 注釈中にキャンバスの空白部分を右クリックすると、すぐに **選択（Select）** ツールへ切り替えられます。
- **右クリックのダブルクリックで色を素早く切り替え**: キャンバスの空白部分を右ダブルクリックすると、リング型カラーパレットが開き、現在の注釈ツールの色をすばやく切り替えられます。
- **ホイールでの無段階調整**: 対応する注釈ツールがアクティブな状態でマウスホイールをスクロールすると、現在のツールの線幅、フォントサイズ、番号ラベルのサイズ、モザイクグリッドのサイズをリアルタイムに調整できます。
- **キャンバスのパンとズーム**: **選択（Select）** ツールモード、またはローカルファイルの編集中に、マウスホイールをスクロールするとキャンバスをシームレスにズームでき、マウス中ボタンを押しながらドラッグするとキャンバスをパンできます。`Ctrl` をダブルクリックするとズームとパンをリセットします。

### ピンウィンドウ専用の操作

| ジェスチャー / ショートカット | 動作効果 |
| :--- | :--- |
| **マウス左ボタンを押したままドラッグ** | デスクトップピンの位置を自由に移動・配置します。 |
| **マウスホイール上/下** | ピンウィンドウを等比率で無段階に拡大/縮小します。 |
| **マウス左ボタンのダブルクリック** | そのピンウィンドウをすばやく閉じます。 |
| **マウス右ボタンのクリック** | 機能メニューを表示します（回転、画像内文字のコピー、翻訳、保存、コピー、閉じるなど）。 |
| **Esc キー** | フォーカス中のピンウィンドウを閉じます。 |

---

## リリースノート

[リリースノート](../docs/releases.zh-CN.md) を参照してください。

## フィードバックと交流

### Issue の提出
実行中に問題が発生した場合や新機能の提案がある場合は、GitHub CLI（`gh`）コマンドラインツールを使用して Issue を提出することをお勧めします。環境情報をワンクリックで収集して自動生成するスクリプトを提供しているため、詳細は [Issue 提出ガイド](../.doc/submit-issue-via-gh.md) を参照してください。

---

## ライセンス

本プロジェクトは **MIT ライセンス** のもとで公開されています。詳細は [LICENSE](../LICENSE) ファイルを参照してください。

## 謝辞

DracoPho はオープンソースコミュニティの成果の上に成り立っています。ここに心からの謝意を表します:

- **元のアップストリームプロジェクト [jswysnemc/mark-shot](https://github.com/jswysnemc/mark-shot) とその作者、ならびにすべての貢献者の皆様。** 本コミュニティ版は元のアップストリームプロジェクトを基に開発されており、その卓越した設計と継続的な貢献がすべての基盤となっています。彼らの素晴らしい仕事に心から感謝します。
- **[serendipitywgy](https://github.com/serendipitywgy)**: `serendipitywgy/mark-shot` を通じて、デスクトップ間の互換性の改善、OCR コピーツールバーアクション、スマート矩形枠の事前選択機能の貢献に感謝します。
- **DracoPho が依存するすべてのオープンソースプロジェクト**（Qt 6、PipeWire、xdg-desktop-portal、layer-shell-qt、wl-clipboard、xclip、grim、RapidOCR、onnxruntime、Tesseract、ZXing-C++ など）。

本コミュニティ版は [北京太殷造物科技有限公司](https://github.com/tystudio-26020701/mark-shot-community)（Beijing Taiyin Zhaowu Technology Co., Ltd.）および貢献者によってメンテナンスされており、**MIT ライセンス** のもとで公開されています。
