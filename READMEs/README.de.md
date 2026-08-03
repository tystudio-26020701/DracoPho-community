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

Dieses README in anderen Sprachen lesen：
[简体中文](../README.zh-CN.md) · [繁體中文](./README.zh-TW.md) ·
[日本語](./README.ja.md) · [한국어](./README.ko.md) ·
[Русский](./README.ru.md) · [Italiano](./README.it.md) ·
[العربية](./README.ar.md) · [Français](./README.fr.md) ·
[Deutsch](./README.de.md) · [Español](./README.es.md) ·
[Português](./README.pt.md)

**Stichwörter**: `C++` / `Qt 6` / `屏幕截图` / `图像标注` / `桌面贴图` / `OCR 识别` / `滚动长截图` / `Wayland` / `Windows`


<details>
<summary>Demonstrationsvideo</summary>
<p align="center">
  <video src="https://github.com/user-attachments/assets/4f86fcee-fef9-409e-98ba-1491ecee06c7" width="100%" controls></video>
</p>
</details>

`mark-shot` ist ein leistungsstarkes Screenshot- und Annotationstool auf Basis von Qt 6. Das Projekt wurde ursprünglich für Wayland-Fenstermanager wie `niri` entworfen und unterstützt derzeit gewöhnliche Screenshot- und Annotations-Workflows unter Linux (X11, GNOME, wlroots/Wayland-Desktops) sowie Windows.

Es kann in Sekundenschnelle den Bildschirm erfassen und eine adaptive Vollbild-Annotationsüberlagerung öffnen, die Funktionen wie Bereichsauswahl, Annotation, Kopieren in die Zwischenablage, Speichern und Pin-auf-Desktop bereitstellt.

---

## Kernfunktionen

### Annotation-Werkzeugkasten
- **Stift und Textmarker**: Unterstützt glattes Freihand-Zeichnen und halbtransparente Hervorhebungsüberlagerungen.
- **Geometrische Vektorwerkzeuge**: Hochpräzise Pfade für Linien, Rechtecke und Ellipsen. Das Rechteck unterstützt drei umschaltbare Stile:
  - `描边`: Das ursprüngliche umrandete oder gefüllte Rechteck, optional mit abgerundeten Ecken.
  - `高亮`: Textmarkerartiger Überlagerungseffekt, realisiert mit `CompositionMode_Multiply` und halbtransparenter Füllung.
  - `反色`: Invertiert die RGB-Werte der Pixel innerhalb des überdeckten Rechteckbereichs und behält dabei die Außenkontur als visuellen Hinweis.
- **Optimierter Pfeil**: Verwendet den klassischen Sechs-Eckpunkte-Pfeilpfad mit glatten Kanten und Anti-Aliasing-Rendering.
- **Doppelt verknüpfter Text**:
  - Unterstützt stufenlose Anpassung bis hin zu sehr großen Schriftgrößen, die per Mausrad oder Attributregler sanft skaliert werden können.
  - Führt ein Pufferdesign mit physischer Breite ein, um unerwünschte Zeilenumbrüche bei extrem hohen Zoomstufen infolge von Rendering-Jitter zu vermeiden.
  - Über den **diagonalen Kontrollpunkt** können Schriftgröße und Textrahmen proportional miteinander skaliert werden; die **seitlichen Kontrolllinien** passen lediglich die Breite des Umbruchbereichs an.
- **Laser-Präsentationsstift**: Geeignet für Präsentationen oder Unterricht; die Striche verblassen mit der Zeit sanft und verschwinden.
- **Automatisch nummerierte Schritte**: Mit einem Klick werden fortlaufend ansteigende nummerierte Schrittmarkierungen platziert.
- **Mosaik**: Ermöglicht das Unkenntlichmachen sensibler Informationen durch regionale Unschärfe (Milchglas-Effekt).
- **Lupe mit zwei unabhängig einstellbaren Rahmen**: Der innere Bildausschnitt und die äußere Linse der Lupe besitzen jeweils eigene Größenanfasser; rechteckige Linsen haben 8 Eck-/Kantenanfasser pro Rahmen, runde Linsen je 4 Anfasser oben, unten, links und rechts. Wird einer der Rahmen angepasst, folgt der andere proportional zum Zoomfaktor, während die Vergrößerung stets konstant bleibt; wird ein Rahmen verschoben, bleibt der andere an seiner Position.
- **Code-Scannen beim Start**: Drücke vor der Auswahl `Q`, um in den Scan-Modus zu wechseln. Nachdem eine QR-Code- oder Barcode-Region ausgewählt wurde, öffnet sich ein Fenster mit kopierbarem Erkennungsergebnis.
- **Schneller Display-Screenshot**: Drücke vor der Auswahl `D`, um sofort alle Ausgabebildschirme zu erfassen und sie pro Display in Miniaturansichten zu zerlegen; durch Überfahren einer Miniaturansicht mit der Maus kann der jeweilige Display-Screenshot kopiert, bearbeitet oder gespeichert werden.
- **GIF- und Videoaufnahme**: Über die Aufnahme-Tastenkombination beim Start oder das Tray-Menü kann ein bestimmter Monitor oder ein benutzerdefinierter Bereich als GIF oder MP4 aufgenommen werden. Eine laufende Aufnahme wird im Tray und im eingefrorenen Frame angezeigt und kann mit `S`, den Überlagerungs-Buttons, dem Tray-Menü oder `--stop-recording` gestoppt werden; beim Start und Speichern werden Desktop-Benachrichtigungen gesendet. Unter Wayland wird bevorzugt der PipeWire-Portal-Backend verwendet; wenn die Portal-Erfassung nicht verfügbar ist, wird auf wlroots-Screencopy oder Polling-Erfassung zurückgegriffen.
- **Bild-Hosting-Upload**: Drücke nach der Auswahl `Ctrl+U` oder klicke auf die Upload-Schaltfläche in der Symbolleiste, um den aktuellen Screenshot auf ein benutzerdefiniertes Bild-Hosting hochzuladen (z. B. ImgURL, sm.ms, imgbb, litterbox usw.). Nach erfolgreichem Upload wird die URL automatisch in die Zwischenablage kopiert. Die Bild-Hosting-Parameter können über `upload.env` konfiguriert oder über `upload.command` an beliebige eigene Upload-Skripte übergeben werden.
- **Exportrahmen im Mac-Stil**: Fügt gespeicherten, kopierten, hochgeladenen, über „Öffnen mit" geöffneten und über Erweiterungsbefehle erzeugten Bildern transparente Ränder, abgerundete Ecken und weiche Schatten hinzu.

### Pin – schwebende Desktop-Fixierung
- Unterstützt das Anheften eines Screenshots oder eines annotierten Bereichs als eigenständiges, rahmenloses, stets im Vordergrund liegendes Pin-Fenster auf dem Bildschirm.
- Unterstützt das direkte Auswählen von per OCR erkanntem Text im Pin-Fenster; mit `Ctrl + C` oder dem Rechtsklick-Menü kann Bildtext kopiert werden.
- Unterstützt das Übersetzen von OCR-Text über eine LLM-Schnittstelle mit OpenAI-kompatiblem API, wobei die Übersetzung positionsgetreu an der ursprünglichen Stelle auf dem Pin-Bild gerendert wird.
- **Praktische Interaktion**:
  - Mit gedrückter linker Maustaste kann das Pin-Fenster frei verschoben werden.
  - Das Mausrad skaliert das Pin-Bild proportional.
  - Ein Doppelklick mit der linken Maustaste oder das Drücken von `Esc` schließt das Pin-Fenster.
  - Ein Rechtsklick öffnet ein Menü mit Optionen wie Drehen in verschiedenen Winkeln, Bildtext kopieren, Übersetzen, Speichern unter, Kopieren oder Schließen.

### Scroll-Screenshots
- Erfasst lange Seiten oder große Bereiche über PipeWire-Screencast, eine interaktive Scroll-Überlagerung und einen Bild-Stitcher.
- Diese Funktion richtet sich in erster Linie an `niri` und ähnlich verhaltende Wayland-Umgebungen; in diesen Umgebungen bleiben Ausgabegeometrie, Erfassungs-Timing und Fensterpositionen leichter stabil.
- **Schwebender Griff bei großen Auswahlbereichen**: Wenn der gewählte Screenshot-Bereich so groß ist, dass auf dem restlichen Bildschirm kein Platz für das Scroll-Vorschaufenster bleibt, wird das Vorschaufenster automatisch ausgeblendet und am Rand des Auswahlbereichs ein **schwebender Ziehgriff** (ein schwebender Button mit Richtungspfeilen) angezeigt.
  - **Auswahl per Ziehen anpassen**: Der schwebende Griff kann gedrückt und gezogen werden, um den Screenshot-Auswahlbereich entlang der Scroll-Achse zu verschieben und so Inhalte zu erfassen, die über den anfänglichen Bildschirmbereich hinausgehen;
  - **Achsenrichtung per Klick wechseln**: Vor Beginn der Erfassung kann durch Klicken auf den schwebenden Griff direkt die Scroll-Richtung (vertikal/horizontal) umgeschaltet werden.
- **Kompatibilitätshinweis**: Scroll-Screenshots unter KDE, GNOME, X11 und anderen Umgebungen ohne `niri` sind weiterhin experimentell und noch nicht ausgereift. Diese Desktop-Stacks unterscheiden sich in den Portal-Backend-Strategien, im Verhalten von Shell bzw. Fenstermanager, beim Feedback der Fenstergeometrie, beim Frame-Timing und bei der Verarbeitung von Scroll-Ereignissen.
- Falls Scroll-Screenshots nicht funktionieren, verwende bitte den normalen Screenshot-Ablauf oder binde über Mark-Shot-Erweiterungsbefehle ein externes Lang-Screenshot-Werkzeug an.
- Um ein Problem mit Scroll-Screenshots zu melden, führe bitte zuerst `mark-shot --debug --debug-log /path/to/mark-shot.log` aus, reproduziere das Problem und hänge das Protokoll an das GitHub-Issue an. Alternativ kann das Debugging in `config.json` über `debug.enabled` und `debug.logPath` aktiviert werden; `DEBUG=1` und `MARK_SHOT_DEBUG_LOG=/path/to/log` funktionieren weiterhin.

### Unterstützung verschiedener Display-Server
- **Wayland**: Nutzt PipeWire-Portal-Screencast für Aufnahmen und experimentelle Scroll-Screenshots und verarbeitet sowohl Shared-Memory- als auch DMA-BUF-Frame-Pfade; verwendet `grim` für wlroots-Screenshots, `layer-shell-qt` für native Überlagerungen und `wl-copy` für die dauerhafte Zwischenablage.
- **X11**: Nutzt `QScreen::grabWindow` für Screenshots, ein Vollbild-Immer-im-Vordergrund-Fenster als Überlagerung und `xclip` für die dauerhafte Zwischenablage.
- **Windows**: Unterstützt grundlegende Screenshot-, Annotations-, Kopier-, Speicher- und Pin-Workflows über die nativen Qt-Screenshot- und Zwischenablage-APIs. Linux-spezifische Backends wie PipeWire, xdg-desktop-portal, `grim`, XCB-Fenstererkennung, LayerShellQt und der GNOME-Shell-Helper werden zur Kompilierzeit deaktiviert.
- Der Linux-Display-Server-Backend wird zur Laufzeit automatisch über `$XDG_SESSION_TYPE` erkannt; Windows verwendet den nativen Qt-Plattform-Backend.
- **Multi-monitor freeze scope**: standardmäßig friert die Bereichsauswahl alle angeschlossenen Monitore ein (ein einzelnes virtuelles Desktop-Fenster, wenn die DPRs unter X11/Windows übereinstimmen); nach dem Bestätigen einer Auswahl auf einem Monitor bleiben die übrigen Monitore bis zum Ende der Sitzung eingefroren und nicht interaktiv. Der **Cursor Screen**-Bereich friert nur den Monitor unter dem Mauszeiger ein.

### Desktop-Integration
- **Desktop-Verknüpfungen**:
  - `mark-shot.desktop`: Als systemweites Screenshot-Werkzeug konfiguriert, das direkt über Systemtastenkürzel aufgerufen werden kann.
  - `mark-shot-edit.desktop`: Als eigenständiger Bildeditor registriert und in das Rechtsklick-Menü „Öffnen mit" von Dateimanagern (z. B. Dolphin, Nautilus) integrierbar.
- Enthält hochauflösende System-Vektor-Icons `mark-shot.svg` und `mark-shot-edit.svg`.

### KDE-KWin-ScreenShot2-Autorisierung

Unter KDE Wayland kann Mark Shot über die Schnittstelle `org.kde.KWin.ScreenShot2` von KWin präzise Bereichs-Screenshots ausführen. KWin behandelt diese Schnittstelle als eingeschränkte D-Bus-Schnittstelle, daher muss die zugehörige Desktop-Datei der Anwendung das Autorisierungsfeld deklarieren.

<details>
<summary>KDE-KWin-ScreenShot2-Autorisierung und Konfiguration der Desktop-Datei (zum Aufklappen klicken)</summary>

Deklariere das Autorisierungsfeld:
```ini
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

Distributionspakete und `cmake --install` installieren die benötigten Desktop-Dateien automatisch. Wenn du lokale Build-Artefakte ausführst, ohne das Projekt zu installieren, erstelle oder aktualisiere `~/.local/share/applications/mark-shot.desktop`:

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

Wenn Mark Shot über den KDE-Befehlstastenkürzel-Dienst gebunden wird, muss zusätzlich `~/.local/share/applications/net.local.mark-shot.desktop` erstellt werden:

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

Nach dem Ändern der Desktop-Datei empfiehlt es sich, sich ab- und wieder anzumelden, damit KDE den Desktop-Datei-Cache neu einliest. Wenn die aktuelle KDE-Sitzung weiterhin `NoAuthorized` zurückgibt, starte KWin oder das System einmal neu.
</details>

---

## Befehlszeilenschnittstelle (CLI)

### Häufige Verwendungsbeispiele

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

#### Headless (nicht-interaktive) Screenshots

Skripte, CI-Automatisierung oder andere Programme können `mark-shot` aufrufen, um Screenshots zu erstellen, ohne die Annotationsoberfläche zu öffnen.
Der erfasste Frame wird als PNG geschrieben und eine kompakte JSON-Zusammenfassung auf der Standardausgabe ausgegeben:

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

Beispiel für die JSON-Ausgabe von `--capture-to` mit einem einzelnen Monitor:

```json
{"path":"/tmp/shot.png","width":2560,"height":1440,"output":"DP-1","error":null}
```

Wenn mehrere `--display` angegeben werden, wird die Ausgabe zu einem Array mit einer Erfassung pro Monitor:

```json
{"captures":[{"path":"/tmp/shots/mark-shot-DP-1-20260801-000000.png","width":2560,"height":1440,"output":"DP-1","error":null},
             {"path":"/tmp/shots/mark-shot-DP-2-20260801-000000.png","width":1920,"height":1080,"output":"DP-2","error":null}]}
```

Jeder ausgewählte Monitor wird mit seiner eigenen Quellgeometrie erfasst, sodass Portal-artige Backends exakt
diesen Monitor und nicht den gesamten virtuellen Desktop zurückliefern.

Headless-Screenshots verwenden dieselben Erfassungs-Backends wie die interaktive Oberfläche (QScreen,
xdg-desktop-portal, PipeWire, grim, KWin/GNOME-Helfer, Windows Graphics Capture),
daher sind Bildqualität und Bereichsauswahlverhalten vollständig identisch. Alle Headless-Parameter schließen
sich mit dem Positionsparameter für Bilddateien gegenseitig aus.

### CLI-Parameterübersicht

| Parameter | Funktionsbeschreibung |
| :--- | :--- |
| `[file]` | **Positionsparameter**: Öffnet eine vorhandene lokale Bilddatei im Annotationsmodus, anstatt den aktuellen Bildschirm zu erfassen. |
| `-h`, `--help` | Zeigt die Hilfeinformationen an und beendet das Programm. |
| `-v`, `--version` | Zeigt die aktuelle Versionsinformation an und beendet das Programm. |
| `--all-outputs` | Erfasst alle Ausgabebildschirme des virtuellen Desktops anstatt nur den aktuell aktiven Bildschirm. |
| `--xdg-window` | Erzwingt ein standardmäßiges XDG-Vollbildfenster (xdg-shell) anstelle der standardmäßigen Wayland-Überlagerung (layer-shell). |
| `--fullscreen` | Überspringt den Auswahlschritt und annotiert direkt den vollständig erfassten Screenshot. |
| `--default-tool <tool>` | Legt das Standard-Annotationswerkzeug nach Abschluss einer normalen Auswahl fest; sofern `--fullscreen-default-tool` nicht gesetzt ist, dient es auch als Standardwerkzeug für den Vollbildmodus. |
| `--fullscreen-default-tool <tool>` | Legt das Standardwerkzeug für den Vollbild-Annotationsmodus fest. |
| `--default-color <color>` | Legt die Standard-Annotationsfarbe fest. Unterstützt `#RRGGBB` und `#RRGGBBAA`. |
| `--tray` | Hält Mark Shot im System-Tray am Laufen und registriert, sofern von der Plattform unterstützt, ein globales Screenshot-Tastenkürzel. |
| `--capture` | Löst einen einzelnen Screenshot aus, wenn der Tray-Autostart in der Konfiguration aktiviert ist. |
| `--pin-image <path>` | Öffnet eine lokale Bilddatei direkt als Pin-Fenster und überspringt Screenshot und Auswahl. |
| `--recording-status` | Gibt über eine laufende Instanz den aktuellen Aufnahmestatus als JSON aus. |
| `--stop-recording` | Fordert eine laufende Instanz auf, die aktuelle Aufnahme zu stoppen. |
| `--debug` | Aktiviert die Debug-Protokollierung für diesen Lauf. |
| `--no-debug` | Deaktiviert die Debug-Protokollierung für diesen Lauf und überschreibt Konfigurationsdatei und Umgebungsvariablen. |
| `--debug-log <path>` | Schreibt das Debug-Protokoll in den angegebenen Pfad; aktiviert die Debug-Protokollierung, sofern nicht gleichzeitig `--no-debug` gesetzt ist. |
| `--capture-to <path>` | Headless-Screenshot: Schreibt ein PNG in die angegebene Datei oder das angegebene Verzeichnis, ohne die Oberfläche zu öffnen; gibt eine JSON-Zusammenfassung auf der Standardausgabe aus. |
| `--region <x,y,w,h>` | In Kombination mit `--capture-to`: erfasst nur den angegebenen logischen Bildschirmbereich. |
| `--display <name>` | In Kombination mit `--capture-to`: erfasst den angegebenen Ausgabebildschirm anhand des Display-Namens. Kann mehrfach angegeben werden, um mehrere Monitore in einem Durchgang zu erfassen (ein PNG pro Monitor). |
| `--include-cursor` | In Kombination mit `--capture-to`: zeichnet den Mauszeiger in den erfassten Frame. |
| `--output-name <name>` | In Kombination mit `--capture-to`: Basisdateiname (ohne Erweiterung), der verwendet wird, wenn der Erfassungspfad ein Verzeichnis ist. |
| `--list-displays` | Gibt Informationen zu allen aktuellen Monitoren als JSON aus und beendet das Programm. |

### Tastenkürzel-Bindung

`mark-shot` als systemweites Screenshot-Tastenkürzel binden:

**niri** (`~/.config/niri/config.kdl` bearbeiten):
```kdl
binds {
    Mod+Shift+S { spawn "mark-shot"; }
}
```

**Hyprland** (`~/.config/hypr/hyprland.conf` bearbeiten):
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bind = SUPER SHIFT, S, exec, mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bind = , Print, exec, mark-shot
```

**Sway / i3** (`~/.config/sway/config` oder `~/.config/i3/config` bearbeiten):
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bindsym Mod4+Shift+S exec mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bindsym Print exec mark-shot
```

**GNOME**: Unter Systemeinstellungen → Tastatur → Tastenkürzel → Benutzerdefinierte Tastenkürzel hinzufügen.

**Tray-Modus**:
```powershell
mark-shot --tray
```

Der Tray-Modus registriert standardmäßig folgende globale Tastenkürzel:
- `Ctrl+Alt+S`: Startet einen Bereichs-Screenshot.

Das Tray-Menü bietet darüber hinaus Aktionen wie Screenshot, Vollbild-Screenshot, Aufnahme starten, Aufnahmestatus, Aufnahme stoppen, Einstellungen und Beenden.


### Erweiterungsbefehle

Die Aktionssymbolleiste auf der rechten Seite stellt einen **Extensions**-Button bereit. Das Programm liest benutzerdefinierte Befehle aus `~/.config/mark-shot/extensions.json`. Die Konfigurationsdatei kann ein JSON-Array oder ein JSON-Objekt sein, das ein `commands`-Array enthält.

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

`command` wird auf Unix-ähnlichen Systemen über `$SHELL -c` und unter Windows über `%COMSPEC% /C` ausgeführt und unterstützt daher Shell-Ausdrücke. Mit `{slurp}` kann die aktuelle Auswahl als Geometriezeichenfolge `x,y widthxheight` an den Befehl übergeben werden. Mit `{image}` oder `{imagePath}` kann der aktuell gerenderte Auswahlbereich als temporärer PNG-Pfad übergeben werden, mit `{imageUrl}` eine `file://`-URL. Diese Platzhalter werden automatisch mit Shell-Quoting versehen; in der Konfiguration müssen keine zusätzlichen Anführungszeichen gesetzt werden. Wenn kein Bildplatzhalter verwendet wird, kann `saveImage` oder `needsImage` auf `true` gesetzt werden; das Programm hängt den temporären PNG-Pfad dann automatisch an das Befehlsende an. `workingDirectory` ist äquivalent zu `cwd`. Der Standardwert von `closeOnStart` ist `true`; Mark Shot wird vor dem Start des Befehls ausgeblendet und geschlossen.

### Anwendungskonfigurationsdatei

Siehe die [Konfigurationsreferenz](../docs/configuration.zh-CN.md).

### Benutzerhandbuch

Für den täglichen Gebrauch (Auswahl über Fensterüberlagerung, Annotationswerkzeuge, Startwerkzeuge,
Pin-Fenster, lange Screenshots, headless CLI sowie die Funktions-Selbsttestliste) siehe das
[Benutzerhandbuch](../docs/user-guide.zh-CN.md) ([English](../docs/user-guide.md)).

Andere Sprachversionen:
[简体中文](../docs/user-guide.zh-CN.md) · [繁體中文](../docs/user-guide.zh-TW.md) ·
[日本語](../docs/user-guide.ja.md) · [한국어](../docs/user-guide.ko.md) ·
[Русский](../docs/user-guide.ru.md) · [Italiano](../docs/user-guide.it.md) ·
[العربية](../docs/user-guide.ar.md) · [Français](../docs/user-guide.fr.md) ·
[Deutsch](../docs/user-guide.de.md) · [Español](../docs/user-guide.es.md) ·
[Português](../docs/user-guide.pt.md)

## Kompilieren und Installieren

### Installationsanleitung

##### Arch Linux (AUR)
Arch-Linux-Benutzer können das Paket direkt über einen AUR-Helfer installieren:
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

`mark-shot` wird aus dem Quellcode kompiliert; `mark-shot-bin` wird als vorkompiliertes pacman-Paket von den GitHub-Releases heruntergeladen und installiert.

##### NixOS
NixOS-Benutzer können installieren, indem sie einen Flake-Input hinzufügen
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

##### Andere Distributionen (vorkompilierte Pakete)
Für andere Distributionen (z. B. Ubuntu, Debian, Fedora) lade das vorkompilierte Installationspaket von der Releases-Seite herunter und installiere es mit einem der folgenden Befehle:
- **Debian / Ubuntu**:
  ```bash
  sudo apt install ./mark-shot_<version>_amd64.deb
  ```
- **Fedora**:
  ```bash
  sudo dnf install ./mark-shot-<version>-1.x86_64.rpm
  ```

> **Ubuntu 26.04 LTS**: Mark Shot wurde auf Ubuntu 26.04 LTS (Resolute) verifiziert und wird unterstützt.
> Beim Bauen aus dem Quellcode unter Ubuntu 26.04 können direkt die in der Distribution enthaltenen
> Qt-6.10-Pakete verwendet werden (der `aqtinstall`-Schritt entfällt):
>
> ```bash
> sudo apt install build-essential cmake ninja-build pkg-config \
>   qt6-base-dev qt6-wayland libpipewire-0.3-dev libxcb-cursor0 \
>   xdg-desktop-portal pipewire xclip
> cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
> cmake --build build
> ```
>
> Headless-Screenshots (`--capture-to`), Multi-Monitor-Screenshots (wiederholbares `--display`) sowie der
> lokale MCP-Dienst funktionieren in Ubuntu-26.04-Sitzungen unter Wayland (GNOME) und X11.

### Systemabhängigkeiten

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

> **Hinweis**: In Umgebungen wie Ubuntu 22.04, die bereits Qt 5 mitbringen, beeinträchtigt die Installation von Qt 6 nach `~/Qt` das System nicht. Beim Kompilieren einfach `-DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64` übergeben.

#### fcitx5-Unterstützung für chinesische Eingabe (Qt 6 unter X11)

Qt 6 enthält kein fcitx5-IME-Plugin. Um fcitx5 für die chinesische Eingabe unter X11 zu verwenden, muss das Plugin aus dem Quellcode kompiliert werden:

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

#### OCR-Backend (optional)

Die Texterkennungsfunktion von Mark Shot hängt von dem integrierten Python-Skript `mark-shot-ocr` ab. Das Skript unterstützt **RapidOCR** (bevorzugt, basierend auf den PaddleOCR-PP-OCR-Modellen) und **Tesseract** (Fallback). Unter Linux wird das Skript automatisch installiert; unter Windows muss es manuell konfiguriert werden.

<details>
<summary><b>Linux</b></summary>

```bash
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime
```

Nach der Installation wird `mark-shot-ocr` automatisch gefunden; eine zusätzliche Konfiguration ist nicht erforderlich.

**Umgebungsvariablen** (optional):

| Variable | Beschreibung | Standardwert |
|------|------|--------|
| `MARK_SHOT_OCR_VERSION` | PaddleOCR-Version (z. B. `PP-OCRv5`, `PP-OCRv4`) | `PP-OCRv5` |
| `MARK_SHOT_OCR_MODEL_TYPE` | Modellgröße: `mobile` oder `server` | `mobile` |
| `MARK_SHOT_OCR_MODEL_DIR` | Benutzerdefiniertes Modellverzeichnis | `~/.local/share/mark-shot/models` |
| `MARK_SHOT_OCR_NO_VENV` | Auf `1` setzen, um die automatische Aktivierung der virtuellen Umgebung zu deaktivieren | — |
| `MARK_SHOT_OCR_PYTHON` | Pfad des Python-Interpreters, der für den Re-Exec verwendet wird | `~/.local/share/mark-shot/ocr-venv/bin/python` |

</details>

<details>
<summary><b>Windows</b></summary>

Das integrierte Hilfsskript wird unter Windows nicht automatisch installiert; die folgenden Schritte müssen manuell ausgeführt werden:

**1. Python 3 installieren**

Lade Python 3.10 oder neuer von [python.org](https://www.python.org/downloads/) herunter und installiere es. Aktiviere bei der Installation die Option **Add python.exe to PATH**.

**2. Das OCR-Hilfsskript kopieren**

Kopiere `../scripts/mark-shot-ocr` aus dem [Mark-Shot-Repository](https://github.com/jswysnemc/mark-shot) in ein lokales Verzeichnis, z. B. `%LOCALAPPDATA%\mark-shot\mark-shot-ocr.py`.

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\mark-shot"
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/tystudio-26020701/mark-shot-community/main/scripts/mark-shot-ocr" `
  -OutFile "$env:LOCALAPPDATA\mark-shot\mark-shot-ocr.py"
```

**3. Eine virtuelle Umgebung erstellen und Abhängigkeiten installieren**

```powershell
python -m venv "$env:LOCALAPPDATA\mark-shot\ocr-venv"
& "$env:LOCALAPPDATA\mark-shot\ocr-venv\Scripts\pip.exe" install -U pip rapidocr onnxruntime
```

> `onnxruntime` bietet CPU-Inferenz. Wenn eine kompatible GPU vorhanden ist, können `onnxruntime-directml` oder `onnxruntime-gpu` installiert werden, um die Erkennung zu beschleunigen.

**4. `ocr.command` in `config.json` konfigurieren**

Öffne `%LOCALAPPDATA%\mark-shot\config.json` (lege sie bei Bedarf neu an) und setze `ocr.command`:

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

Ersetze `%LOCALAPPDATA%` durch den tatsächlich expandierten Pfad (z. B. `C:\Users\你的用户名\AppData\Local`). Der Platzhalter `{image}` wird zur Laufzeit durch den temporären Screenshot-Pfad ersetzt; wenn er weggelassen wird, hängt Mark Shot ihn automatisch an.

> **Tipp**: Das Setzen der Umgebungsvariablen `MARK_SHOT_OCR_NO_VENV=1` überspringt die automatische Erkennung der virtuellen Umgebung im Skript, da bereits direkt das Python aus der virtuellen Umgebung verwendet wird.

</details>

#### Code-Scan-Backend (optional)

```bash
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

Der Scan-Helper verwendet bevorzugt `zxing-cpp` und unterstützt gängige Formate wie QR Code, Data Matrix, Aztec, PDF417, EAN, UPC, Code 39, Code 93 und Code 128. Falls `pyzbar` oder OpenCV installiert sind, werden diese als Fallback-Backend verwendet.

#### Bild-Hosting-Upload-Backend (optional)

Die Bild-Hosting-Upload-Funktion verwendet standardmäßig das integrierte Python-Skript `mark-shot-upload`, das keine zusätzlichen Abhängigkeiten erfordert (es nutzt nur die Python-3-Standardbibliothek). Das Skript wird über Umgebungsvariablen konfiguriert und unterstützt beliebige Bild-Hosting-Dienste, die mit dem multipart/form-data-Upload-Protokoll kompatibel sind.

<details>
<summary>Von den integrierten Helfern unterstützte Umgebungsvariablen</summary>

| Umgebungsvariable | Beschreibung | Standardwert |
|---------|------|--------|
| `MARK_SHOT_UPLOAD_URL` | **Pflichtfeld**: Endpoint der Bild-Hosting-Upload-Schnittstelle | — |
| `MARK_SHOT_UPLOAD_FIELD` | Name des Dateifelds | `image` |
| `MARK_SHOT_UPLOAD_API_KEY` | API-Schlüssel / Token | — |
| `MARK_SHOT_UPLOAD_AUTH_HEADER` | Name des Authentifizierungsheaders | `Authorization` |
| `MARK_SHOT_UPLOAD_AUTH_SCHEME` | Authentifizierungsschema (z. B. `Bearer`); bei leerem Wert wird der API-Schlüssel direkt verwendet | `Bearer` |
| `MARK_SHOT_UPLOAD_URL_PATH` | Gepunkteter Pfad der URL in der JSON-Antwort (z. B. `data.url`) | Automatische Erkennung |
| `MARK_SHOT_UPLOAD_DELETE_URL_PATH` | Pfad der Lösch-URL | Automatische Erkennung |
| `MARK_SHOT_UPLOAD_HEADER_xxx` | Benutzerdefinierte Anfrage-Header (z. B. `MARK_SHOT_UPLOAD_HEADER_X-Custom=foo`) | — |
| `MARK_SHOT_UPLOAD_FIELD_xxx` | Zusätzliche Formularfelder (z. B. `MARK_SHOT_UPLOAD_FIELD_album=123`) | — |

</details>

<details>
<summary>Konfigurationsbeispiel: ImgURL V3</summary>

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

ImgURL V3 verwendet die Authentifizierung `Authorization: Bearer <token>` (`AUTH_SCHEME` ist standardmäßig `Bearer` und muss nicht geändert werden).

</details>

<details>
<summary>Konfigurationsbeispiel: sm.ms</summary>

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

sm.ms verwendet den Token direkt als Authorization-Wert, daher wird `AUTH_SCHEME` als leere Zeichenfolge gesetzt.

</details>

<details>
<summary>Konfigurationsbeispiel: imgbb</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://api.imgbb.com/1/upload?key=你的API_KEY",
    "MARK_SHOT_UPLOAD_FIELD": "image",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

imgbb übergibt den API-Schlüssel über einen URL-Abfrageparameter; `API_KEY` muss nicht gesetzt werden.

</details>

<details>
<summary>Konfigurationsbeispiel: litterbox (temporäres Bild-Hosting, kein API-Schlüssel erforderlich)</summary>

```json
"upload": {
  "command": "curl -sf --max-time 30 -A 'Mozilla/5.0' -F reqtype=fileupload -F time=72h -F fileToUpload=@{image} https://litterbox.catbox.moe/resources/internals/api.php",
  "timeoutMs": 35000
}
```

Die Antwort von litterbox ist eine reine Text-URL (kein JSON). Mark Shot erkennt automatisch Ausgaben, die mit `http://`/`https://` beginnen, als Upload-Ergebnis.

</details>

<details>
<summary>Benutzerdefinierte Upload-Befehle</summary>

Falls der integrierte Helper nicht ausreicht, kann über `upload.command` ein beliebiges benutzerdefiniertes Upload-Skript angebunden werden. Der Befehl muss folgende Anforderungen erfüllen:

1. **Exit-Code**: Bei Erfolg ist der Exit-Code 0; jeder Wert ungleich 0 gilt als Fehler
2. **Ausgabeformat** (eine von zwei Optionen):
   - **JSON**: `{"url":"https://...","deleteUrl":"https://...","errors":[]}` (`url` ist Pflichtfeld, die übrigen Felder sind optional)
   - **Reine Text-URL**: Die erste nicht leere Zeile der Standardausgabe beginnt mit `http://` oder `https://`
3. **Platzhalter**: `{image}`, `{imagePath}` und `{imageUrl}` werden unterstützt; enthält der Befehl keinen Platzhalter, hängt Mark Shot automatisch den temporären Bildpfad an das Befehlsende an

```json
"upload": {
  "command": "/path/to/your-uploader.sh --file {image} --json",
  "timeoutMs": 30000,
  "env": {
    "UPLOADER_API_KEY": "xxx"
  }
}
```

Die Umgebungsvariablen aus `upload.env` werden auch an den benutzerdefinierten Befehl weitergegeben, sodass Konfigurationen wiederverwendet werden können.

</details>

#### Windows

Installiere Qt 6, CMake und Ninja passend zum verwendeten Compiler sowie einen Compiler mit C++17-Unterstützung, z. B. MSVC oder MinGW. Für Windows-Builds sind Qt DBus, PipeWire, X11/XCB, LayerShellQt, `grim`, `wl-copy` oder `xclip` nicht erforderlich.

```powershell
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
cmake --build build-windows
```

Der aktuelle Windows-Unterstützungsumfang umfasst normale Screenshots und Bildannotation. Scroll-Screenshots, die auf Compositor spezialisierte Fenstererkennung und Linux-Desktop-Verknüpfungen stehen unter Windows nicht zur Verfügung. Die integrierten Python-Hilfsskripte (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`) werden nicht automatisch installiert; für die manuelle Konfiguration siehe den Abschnitt [OCR-Backend](#ocr-后端可选), [Code-Scan-Backend](#扫码后端可选) und den Abschnitt zur Übersetzung weiter oben.

### Bauen und Kompilieren

```bash
# 使用系统 Qt 6
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 如果 Qt 6 安装在用户目录，额外指定 CMAKE_PREFIX_PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64

# 执行编译
cmake --build build
```

Alternativ mit nix:

```bash
nix build
```

LayerShellQt wird automatisch erkannt. Wird es gefunden, wird die vollständige Wayland-layer-shell-Unterstützung aktiviert; ist es nicht vorhanden, gelingt die Kompilierung dennoch und zur Laufzeit wird automatisch auf ein standardmäßiges Vollbildfenster zurückgegriffen.

### Installation und Integration

```bash
cmake --install build --prefix "$HOME/.local"
```

Dieser Befehl installiert die ausführbare Datei, die Hilfsskripte (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`, `mark-shot-upload`), Desktop-Verknüpfungen und Icons.

### GNOME-Wayland-Scroll-Screenshot-Erweiterung

Für Scroll-Screenshots unter GNOME Wayland muss die Erweiterung **Mark Shot Scroll Helper** aktiviert sein. Ohne diese Erweiterung kann Mark Shot den ausgewählten Bereich nicht still und kontinuierlich erfassen und auch kein natives GNOME-Scroll-Vorschaufenster zeichnen; daher wird die Scroll-Screenshot-Schaltfläche unter GNOME Wayland deaktiviert.

Die Erweiterungsdateien befinden sich im Projekt-Repository unter `../packaging/gnome-extension/mark-shot-scroll-helper@snemc.org`.

<details>
<summary><b>Anleitung zur Installation und Aktivierung der GNOME-Wayland-Scroll-Screenshot-Erweiterung (auf-/zuklappen)</b></summary>

##### Variante A: Installation über das Distributionspaket
Wenn Mark Shot über ein Distributionspaket (z. B. `.deb` oder `.rpm`) installiert wurde, ist die Erweiterung bereits systemweit mitinstalliert. Führe den folgenden Befehl aus, um die Erweiterung für den aktuellen Benutzer zu aktivieren:
```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```
*Falls die Erweiterung nicht gefunden wird, melde dich ab und wieder an, bevor du es erneut versuchst.*

##### Variante B: Installation aus dem Quellverzeichnis des Repositories
Wenn Mark Shot aus dem Quellcode oder manuell lokal gebaut wurde, muss die Erweiterung zunächst in das GNOME-Erweiterungsverzeichnis des Benutzers kopiert werden:
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

Überprüfe, ob die D-Bus-Schnittstelle des Helpers verfügbar ist:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
```

Erwartetes Ergebnis: `('4.2',)`. Starte `mark-shot` nach dem Aktivieren der Erweiterung neu.

</details>

---

## Interaktive Tastenkürzel und Gesten

### Tastenkürzel zum Werkzeugwechsel

| Tastenkürzel | Zielwerkzeug | Funktionsbeschreibung |
| :---: | :--- | :--- |
| **V** | Verschieben / Navigieren (Move / Pan) | Dient im vorhandenen Bildmodus zum Verschieben und Ziehen der Bildleinwand. |
| **S** | Auswählen (Select) | Wählt gezeichnete Vektorannotationen aus und verschiebt, skaliert oder löscht sie. |
| **P** | Stift (Pen) | Zeichnet freie Kurven. |
| **L** | Linie (Line) | Zeichnet gerade Vektorlinien. |
| **H** | Textmarker (Highlighter) | Halbtransparente Hervorhebung, geeignet zum Markieren wichtiger Stellen. |
| **R** | Rechteck (Rectangle) | Zeichnet rechteckige Rahmen. |
| **E** | Ellipse (Ellipse) | Zeichnet elliptische Rahmen. |
| **A** | Pfeil (Arrow) | Zeichnet den klassischen sechseckigen, spitzen Pfeil mit langem, scharfem Winkel. |
| **T** | Text (Text) | Eingabe und Anordnung von Rich Text, unterstützt 1000-px-Schriftgrößen und Zieh-Verknüpfung. |
| **N** | Nummer (Number) | Automatisch fortlaufende Schrittnummern-Aufkleber. |
| **M** | Mosaik (Mosaic) | Unkenntlichmachen sensibler Bereiche per Milchglas-Effekt. |
| **G** | Laserpointer (Laser) | Temporäre Spuren für Unterricht oder Präsentationen, die automatisch sanft verblassen. |

### Hilfswerkzeuge im Startbildschirm

| Tastenkürzel | Werkzeug | Funktionsbeschreibung |
| :---: | :--- | :--- |
| **C** | Pipette (Color Picker) | Misst Pixel des Screenshots, bevor der Screenshot-Bereich ausgewählt wird. Das Mausrad passt die Lupengröße an; ein Linksklick öffnet das Farbfeld, aus dem Formate wie HEX, RGB, HSL, HSV und Qt kopiert werden können. Rechtsklick oder Esc kehrt zur normalen Auswahl zurück. |
| **R** | Lineal (Ruler) | Misst Koordinaten, bevor der Screenshot-Bereich ausgewählt wird. Beim Überfahren wird die aktuelle Pixelposition angezeigt; per Links-Ziehen wird ein Messrechteck mit Pixel-Skala gezeichnet, das Breite, Höhe, Diagonale und Fläche anzeigt. Rechtsklick oder Esc kehrt zur normalen Auswahl zurück. |
| **Q** | Code-Scanner (Code Scanner) | Wechselt in den Scan-Modus für QR-Codes und Barcodes. Nach der Auswahl einer Region wird deren Code-Inhalt erkannt und in einem kopierbaren Fenster angezeigt. Rechtsklick oder Esc kehrt zur normalen Auswahl zurück. |
| **D** | Display-Erfassung (Display Capture) | Erfasst sofort alle Ausgabebildschirme, schneidet sie pro Monitor zu und zeigt Miniaturansichten; beim Überfahren mit der Maus kann kopiert, bearbeitet oder gespeichert werden. |

### Globale Aktions-Tastenkürzel

| Tastenkürzel | Ausgelöste Aktion |
| :---: | :--- |
| **Esc** | Beendet sofort und schließt das Annotationsfenster. |
| **Ctrl + C** | Bestätigt alle Texteingaben und kopiert den aktuellen Screenshot bzw. den annotierten Bereich in die Systemzwischenablage. |
| **Ctrl + S** oder **Enter / Return** | Bestätigt alle Texteingaben und speichert den aktuellen Screenshot. |
| **Ctrl + P** | Fixiert den aktuellen Auswahlbereich als schwebendes Pin-Fenster. |
| **Ctrl + U** | Lädt den aktuellen Screenshot auf ein benutzerdefiniertes Bild-Hosting hoch; nach erfolgreichem Upload wird die URL automatisch in die Zwischenablage kopiert. |
| **Ctrl + Z** | Macht den letzten Annotationsschritt rückgängig. |
| **Ctrl + Y** oder **Ctrl + Shift + Z** | Stellt eine rückgängig gemachte Annotationsaktion wieder her. |
| **Backspace** oder **Delete** | Löscht die ausgewählte Annotation, wenn das Werkzeug **Auswählen (Select)** aktiv ist und eine Annotation markiert wurde. |
| **F** | Schaltet den Erfassungsbereich des aktuellen Screenshots um (zwischen Auswahlmodus und Vollbildmodus). |

### Fortgeschrittene Interaktionstechniken

- **Formbeschränkung beim Zeichnen**: Beim Zeichnen eines **Rechtecks (Rectangle)** oder einer **Ellipse (Ellipse)** erzwingt das Halten von `Ctrl` ein Quadrat bzw. einen Kreis.
- **Schneller Wechsel zum Auswahlwerkzeug**: Während des Annotierens wechselt ein Rechtsklick auf eine leere Stelle der Leinwand sofort zum Werkzeug **Auswählen (Select)**.
- **Farbe per Doppel-Rechtsklick schnell wechseln**: Ein Doppelklick mit der rechten Maustaste auf eine leere Stelle der Leinwand öffnet den Ringfarbwähler, um die Farbe des aktuellen Annotationswerkzeugs schnell zu wechseln.
- **Stufenlose Mausrad-Einstellung**: Bei aktivem Annotationswerkzeug passt das Mausrad in Echtzeit Linienstärke, Schriftgröße, Größe der Nummernaufkleber oder die Mosaik-Rastergröße des aktuellen Werkzeugs an.
- **Leinwand verschieben und zoomen**: Im Modus des Werkzeugs **Auswählen (Select)** oder beim Bearbeiten lokaler Dateien zoomt das Mausrad nahtlos auf der Leinwand; Ziehen mit gedrückter mittlerer Maustaste verschiebt die Leinwand. Ein Doppelklick auf `Ctrl` setzt Zoom und Verschiebung zurück.

### Interaktionen speziell für Pin-Fenster

| Geste / Tastenkürzel | Auswirkung |
| :--- | :--- |
| **Linke Maustaste gedrückt halten und ziehen** | Verschiebt und platziert das Pin-Fenster frei auf dem Desktop. |
| **Mausrad nach oben/unten** | Vergrößert bzw. verkleinert das Pin-Fenster proportional und stufenlos. |
| **Doppelklick mit linker Maustaste** | Schließt das Pin-Fenster in Sekundenschnelle. |
| **Klick mit rechter Maustaste** | Öffnet ein Funktionsmenü (u. a. Drehen, Bildtext kopieren, Übersetzen, Speichern, Kopieren, Schließen). |
| **Esc-Taste** | Schließt das aktuell fokussierte Pin-Fenster. |

---

## Versionshinweise

Siehe die [Versionshinweise](../docs/releases.zh-CN.md).

## Feedback und Kommunikation

### Ein Issue melden
Wenn du beim Ausführen auf Probleme stößt oder Vorschläge für neue Funktionen hast, empfehlen wir, ein Issue über das GitHub-CLI-Befehlszeilentool (`gh`) zu melden. Wir stellen ein Skript bereit, das Umgebungsinformationen mit einem Klick sammelt und automatisch generiert; weitere Details findest du in der [Anleitung zum Melden von Issues](../.doc/submit-issue-via-gh.md).

---

## Lizenz

Dieses Projekt ist unter der **MIT-Lizenz** als Open Source lizenziert; weitere Details findest du in der Datei [LICENSE](../LICENSE).

## Danksagung

Mark Shot steht auf den Schultern der Open-Source-Community; dafür gilt unser aufrichtiger Dank:

- **Das ursprüngliche Upstream-Projekt [jswysnemc/mark-shot](https://github.com/jswysnemc/mark-shot) sowie seine Autoren und alle Mitwirkenden.** Diese Community-Version baut auf dem ursprünglichen Upstream-Projekt auf; dessen herausragendes Design und die kontinuierlichen Beiträge sind die Grundlage für alles, und wir danken aufrichtig für ihre großartige Arbeit.
- **[serendipitywgy](https://github.com/serendipitywgy)**: Dank für die Beiträge über `serendipitywgy/mark-shot`, darunter verbesserte Cross-Desktop-Kompatibilität, die OCR-Kopier-Aktionsleiste und die intelligente Rechteck-Vorauswahl.
- **Alle Open-Source-Projekte, von denen Mark Shot abhängt**, darunter Qt 6, PipeWire, xdg-desktop-portal, layer-shell-qt, wl-clipboard, xclip, grim, RapidOCR, onnxruntime, Tesseract, ZXing-C++ und weitere.

Diese Community-Version wird von der [Beijing Taiyin Zhaowu Technology Co., Ltd.](https://github.com/tystudio-26020701/mark-shot-community) und Mitwirkenden gepflegt und ist unter der **MIT-Lizenz** als Open Source lizenziert.
