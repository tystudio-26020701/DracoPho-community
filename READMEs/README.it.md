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

Leggi questo README in altre lingue：
[简体中文](../README.zh-CN.md) · [繁體中文](./README.zh-TW.md) · [日本語](./README.ja.md) · [한국어](./README.ko.md) · [Русский](./README.ru.md) · [Italiano](./README.it.md) · [العربية](./README.ar.md) · [Français](./README.fr.md) · [Deutsch](./README.de.md) · [Español](./README.es.md) · [Português](./README.pt.md)

**Tag**: `C++` / `Qt 6` / `屏幕截图` / `图像标注` / `桌面贴图` / `OCR 识别` / `滚动长截图` / `Wayland` / `Windows`


<details>
<summary>Video dimostrativo</summary>
<p align="center">
  <video src="https://github.com/user-attachments/assets/4f86fcee-fef9-409e-98ba-1491ecee06c7" width="100%" controls></video>
</p>
</details>

`mark-shot` è uno strumento ad alte prestazioni per la cattura e l'annotazione di schermate, sviluppato con Qt 6. Il progetto è nato originariamente per window manager Wayland come `niri`; oggi supporta i normali flussi di lavoro di cattura e annotazione su Linux (X11, GNOME, desktop wlroots/Wayland) e su Windows.

Cattura istantaneamente lo schermo e apre una sovrapposizione di annotazione a schermo intero adattiva, offrendo funzioni come ritaglio delle aree, annotazione, copia negli appunti, salvataggio e blocco sul desktop.

---

## Caratteristiche principali

### Cassetta degli strumenti di annotazione
- **Pennello ed evidenziatore**: disegno di linee libere morbide e strati di evidenziazione semitrasparenti.
- **Strumenti vettoriali geometrici**: linee, rettangoli ed ellissi ad alta precisione. Il rettangolo supporta tre stili:
  - `Contorno`: il rettangolo originale con contorno o riempimento, con angoli arrotondati opzionali.
  - `Evidenziazione`: effetto di copertura stile evidenziatore ottenuto con `CompositionMode_Multiply` e riempimento semitrasparente.
  - `Inverti`: inverte i canali RGB dei pixel nell'area coperta dal rettangolo, mantenendo il contorno esterno come riferimento visivo.
- **Freccia ottimizzata**: usa il classico percorso a sei vertici, con bordi morbidi e rendering antialiased.
- **Testo a doppio collegamento**:
  - Supporta dimensioni di carattere molto grandi con regolazione continua, scalabili dolcemente tramite la rotellina del mouse o il cursore delle proprietà.
  - Introduce un buffer di larghezza fisica per evitare interruzioni di riga indesiderate dovute a oscillazioni di rendering a fattori di scala molto elevati.
  - **I punti di controllo diagonali** ridimensionano proporzionalmente sia la dimensione del carattere sia la cornice del testo; **le linee di controllo laterali** regolano invece solo la larghezza dei confini di impaginazione.
- **Laser per presentazioni**: adatto a demo o didattica; i tratti svaniscono dolcemente nel tempo.
- **Numerazione automatica dei passaggi**: un clic posiziona marcatori numerici progressivi.
- **Mosaico**: offusca con effetto vetro smerigliato le informazioni sensibili.
- **Lente d'ingrandimento a due cornici indipendenti**: la cornice di inquadratura interna e la lente esterna hanno ciascuna le proprie maniglie di ridimensionamento; la lente rettangolare ha 8 maniglie per cornice (angoli e lati), quella circolare 4 maniglie per cornice (alto, basso, sinistra, destra). Regolando una delle due cornici, l'altra viene collegata in base al fattore di ingrandimento, che rimane sempre invariato; spostando una singola cornice, l'altra resta in posizione.
- **Scansione dei codici in fase di avvio**: prima di selezionare, premi `Q` per entrare nella modalità di scansione; dopo aver selezionato l'area di un codice QR o di un codice a barre, si apre una finestra con il risultato riconosciuto, da cui è possibile copiarlo.
- **Cattura rapida dei display**: prima di selezionare, premi `D` per catturare immediatamente tutti gli schermi di output, ritagliandoli per display e mostrandoli come miniature; passando il mouse su una miniatura puoi copiare, modificare o salvare la cattura di quel display.
- **Registrazione GIF e video**: tramite la scorciatoia di registrazione della fase di avvio o il menu della barra di sistema, puoi registrare un display specifico o un'area personalizzata come GIF o MP4. Una registrazione attiva mostra il proprio stato nella barra di sistema e nel fotogramma congelato; puoi fermarla con `S`, il pulsante della sovrapposizione, il menu della barra di sistema o `--stop-recording`, e vengono inviate notifiche desktop all'avvio e al salvataggio. Su Wayland la registrazione usa preferibilmente il backend PipeWire portal; quando la cattura tramite portal non è disponibile, si ripiega su wlroots screencopy o sull'acquisizione a polling.
- **Caricamento su hosting di immagini**: dopo la selezione, premi `Ctrl+U` o fai clic sul pulsante di caricamento della barra degli strumenti per caricare la cattura corrente su un hosting di immagini personalizzato (ad es. ImgURL, sm.ms, imgbb, litterbox); al termine, l'URL viene copiato automaticamente negli appunti. Puoi configurare i parametri dell'hosting tramite `upload.env` oppure collegare qualsiasi script di caricamento personalizzato tramite `upload.command`.
- **Cornice di esportazione in stile Mac**: aggiunge margini trasparenti, angoli arrotondati e una morbida ombra alle immagini per salvataggio, copia, caricamento, apertura con altri programmi e comandi estesi.

### Blocco fluttuante (Pin)
- Consente di fissare sullo schermo la cattura o l'area annotata come finestra fluttuante indipendente, senza bordi e sempre in primo piano.
- Nella finestra bloccata puoi selezionare direttamente il testo riconosciuto con OCR e copiarlo con `Ctrl + C` o dal menu contestuale.
- Puoi richiamare un LLM tramite interfacce compatibili con OpenAI per tradurre il testo OCR e renderizzare la traduzione sulla finestra bloccata, sovrapposta nella posizione originale dell'immagine.
- **Interazioni comode**:
  - Trascinando con il tasto sinistro del mouse puoi spostare liberamente la finestra bloccata.
  - La rotellina del mouse ridimensiona la finestra bloccata in modo proporzionale.
  - Un doppio clic con il tasto sinistro o il tasto `Esc` chiude la finestra bloccata.
  - Il clic con il tasto destro apre un menu con rotazione su più angolazioni, copia del testo dell'immagine, traduzione, salvataggio con nome, copia o chiusura.

### Cattura a scorrimento
- Cattura schermate di pagine o aree lunghe tramite PipeWire screencast, una sovrapposizione di scorrimento interattiva e un motore di cucitura delle immagini.
- La funzione è pensata soprattutto per `niri` e per gli ambienti Wayland dal comportamento simile, dove geometria di output, tempi di cattura e posizione delle finestre restano più facilmente stabili.
- **Maniglia fluttuante per selezioni grandi**: quando l'area di cattura selezionata è così grande che lo spazio rimanente sullo schermo non basta a mostrare il pannello di anteprima dello scorrimento, il pannello si nasconde automaticamente e sul bordo della selezione appare una **maniglia di trascinamento fluttuante** (un pulsante flottante con frecce direzionali).
  - **Trascina per regolare la selezione**: tieni premuta e trascina la maniglia fluttuante per spostare l'area di cattura lungo l'asse di scorrimento e catturare contenuti oltre lo schermo iniziale.
  - **Clic per cambiare asse**: prima di avviare la cattura, un clic sulla maniglia fluttuante cambia direttamente la direzione di scorrimento (verticale/orizzontale).
- **Note di compatibilità**: su KDE, GNOME, X11 e in altri ambienti non basati su `niri`, la cattura a scorrimento è ancora una funzione sperimentale e imperfetta. Questi stack desktop differiscono per le politiche dei backend portal, il comportamento di shell e window manager, il feedback sulla geometria delle finestre, la temporizzazione dei fotogrammi e la gestione degli eventi di scorrimento.
- Se la cattura a scorrimento non è utilizzabile, usa il normale flusso di cattura oppure collega uno strumento esterno per catture lunghe tramite i comandi estesi di Mark Shot.
- Per segnalare un problema con la cattura a scorrimento, esegui prima `mark-shot --debug --debug-log /path/to/mark-shot.log`, riproduci il problema e allega il log alla issue su GitHub. Puoi anche attivarla tramite `debug.enabled` e `debug.logPath` in `config.json`; `DEBUG=1` e `MARK_SHOT_DEBUG_LOG=/path/to/log` restano comunque disponibili.

### Supporto multi display server
- **Wayland**: usa PipeWire portal screencast per la registrazione e la cattura a scorrimento sperimentale, gestendo sia i fotogrammi in memoria condivisa sia quelli DMA-BUF; usa `grim` per la cattura su wlroots, `layer-shell-qt` per creare sovrapposizioni native e `wl-copy` per mantenere persistenti gli appunti.
- **X11**: cattura con `QScreen::grabWindow`, una finestra a schermo intero sempre in primo piano come sovrapposizione e `xclip` per appunti persistenti.
- **Windows**: usa le API native di Qt per cattura e appunti per il flusso base di cattura, annotazione, copia, salvataggio e blocco. I backend specifici di Linux come PipeWire, xdg-desktop-portal, `grim`, il rilevamento delle finestre XCB, LayerShellQt e l'helper di GNOME Shell vengono disattivati in fase di compilazione.
- Su Linux il backend del display server viene rilevato automaticamente in fase di esecuzione tramite `$XDG_SESSION_TYPE`; su Windows viene usato il backend nativo di Qt.

### Integrazione desktop
- **Scorciatoie desktop**:
  - `mark-shot.desktop`: configurato come strumento di cattura globale di sistema, invocabile direttamente con le scorciatoie di sistema.
  - `mark-shot-edit.desktop`: registrato come editor di immagini indipendente, integrabile nel menu "Apri con" dei file manager (ad es. Dolphin, Nautilus).
- Include le icone vettoriali di sistema ad alta risoluzione `mark-shot.svg` e `mark-shot-edit.svg`.

### Autorizzazione KDE KWin ScreenShot2

Su KDE Wayland, Mark Shot può usare l'interfaccia `org.kde.KWin.ScreenShot2` di KWin per eseguire catture precise di aree. KWin considera questa interfaccia come un'interfaccia D-Bus soggetta a restrizioni, quindi il file desktop dell'applicazione deve dichiarare il campo di autorizzazione.

<details>
<summary>Autorizzazione KDE KWin ScreenShot2 e configurazione dei file desktop (fai clic per espandere)</summary>

Dichiarazione del campo di autorizzazione:
```ini
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

I pacchetti delle distribuzioni e `cmake --install` installano automaticamente i file desktop necessari. Se esegui direttamente i file di build locali senza installare il progetto, crea o aggiorna `~/.local/share/applications/mark-shot.desktop`:

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

Se hai associato Mark Shot tramite il servizio di scorciatoie dei comandi di KDE, crea anche `~/.local/share/applications/net.local.mark-shot.desktop`:

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

Dopo aver modificato i file desktop, si consiglia di disconnettersi e accedere di nuovo, così che KDE possa rileggere la cache dei file desktop. Se la sessione KDE corrente restituisce ancora `NoAuthorized`, riavvia KWin o riavvia il sistema una volta.
</details>

---

## Interfaccia a riga di comando (CLI)

### Esempi di utilizzo comune

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

#### Cattura senza interfaccia (non interattiva)

Gli script, le automazioni CI o altri programmi possono invocare `mark-shot` per catturare lo schermo senza aprire l'interfaccia di annotazione.
Il fotogramma catturato viene scritto come PNG e sullo standard output viene stampato un riepilogo JSON compatto:

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

Esempio di output JSON di `--capture-to` con un singolo display:

```json
{"path":"/tmp/shot.png","width":2560,"height":1440,"output":"DP-1","error":null}
```

Quando si specificano più `--display`, l'output diventa un array con una cattura per schermo:

```json
{"captures":[{"path":"/tmp/shots/mark-shot-DP-1-20260801-000000.png","width":2560,"height":1440,"output":"DP-1","error":null},
             {"path":"/tmp/shots/mark-shot-DP-2-20260801-000000.png","width":1920,"height":1080,"output":"DP-2","error":null}]}
```

Ogni display selezionato viene catturato usando la propria geometria sorgente, quindi i backend basati su portal restituiscono con precisione
quel display e non l'intero desktop virtuale.

La cattura senza interfaccia riutilizza tutti i backend di acquisizione dell'interfaccia interattiva (QScreen,
xdg-desktop-portal, PipeWire, grim, helper KWin/GNOME, Windows Graphics Capture),
quindi qualità dell'immagine e comportamento di ritaglio delle aree sono identici. Tutti i parametri di cattura senza interfaccia si escludono a vicenda con il parametro posizionale del file immagine.

### Descrizione dei parametri CLI

| Opzione | Descrizione |
| :--- | :--- |
| `[file]` | **Parametro posizionale**: apre un file immagine locale esistente in modalità annotazione, invece di catturare lo schermo corrente. |
| `-h`, `--help` | Mostra le informazioni di aiuto ed esce. |
| `-v`, `--version` | Mostra le informazioni sulla versione corrente ed esce. |
| `--all-outputs` | Cattura tutti gli schermi di output del desktop virtuale, invece di catturare solo lo schermo attivo corrente. |
| `--xdg-window` | Forza l'uso di una normale finestra XDG a schermo intero (xdg-shell) al posto della sovrapposizione Wayland predefinita (layer-shell). |
| `--fullscreen` | Salta la fase di selezione e annota direttamente l'intera cattura a schermo intero. |
| `--default-tool <tool>` | Specifica lo strumento di annotazione predefinito al termine di una selezione normale; viene usato anche come strumento predefinito della modalità a schermo intero quando non è impostato `--fullscreen-default-tool`. |
| `--fullscreen-default-tool <tool>` | Specifica lo strumento predefinito per la modalità di annotazione a schermo intero. |
| `--default-color <color>` | Specifica il colore di annotazione predefinito. Supporta `#RRGGBB` e `#RRGGBBAA`. |
| `--tray` | Mantiene Mark Shot in esecuzione nella barra di sistema e registra la scorciatoia globale per la cattura quando la piattaforma lo supporta. |
| `--capture` | Quando l'avvio automatico dalla barra di sistema è abilitato nella configurazione, forza l'esecuzione di una singola cattura. |
| `--pin-image <path>` | Apre direttamente un'immagine locale come finestra bloccata, saltando cattura e selezione. |
| `--recording-status` | Emette lo stato corrente della registrazione come JSON tramite l'istanza in esecuzione. |
| `--stop-recording` | Chiede all'istanza in esecuzione di interrompere la registrazione attiva corrente. |
| `--debug` | Abilita i log di debug per questa esecuzione. |
| `--no-debug` | Disabilita i log di debug per questa esecuzione, sovrascrivendo file di configurazione e variabili d'ambiente. |
| `--debug-log <path>` | Scrive i log di debug nel percorso specificato; abilita i log di debug a meno che non venga impostato anche `--no-debug`. |
| `--capture-to <path>` | Cattura senza interfaccia: scrive il PNG nel file o nella directory specificati senza aprire l'interfaccia; stampa un riepilogo JSON sullo standard output. |
| `--region <x,y,w,h>` | Da usare con `--capture-to`: cattura solo la regione logica di schermo specificata. |
| `--display <name>` | Da usare con `--capture-to`: cattura lo schermo di output specificato in base al nome del display. Può essere ripetuto per catturare più display in una sola volta (un PNG per schermo). |
| `--include-cursor` | Da usare con `--capture-to`: disegna il puntatore del mouse nel fotogramma catturato. |
| `--output-name <name>` | Da usare con `--capture-to`: nome file di base (senza estensione) usato quando il percorso di cattura è una directory. |
| `--list-displays` | Emette le informazioni su tutti i display correnti come JSON ed esce. |

### Associazioni di scorciatoie

Per associare `mark-shot` come scorciatoia di sistema per la cattura dello schermo:

**niri** (modifica `~/.config/niri/config.kdl`):
```kdl
binds {
    Mod+Shift+S { spawn "mark-shot"; }
}
```

**Hyprland** (modifica `~/.config/hypr/hyprland.conf`):
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bind = SUPER SHIFT, S, exec, mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bind = , Print, exec, mark-shot
```

**Sway / i3** (modifica `~/.config/sway/config` o `~/.config/i3/config`):
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bindsym Mod4+Shift+S exec mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bindsym Print exec mark-shot
```

**GNOME**: aggiungi in Impostazioni di sistema → Tastiera → Scorciatoie da tastiera → Scorciatoie personalizzate.

**Modalità barra di sistema**:
```powershell
mark-shot --tray
```

La modalità barra di sistema registra di default le seguenti scorciatoie globali:
- `Ctrl+Alt+S`: avvia la cattura di un'area.

Il menu della barra di sistema offre anche cattura, cattura a schermo intero, avvia registrazione, stato registrazione, interrompi registrazione, impostazioni ed esci.


### Comandi estesi

La barra degli strumenti delle azioni a destra offre il pulsante **Extensions**; il programma legge i comandi personalizzati dell'utente da `~/.config/mark-shot/extensions.json`. Il file di configurazione può essere un array JSON oppure un oggetto JSON contenente un array `commands`.

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

`command` viene eseguito tramite `$SHELL -c` sui sistemi Unix-like e tramite `%COMSPEC% /C` su Windows, quindi supporta le espressioni di shell. Usa `{slurp}` per passare al comando la selezione corrente come stringa di geometria `x,y widthxheight`. Usa `{image}` o `{imagePath}` per passare la selezione renderizzata corrente come percorso di un PNG temporaneo, oppure `{imageUrl}` per un URL `file://`. Questi segnaposto vengono automaticamente sottoposti a escaping per la shell: non aggiungere ulteriori virgolette nella configurazione. Se non usi alcun segnaposto immagine, puoi impostare `saveImage` o `needsImage` a `true` e il programma aggiungerà automaticamente il percorso del PNG temporaneo alla fine del comando. `workingDirectory` equivale a `cwd`. `closeOnStart` vale `true` di default: prima di avviare il comando, Mark Shot viene nascosto e chiuso.

### File di configurazione dell'applicazione

Consulta il [riferimento alla configurazione](../docs/configuration.zh-CN.md).

### Manuale utente

Per le operazioni quotidiane (selezione al passaggio del mouse sulle finestre, strumenti di annotazione, strumenti di avvio, finestre bloccate, catture lunghe, CLI headless
e l'elenco di autotest delle funzioni) consulta il [manuale utente](../docs/user-guide.zh-CN.md)
([English](../docs/user-guide.md)).

Altre lingue:
[简体中文](../docs/user-guide.zh-CN.md) · [繁體中文](../docs/user-guide.zh-TW.md) ·
[日本語](../docs/user-guide.ja.md) · [한국어](../docs/user-guide.ko.md) ·
[Русский](../docs/user-guide.ru.md) · [Italiano](../docs/user-guide.it.md) ·
[العربية](../docs/user-guide.ar.md) · [Français](../docs/user-guide.fr.md) ·
[Deutsch](../docs/user-guide.de.md) · [Español](../docs/user-guide.es.md) ·
[Português](../docs/user-guide.pt.md)

## Compilazione e installazione

### Guida all'installazione

##### Arch Linux (AUR)
Gli utenti Arch Linux possono installare direttamente tramite un helper AUR:
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

`mark-shot` viene compilato dai sorgenti; `mark-shot-bin` scarica da GitHub Releases il pacchetto pacman precompilato e lo installa.

##### NixOS
Gli utenti NixOS possono installare aggiungendo un Flake input
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

##### Altre distribuzioni (pacchetti precompilati)
Per le altre distribuzioni (ad es. Ubuntu, Debian, Fedora), scarica il pacchetto compilato dalla pagina Releases ed esegui i comandi seguenti per installarlo:
- **Debian / Ubuntu**:
  ```bash
  sudo apt install ./mark-shot_<version>_amd64.deb
  ```
- **Fedora**:
  ```bash
  sudo dnf install ./mark-shot-<version>-1.x86_64.rpm
  ```

> **Ubuntu 26.04 LTS**: Mark Shot è stato verificato e supportato su Ubuntu 26.04 LTS (Resolute).
> Su Ubuntu 26.04 puoi compilare dai sorgenti usando direttamente i pacchetti Qt 6.10 della distribuzione
> (senza il passaggio `aqtinstall`):
>
> ```bash
> sudo apt install build-essential cmake ninja-build pkg-config \
>   qt6-base-dev qt6-wayland libpipewire-0.3-dev libxcb-cursor0 \
>   xdg-desktop-portal pipewire xclip
> cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
> cmake --build build
> ```
>
> La cattura headless (`--capture-to`), la cattura multi-display (`--display` ripetibile) e il servizio
> MCP locale funzionano sia nelle sessioni Wayland (GNOME) sia X11 su Ubuntu 26.04.

### Dipendenze di sistema

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

> **Nota**: su sistemi come Ubuntu 22.04, che includono Qt 5, installare Qt 6 in `~/Qt` non interferisce con il sistema. In fase di compilazione basta passare `-DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64`.

#### Supporto per l'input cinese fcitx5 (Qt 6 su X11)

Qt 6 non include il plugin del metodo di input fcitx5. Per usare l'input cinese fcitx5 in ambiente X11, devi compilare il plugin dai sorgenti:

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

#### Backend OCR (opzionale)

La funzione di riconoscimento del testo di Mark Shot dipende dallo script Python integrato `mark-shot-ocr`. Lo script supporta **RapidOCR** (preferito, basato sui modelli PaddleOCR PP-OCR) e **Tesseract** (di riserva). Su Linux lo script viene installato automaticamente; su Windows va configurato manualmente.

<details>
<summary><b>Linux</b></summary>

```bash
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime
```

Al termine dell'installazione `mark-shot-ocr` viene rilevato automaticamente, senza bisogno di altra configurazione.

**Variabili d'ambiente** (opzionali):

| Variabile | Descrizione | Valore predefinito |
|------|------|--------|
| `MARK_SHOT_OCR_VERSION` | Versione PaddleOCR (`PP-OCRv5`, `PP-OCRv4`, ecc.) | `PP-OCRv5` |
| `MARK_SHOT_OCR_MODEL_TYPE` | Dimensione del modello: `mobile` o `server` | `mobile` |
| `MARK_SHOT_OCR_MODEL_DIR` | Directory personalizzata per l'archiviazione dei modelli | `~/.local/share/mark-shot/models` |
| `MARK_SHOT_OCR_NO_VENV` | Impostala a `1` per disabilitare il passaggio automatico all'ambiente virtuale | — |
| `MARK_SHOT_OCR_PYTHON` | Percorso dell'interprete Python usato per il re-exec | `~/.local/share/mark-shot/ocr-venv/bin/python` |

</details>

<details>
<summary><b>Windows</b></summary>

Gli script di supporto integrati non vengono installati automaticamente su Windows; completa manualmente i passaggi seguenti:

**1. Installa Python 3**

Scarica e installa Python 3.10 o versioni successive da [python.org](https://www.python.org/downloads/). Durante l'installazione spunta **Add python.exe to PATH**.

**2. Copia lo script OCR di supporto**

Copia `scripts/mark-shot-ocr` dal [repository di Mark Shot](https://github.com/jswysnemc/mark-shot) in una directory locale, ad esempio `%LOCALAPPDATA%\mark-shot\mark-shot-ocr.py`.

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\mark-shot"
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/tystudio-26020701/mark-shot-community/main/scripts/mark-shot-ocr" `
  -OutFile "$env:LOCALAPPDATA\mark-shot\mark-shot-ocr.py"
```

**3. Crea l'ambiente virtuale e installa le dipendenze**

```powershell
python -m venv "$env:LOCALAPPDATA\mark-shot\ocr-venv"
& "$env:LOCALAPPDATA\mark-shot\ocr-venv\Scripts\pip.exe" install -U pip rapidocr onnxruntime
```

> `onnxruntime` fornisce l'inferenza su CPU. Se hai una GPU compatibile, puoi installare `onnxruntime-directml` o `onnxruntime-gpu` per accelerare il riconoscimento.

**4. Configura `ocr.command` in `config.json`**

Apri `%LOCALAPPDATA%\mark-shot\config.json` (crealo se non esiste) e imposta `ocr.command`:

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

Sostituisci `%LOCALAPPDATA%` con il percorso effettivamente espanso (ad es. `C:\Users\你的用户名\AppData\Local`). Il segnaposto `{image}` viene sostituito in fase di esecuzione con il percorso della cattura temporanea; se lo ometti, Mark Shot lo aggiunge automaticamente.

> **Suggerimento**: imposta la variabile d'ambiente `MARK_SHOT_OCR_NO_VENV=1` per saltare il rilevamento automatico dell'ambiente virtuale integrato nello script, dato che usi già direttamente il Python dell'ambiente virtuale.

</details>

#### Backend per la scansione dei codici (opzionale)

```bash
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

L'helper per la scansione dei codici usa preferibilmente `zxing-cpp` e supporta formati comuni come QR Code, Data Matrix, Aztec, PDF417, EAN, UPC, Code 39, Code 93 e Code 128. Se `pyzbar` o OpenCV sono installati, vengono usati anche come backend di riserva.

#### Backend per il caricamento su hosting di immagini (opzionale)

Il caricamento su hosting di immagini usa di default lo script Python integrato `mark-shot-upload`, senza dipendenze aggiuntive (usa solo la libreria standard di Python 3). Lo script configura i parametri dell'hosting tramite variabili d'ambiente e supporta qualsiasi servizio compatibile con il protocollo di caricamento multipart/form-data.

<details>
<summary>Variabili d'ambiente supportate dall'helper integrato</summary>

| Variabile d'ambiente | Descrizione | Valore predefinito |
|---------|------|--------|
| `MARK_SHOT_UPLOAD_URL` | **Obbligatoria**, endpoint dell'API di caricamento dell'hosting | — |
| `MARK_SHOT_UPLOAD_FIELD` | Nome del campo del file | `image` |
| `MARK_SHOT_UPLOAD_API_KEY` | API Key / Token | — |
| `MARK_SHOT_UPLOAD_AUTH_HEADER` | Nome dell'header di autenticazione | `Authorization` |
| `MARK_SHOT_UPLOAD_AUTH_SCHEME` | Schema di autenticazione (ad es. `Bearer`); se vuoto, usa direttamente l'API Key | `Bearer` |
| `MARK_SHOT_UPLOAD_URL_PATH` | Percorso a punti dell'URL nella risposta JSON (ad es. `data.url`) | Rilevamento automatico |
| `MARK_SHOT_UPLOAD_DELETE_URL_PATH` | Percorso dell'URL di eliminazione | Rilevamento automatico |
| `MARK_SHOT_UPLOAD_HEADER_xxx` | Header di richiesta personalizzati (ad es. `MARK_SHOT_UPLOAD_HEADER_X-Custom=foo`) | — |
| `MARK_SHOT_UPLOAD_FIELD_xxx` | Campi di modulo aggiuntivi (ad es. `MARK_SHOT_UPLOAD_FIELD_album=123`) | — |

</details>

<details>
<summary>Esempio di configurazione: ImgURL V3</summary>

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

ImgURL V3 usa l'autenticazione `Authorization: Bearer <token>` (`AUTH_SCHEME` vale `Bearer` di default, nessuna modifica necessaria).

</details>

<details>
<summary>Esempio di configurazione: sm.ms</summary>

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

sm.ms usa il Token direttamente come valore di Authorization, quindi `AUTH_SCHEME` va impostato a stringa vuota.

</details>

<details>
<summary>Esempio di configurazione: imgbb</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://api.imgbb.com/1/upload?key=你的API_KEY",
    "MARK_SHOT_UPLOAD_FIELD": "image",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

imgbb passa l'API Key tramite un parametro di query nell'URL, quindi non serve impostare `API_KEY`.

</details>

<details>
<summary>Esempio di configurazione: litterbox (hosting temporaneo, senza API Key)</summary>

```json
"upload": {
  "command": "curl -sf --max-time 30 -A 'Mozilla/5.0' -F reqtype=fileupload -F time=72h -F fileToUpload=@{image} https://litterbox.catbox.moe/resources/internals/api.php",
  "timeoutMs": 35000
}
```

La risposta di litterbox è un URL in testo semplice (non JSON); Mark Shot riconosce automaticamente come risultato del caricamento l'output che inizia con `http://`/`https://`.

</details>

<details>
<summary>Comando di caricamento personalizzato</summary>

Se l'helper integrato non soddisfa le tue esigenze, puoi collegare qualsiasi script di caricamento personalizzato tramite `upload.command`. Il comando deve soddisfare i seguenti requisiti:

1. **Codice di uscita**: 0 in caso di successo; un valore diverso da zero indica un errore
2. **Formato di output** (una delle due opzioni):
   - **JSON**: `{"url":"https://...","deleteUrl":"https://...","errors":[]}` (`url` obbligatorio, gli altri opzionali)
   - **URL in testo semplice**: la prima riga non vuota dello stdout inizia con `http://` o `https://`
3. **Segnaposto**: supporta `{image}`, `{imagePath}`, `{imageUrl}`; se il comando non contiene segnaposto, Mark Shot aggiunge automaticamente il percorso dell'immagine temporanea alla fine del comando

```json
"upload": {
  "command": "/path/to/your-uploader.sh --file {image} --json",
  "timeoutMs": 30000,
  "env": {
    "UPLOADER_API_KEY": "xxx"
  }
}
```

Le variabili d'ambiente in `upload.env` vengono passate anche al comando personalizzato, così da riutilizzare facilmente la configurazione.

</details>

#### Windows

Installa Qt 6, CMake e Ninja compatibili con il compilatore in uso, oltre a un compilatore con supporto C++17, ad esempio MSVC o MinGW. La build su Windows non richiede Qt DBus, PipeWire, X11/XCB, LayerShellQt, `grim`, `wl-copy` o `xclip`.

```powershell
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
cmake --build build-windows
```

La copertura attuale su Windows riguarda la cattura normale e l'annotazione delle immagini. La cattura a scorrimento, il rilevamento delle finestre specifico del compositor e le scorciatoie desktop di Linux non sono disponibili su Windows. Gli script Python integrati (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`) non vengono installati automaticamente: consulta la sezione [Backend OCR](#ocr-后端可选), [Backend per la scansione dei codici](#扫码后端可选) e quella sulla traduzione qui sopra per la configurazione manuale.

### Build e compilazione

```bash
# 使用系统 Qt 6
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 如果 Qt 6 安装在用户目录，额外指定 CMAKE_PREFIX_PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64

# 执行编译
cmake --build build
```

Oppure usa nix

```bash
nix build
```

LayerShellQt viene rilevato automaticamente. Se presente, viene abilitato il pieno supporto Wayland layer-shell; se assente, la compilazione riesce comunque e in fase di esecuzione si ripiega automaticamente su una normale finestra a schermo intero.

### Installazione e integrazione

```bash
cmake --install build --prefix "$HOME/.local"
```

Questo comando installa l'eseguibile, gli script di supporto (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`, `mark-shot-upload`), le scorciatoie desktop e le icone.

### Estensione per la cattura a scorrimento su GNOME Wayland

La cattura a scorrimento su GNOME Wayland richiede l'attivazione dell'estensione **Mark Shot Scroll Helper**. Senza questa estensione, Mark Shot non può catturare in modo silenzioso e continuo l'area selezionata, né disegnare il pannello nativo di anteprima dello scorrimento di GNOME, quindi su GNOME Wayland il pulsante della cattura a scorrimento viene disabilitato.

I file dell'estensione si trovano nel percorso `packaging/gnome-extension/mark-shot-scroll-helper@snemc.org` del repository del progetto.

<details>
<summary><b>Espandi/comprimi la guida all'installazione e all'attivazione dell'estensione per le catture a scorrimento su GNOME Wayland</b></summary>

##### Metodo A: installazione tramite il pacchetto della distribuzione
Se hai installato Mark Shot tramite un pacchetto della distribuzione (ad es. `.deb` o `.rpm`), l'estensione è già installata di default con il sistema. Esegui il comando seguente per abilitarla per l'utente corrente:
```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```
*Se ti viene segnalato che l'estensione non viene trovata, disconnettiti e accedi di nuovo al sistema prima di riprovare.*

##### Metodo B: installazione dalla directory dei sorgenti del repository
Se hai compilato dai sorgenti o con una build manuale locale, devi prima copiare l'estensione nella cartella delle estensioni GNOME dell'utente:
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

Verifica che l'interfaccia D-Bus dell'helper sia disponibile:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
```

Il risultato atteso è `('4.2',)`. Dopo aver abilitato l'estensione, riavvia `mark-shot`.

</details>

---

## Guida a scorciatoie e gesti interattivi

### Scorciatoie per il cambio strumento

| Scorciatoia | Strumento attivato | Descrizione |
| :---: | :--- | :--- |
| **V** | Sposta / Naviga (Move / Pan) | Nella modalità immagine esistente, consente di spostare e trascinare la tela dell'immagine. |
| **S** | Seleziona (Select) | Seleziona e sposta, ridimensiona o elimina le annotazioni vettoriali già disegnate. |
| **P** | Pennello (Pen) | Disegno di curve libere. |
| **L** | Linea (Line) | Disegna linee vettoriali dritte. |
| **H** | Evidenziatore (Highlighter) | Sovrapposizione di evidenziazione semitrasparente, ideale per marcare i punti importanti. |
| **R** | Rettangolo (Rectangle) | Disegna il contorno di un rettangolo. |
| **E** | Ellisse (Ellipse) | Disegna il contorno di un'ellisse. |
| **A** | Freccia (Arrow) | Disegna la classica freccia a sei vertici, affilata e lunga, con angolo acuto. |
| **T** | Testo (Text) | Inserisci e componi testo formattato, con dimensioni fino a 1000px e collegamenti di trascinamento. |
| **N** | Numerazione (Number) | Etichette numeriche a incremento automatico per i passaggi. |
| **M** | Mosaico (Mosaic) | Offusca con effetto vetro smerigliato le aree sensibili. |
| **G** | Laser (Laser) | Tratti temporanei per didattica o presentazioni, che svaniscono dolcemente da soli. |

### Strumenti ausiliari della schermata iniziale

| Scorciatoia | Strumento | Descrizione |
| :---: | :--- | :--- |
| **C** | Selettore colore (Color Picker) | Campiona i pixel della cattura prima di selezionare l'area dello schermo. La rotellina del mouse regola la dimensione della lente d'ingrandimento; un clic con il tasto sinistro apre il pannello dei colori, da cui puoi copiare formati come HEX, RGB, HSL, HSV e Qt. Il tasto destro o Esc tornano alla selezione normale. |
| **R** | Righello (Ruler) | Misura le coordinate prima di selezionare l'area dello schermo. Al passaggio del mouse mostra il pixel corrente; trascinando con il tasto sinistro disegna un rettangolo di misura con scala in pixel, mostrando larghezza, altezza, diagonale e area. Il tasto destro o Esc tornano alla selezione normale. |
| **Q** | Scanner di codici (Code Scanner) | Entra nella modalità di scansione di codici QR e codici a barre. Dopo aver selezionato un'area, il contenuto del codice viene riconosciuto e mostrato in una finestra da cui puoi copiarlo. Il tasto destro o Esc tornano alla selezione normale. |
| **D** | Cattura display (Display Capture) | Cattura immediatamente tutti gli schermi di output, li ritaglia per display e mostra le miniature; passando il mouse su una miniatura puoi copiare, modificare o salvare la cattura di quel display. |

### Scorciatoie globali

| Scorciatoia | Azione |
| :---: | :--- |
| **Esc** | Esce immediatamente e chiude la finestra di annotazione. |
| **Ctrl + C** | Conferma tutte le modifiche al testo e copia la cattura corrente / la selezione annotata negli appunti di sistema. |
| **Ctrl + S** oppure **Invio / Return** | Conferma tutte le modifiche al testo e salva la cattura corrente. |
| **Ctrl + P** | Fissa la selezione corrente come finestra bloccata fluttuante. |
| **Ctrl + U** | Carica la cattura corrente sull'hosting di immagini personalizzato; al termine l'URL viene copiato automaticamente negli appunti. |
| **Ctrl + Z** | Annulla l'ultima operazione di annotazione. |
| **Ctrl + Y** oppure **Ctrl + Shift + Z** | Ripete l'operazione di annotazione annullata. |
| **Backspace** oppure **Delete** | Quando lo strumento **Seleziona (Select)** è attivo e un'annotazione è selezionata, elimina l'annotazione selezionata. |
| **F** | Alterna l'area coperta dalla cattura corrente (passa tra modalità selezione e modalità a schermo intero). |

### Tecniche di interazione avanzate

- **Vincoli per le forme**: mentre disegni un **rettangolo (Rectangle)** o un'**ellisse (Ellipse)**, tieni premuto `Ctrl` per forzare rispettivamente quadrato o cerchio perfetto.
- **Passaggio rapido allo strumento di selezione**: durante l'annotazione, un clic con il tasto destro su un'area vuota della tela passa immediatamente allo strumento **Seleziona (Select)**.
- **Doppio clic con il tasto destro per cambiare colore**: un doppio clic con il tasto destro su un'area vuota della tela apre la ruota dei colori per cambiare rapidamente il colore dello strumento di annotazione corrente.
- **Regolazione continua con la rotellina**: con lo strumento di annotazione corrispondente attivo, la rotellina del mouse regola in tempo reale lo spessore della linea, la dimensione del carattere, la dimensione delle etichette numeriche o la dimensione della griglia del mosaico.
- **Spostamento e zoom della tela**: nella modalità dello strumento **Seleziona (Select)**, oppure durante la modifica di un file locale, la rotellina del mouse esegue uno zoom continuo sulla tela; trascinando con il tasto centrale del mouse puoi spostarla. Un doppio clic su `Ctrl` ripristina zoom e spostamento.

### Interazioni specifiche della finestra bloccata

| Gesto / Scorciatoia | Effetto |
| :--- | :--- |
| **Tieni premuto e trascina con il tasto sinistro del mouse** | Sposta e posiziona liberamente la finestra bloccata sul desktop. |
| **Rotellina del mouse verso l'alto/il basso** | Ingrandisce/riduce la finestra bloccata in modo proporzionale e continuo. |
| **Doppio clic con il tasto sinistro del mouse** | Chiude rapidamente la finestra bloccata. |
| **Clic con il tasto destro del mouse** | Apre il menu delle funzioni (rotazione, copia del testo dell'immagine, traduzione, salvataggio, copia, chiusura, ecc.). |
| **Tasto Esc** | Chiude la finestra bloccata che ha attualmente il focus. |

---

## Note di rilascio

Consulta le [note di rilascio](../docs/releases.zh-CN.md).

## Feedback e comunicazione

### Segnalare un problema
Se riscontri un problema durante l'utilizzo o hai suggerimenti per nuove funzionalità, ti consigliamo di segnalare il problema tramite la CLI di GitHub (`gh`). Mettiamo a disposizione uno script che raccoglie automaticamente le informazioni sull'ambiente: consulta la [guida alla segnalazione dei problemi](../.doc/submit-issue-via-gh.md).

---

## Informazioni sulla licenza

Questo progetto è open source sotto **licenza MIT**; per i dettagli consulta il file [LICENSE](../LICENSE).

## Ringraziamenti

Mark Shot si basa sulle spalle della comunità open source, a cui rivolgiamo il nostro sincero ringraziamento:

- **Il progetto upstream originale [jswysnemc/mark-shot](https://github.com/jswysnemc/mark-shot), insieme ai suoi autori e a tutti i contributori.** Questa edizione community è sviluppata a partire dal progetto upstream originale; il suo design eccellente e i contributi continui sono la base di tutto questo, e ringraziamo di cuore il loro ottimo lavoro.
- **[serendipitywgy](https://github.com/serendipitywgy)**: grazie per aver contribuito tramite `serendipitywgy/mark-shot` con miglioramenti alla compatibilità tra desktop, l'azione della barra degli strumenti per la copia del testo OCR e la funzione di preselezione intelligente dei riquadri.
- **Tutti i progetti open source da cui Mark Shot dipende**, tra cui Qt 6, PipeWire, xdg-desktop-portal, layer-shell-qt, wl-clipboard, xclip, grim, RapidOCR, onnxruntime, Tesseract, ZXing-C++ e altri.

Questa edizione community è mantenuta da [Beijing Taiyin Zhaowu Technology Co., Ltd.](https://github.com/tystudio-26020701/mark-shot-community) e dai contributori, ed è open source sotto **licenza MIT**.
