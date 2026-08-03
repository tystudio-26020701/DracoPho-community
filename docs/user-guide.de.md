# Mark Shot Benutzerhandbuch

Dieses Handbuch behandelt den täglichen Betrieb von Mark Shot mit einem
Schwerpunkt auf der **Fenster-/Komponenten-Hover-Auswahl** (beim Bewegen der
Maus wird das Fenster unter dem Cursor automatisch verfolgt und hervorgehoben;
ein Klick wählt es aus), dem Annotation-Workflow, Headless-Erfassung und der
Konfiguration.

> Die Dokumente in diesem Repository werden im Community-Fork verfasst und in
> die Upstream- und Enterprise-Repositories gespiegelt. Die Enterprise-Edition
> enthält einen zusätzlichen Abschnitt für ihren lokalen MCP-Server.

---

## 1. Schnellstart

### 1.1 Starten

Eine Regions-Erfassungssitzung starten:

```bash
mark-shot
```

Einen Desktop-Hotkey drücken (siehe § 8) oder aus einem Terminal starten. Ein
eingefrorenes Vollbild-Overlay öffnet sich auf dem fokussierten Display. Die
Maus bewegen, um ein Auswahlrechteck zu ziehen, dann loslassen, um den
Annotation-Editor zu öffnen.

### 1.2 Portable Builds

Wenn Sie ein portables Bundle verwenden (`mark-shot-upstream`,
`mark-shot-community`, `mark-shot-enterprise`), starten Sie es mit dem
gebündelten Launcher, damit die gebündelten Qt-Bibliotheken, Plugins und
Hilfsskripte gefunden werden:

```bash
portable/mark-shot-community/bin/run-mark-shot.sh
```

Der Launcher stellt sein `bin/`-Verzeichnis vor `PATH`, was für die
Fenstererkennungs-Hilfsskripte (`mark-shot-window-detection-*`) sowie die
OCR-/Upload-Helfer erforderlich ist.

---

## 2. Fenster-/Komponenten-Hover-Auswahl

Mark Shot kann die Fenster des aktuellen Desktops erkennen, bevor Sie eine
Region auswählen. Während das Auswahl-Overlay geöffnet ist, **wird beim Bewegen
der Maus das Fenster unter dem Cursor mit einem petrolfarbenen Rahmen
hervorgehoben**. **Ein einfacher Linksklick (ohne Ziehen) wählt das gesamte
Fenster** als Erfassungsregion aus; Sie können es dann direkt annotieren,
kopieren, anpinnen oder speichern.

Die hervorgehobenen Fenster stammen aus einem pro-Compositor-Erkennungsskript,
das vor dem Erscheinen des Overlays ausgeführt wird:

| Desktop | Erkennungsquelle | Hinweise |
| :--- | :--- | :--- |
| GNOME Wayland | gebündelte Shell-Erweiterung `mark-shot-scroll-helper@snemc.org` über D-Bus | erfordert, dass die Erweiterung aktiviert ist (siehe § 2.1) |
| KDE Plasma Wayland | One-Shot-KWin-Scripting über `qdbus6` / `qdbus` + journalctl | erfordert eine KWin-Sitzung |
| Hyprland | `hyprctl -j clients` | |
| niri | `niri msg -j windows` + Konfigurationsparsing | |
| X11 | In-Process-XCB-Enumeration von `_NET_CLIENT_LIST_STACKING` | kein Skript erforderlich |
| Windows | In-Process-`EnumWindows` | kein Skript erforderlich |

Es werden nur **Top-Level-Fenster** verfolgt. Einzelne Widgets innerhalb eines
Fensters („Komponenten“) werden von Wayland-Compositoren nicht bereitgestellt,
daher zielt die Hover-Auswahl auf allen Plattformen auf ganze Fenster.

### 2.1 GNOME Wayland: die Helper-Erweiterung aktivieren

```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```

Überprüfen, ob der D-Bus-Helper antwortet:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
# -> ('5',)
```

Wenn der Aufruf fehlschlägt, ab- und wieder anmelden (oder auf X11 GNOME Shell
neu starten) und erneut versuchen. Ohne die Erweiterung beendet sich das
GNOME-Helferskript mit einem Fehler und die Hover-Auswahl bleibt deaktiviert
(die normale Zieh-Auswahl funktioniert weiterhin).

### 2.2 Verwendung

1. Eine Erfassung auslösen (`mark-shot` oder den Desktop-Hotkey).
2. Ohne eine Maustaste zu drücken, den Cursor über ein Fenster bewegen. Ein
   petrolfarbener Rahmen umrandet das Fenster, das ausgewählt würde.
3. **Einmal klicken** (drücken und loslassen, ohne sich mehr als ein paar Pixel
   zu bewegen), um dieses Fenster auszuwählen. Wenn sich Fenster überlappen,
   gewinnt das oberste Fenster unter dem Cursor (z-Order-bewusst).
4. Das Loslassen öffnet den Annotation-Editor mit dem exakt eingerahmten
   Fenster.
5. Um stattdessen eine **manuelle** Region zu erstellen, einfach wie gewohnt ein
   Rechteck ziehen – der Hover-Rahmen wird ignoriert, sobald das Ziehen die
   Klick-Schwelle überschreitet.

Die Hover-Hervorhebung ist deaktiviert, während das Startwerkzeug
Farbwähler (`C`) oder Lineal (`R`) aktiv ist, und bleibt verfügbar für
Codescanner (`Q`), Display-Erfassung (`D`) und die Startmodi GIF-/Videorecording.

### 2.3 Auswählen von Fenstern auf dem richtigen Monitor

Die Fenstererkennung läuft pro Erfassungsziel. Bei einem Multi-Monitor-Setup
erhält jedes eingefrorene Fenster nur die Fenster, die seine eigene Geometrie
schneiden, sodass der Hover-Rahmen dem entspricht, was Sie auf diesem Display
sehen.

### 2.4 Aktivieren / Deaktivieren

Die Funktion ist standardmäßig aktiviert (`windowDetection.enabled = true`).
Schalten Sie sie in **Einstellungen → Erweitert → Fenstererkennung aktiviert**
um oder bearbeiten Sie `~/.config/mark-shot/config.json`:

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

- `command`: das Erkennungsskript. Auf GNOME / KDE / Hyprland / niri Wayland
  wird das zu Ihrer Sitzung passende gebündelte `mark-shot-window-detection-*`-
  Skript automatisch ausgewählt; auf X11 und Windows wird die Plattform
  In-Process enumeriert und `command` kann leer bleiben. **Ein vom Benutzer
  bereitgestelltes benutzerdefiniertes Kommando (z. B. ein absoluter Pfad) wird
  immer respektiert.**
- `timeoutMs`: maximale Wartezeit auf das Skript (100–30000 ms, Standard 1000).
- `env`: zusätzliche Umgebungsvariablen, die an das Skript übergeben werden.
  Pro-Compositor-Anpassungen (Offsets) sind in den Skriptköpfen dokumentiert.

### 2.5 Fehlerbehebung

| Symptom | Prüfen |
| :--- | :--- |
| Kein petrolfarbener Rahmen auf GNOME Wayland | Erweiterung aktiviert? Der `gdbus`-Aufruf oben muss eine Version zurückgeben |
| Kein petrolfarbener Rahmen auf X11 / Windows | keine – die Plattform-Enumeration ist eingebaut; stellen Sie sicher, dass die Erfassungssitzung kein Startzeiger-Werkzeug verwendet |
| Der Hover-Rahmen wählt das falsche (darunterliegende) Fenster aus | z-Order-Daten fehlen in einem benutzerdefinierten Erkennungsskript; Fenster ohne `zOrder` werden als unterste Ebene eingestuft |
| Die Erfassung startet langsam | das Erkennungsskript läuft vor dem Overlay; `timeoutMs` nur erhöhen, wenn der Desktop langsam ist, oder `enabled:false` setzen, um es zu überspringen |
| Diagnose ansehen | `mark-shot --debug --debug-log /tmp/mark-shot.log` ausführen; nach `window-detection`-Zeilen suchen |

---

## 3. Regionsauswahl & Startwerkzeuge

Bevor die Region übernommen wird, können Sie die Startwerkzeuge des Overlays
verwenden:

| Hotkey | Werkzeug | Verhalten |
| :---: | :--- | :--- |
| `C` | Farbwähler | Ein Pixel abtasten; das Rad vergrößert/verkleinert die Lupe; Linksklick öffnet ein Farbpanel (HEX / RGB / HSL / HSV / Qt-Formate); Rechtsklick oder `Esc` beendet |
| `R` | Lineal | Hover liest Pixelkoordinaten; Links-Ziehen misst ein Rechteck mit Breite, Höhe, Diagonale und Fläche; Rechtsklick oder `Esc` beendet |
| `Q` | Codescanner | Eine Region um einen QR-/Barcode ziehen; das dekodierte Ergebnis wird in einem kopierbaren Fenster geöffnet |
| `D` | Display-Erfassung | Erfasst alle Ausgaben, schneidet pro Display zu, zeigt überfahrbare Miniaturen (kopieren / bearbeiten / speichern) |
| `S` | Aktive GIF-/Videoaufnahme stoppen | stoppt die im Overlay angezeigte Aufnahme |

`Esc` bricht die Sitzung ab; Rechtsklick (ohne Startwerkzeug) bricht ebenfalls
ab.

#### 3.1 Einfrierverhalten bei mehreren Monitoren

Mit dem standardmäßigen Erfassungsbereich **Freeze All Screens** wird jeder
angeschlossene Bildschirm eingefroren, während eine Region ausgewählt wird.
Sobald Sie eine Auswahl auf einem Monitor bestätigen, zeigen die anderen Displays
weiterhin ihr eingefrorenes Bild als nicht interaktiven Hintergrund: Maus-,
Tastatur-, Rad- und Tastenkombinationseingaben werden verschluckt und die Overlays
zeigen keine Werkzeugleisten, sodass der Rest des virtuellen Desktops eingefroren
bleibt, bis die Erfassungssitzung endet. Wenn Sie stattdessen den Bereich
**Cursor Screen** (Settings → Capture → Freeze Scope) verwenden, wird nur der
Monitor unter dem Cursor eingefroren und die anderen Bildschirme bleiben
vollständig nutzbar.

---

## 4. Annotation-Werkzeuge

Nachdem eine Region ausgewählt wurde (oder ein lokales Bild geöffnet wurde),
öffnet sich der Editor mit der Annotation-Werkzeugleiste. Die Werkzeuge werden
mit den Zifferntasten oder der Werkzeugleiste umgeschaltet:

| Hotkey | Werkzeug | Beschreibung |
| :--- | :--- | :--- |
| `V` | Verschieben / Schwenken | die gesamte Auswahl verschieben, ein lokales Bild-Canvas schwenken |
| `S` | Auswählen | vorhandene Annotationen auswählen, verschieben, skalieren, drehen, löschen |
| `P` | Stift | glatte Freihand-Striche |
| `L` | Linie | gerade Linien |
| `H` | Textmarker | halbtransparenter Marker; Freihand- oder Gerade-Linie-Stil |
| `R` | Rechteck | Kasten mit `Stroke`- / `Highlight`- / `Invert`-Stilen, abgerundete Ecken |
| `E` | Ellipse | Ellipse / Kreis |
| `A` | Pfeil | klassische Pfeile (befiedert, KDE, bidirektional) |
| `T` | Text | Rich-Text; Rad oder Schieberegler ändern die Größe; diagonale Griffe skalieren beides, seitliche Griffe passen den Umbruch an; exakte pt-Größe, Schriftfamilie, fett / kursiv im Schriftpanel |
| `N` | Nummer | fortlaufende nummerierte Marker (arabisch, alpha, römisch, chinesisch, …) |
| `M` | Mosaik | acrylartiger Frost-Unschärfeeffekt zum Verbergen sensibler Inhalte |
| `G` | Laser | temporäre Striche, die automatisch verblassen |

Zeichen-Tipps:

- Beim Zeichnen eines Rechtecks / einer Ellipse `Ctrl` gedrückt halten, um ein
  Quadrat / einen Kreis zu erzwingen.
- Das Rad drehen, während ein Werkzeug aktiv ist, um Strichbreite, Textgröße,
  Zahlenskalierung oder Mosaik-Blockgröße anzupassen (Live-Vorschau).
- Unter `Select` mit dem Rad scrollen, um das Canvas zu zoomen, und die
  mittlere Taste gedrückt halten, um zu schwenken; `Ctrl` doppelt antippen, um
  zurückzusetzen.

### 4.1 Eine vorhandene Annotation bearbeiten

Zu **Select** (`S`) wechseln. Auf eine Annotation klicken, um ihre Griffe
anzuzeigen:

- innerhalb ziehen, um zu verschieben;
- Ecken-/Kantengriffe ziehen, um die Größe zu ändern;
- den runden Griff über der Oberkante ziehen, um zu drehen;
- `Delete` / `Backspace` drücken, um zu entfernen;
- doppelklicken Sie auf Text, um ihn direkt zu bearbeiten.

Das Eigenschaften-Panel (rechte Seite) bearbeitet die ausgewählte Annotation:
Farbe, Breite, Stil, Textschriftfamilie / -größe / fett / kursiv. Mehrere
Annotationen können ausgewählt werden, indem unter dem `Select`-Werkzeug ein
Auswahlrechteck gezogen wird; die Gruppe kann dann gemeinsam verschoben,
skaliert, gedreht und gelöscht werden.

### 4.2 Aktionen

| Tastenkürzel | Aktion |
| :--- | :--- |
| `Ctrl+C` | in die Zwischenablage kopieren |
| `Ctrl+S` / `Enter` | speichern (Pfadvorlage aus den Einstellungen) |
| `Ctrl+P` | als schwebendes Sticker-Fenster anpinnen |
| `Ctrl+U` | zum konfigurierten Bildhost hochladen; die URL wird kopiert |
| `Ctrl+Z` / `Ctrl+Y` | Rückgängig / Wiederholen |
| `F` | den Erfassungsbereich umschalten (Auswahl ↔ Vollbild) |

### 4.3 Export-Rahmen

**Einstellungen → Export → Mac-Stil-Rahmen** aktivieren, um gespeicherten /
kopierten / hochgeladenen Bildern transparente Auffüllung, abgerundete Ecken
und einen weichen Schatten hinzuzufügen.

---

## 5. Angepinnte Fenster-Sticker

| Geste / Tastenkürzel | Verhalten |
| :--- | :--- |
| Links-Ziehen | den Sticker neu positionieren |
| Rad | proportional skalieren |
| Doppelter Linksklick / `Esc` | schließen |
| Rechtsklick | Kontextmenü (drehen, zoomen, immer im Vordergrund, Text kopieren, übersetzen, speichern, kopieren, schließen) |

OCR-Text in einem angepinnten Fenster ist auswählbar und kopierbar (`Ctrl+C` /
Kontextmenü). Übersetzung (OpenAI-kompatibler Endpunkt) rendert den übersetzten
Text an den ursprünglichen Layout-Positionen wieder auf das Bild.

---

## 6. Scrollender Screenshot

1. Eine Region auswählen (oder für sehr große Regionen den schwebenden
   Zieh-Griff verwenden).
2. Das Overlay scrollt das Zielfenster; erfasste Frames werden zu einem langen
   Bild zusammengenäht.
3. GNOME Wayland erfordert die Mark Shot Scroll Helper-Erweiterung (§ 2.1).

Die Scroll-Erfassung ist auf niri und ähnlichen wlroots/Wayland-Compositoren
produktionsreif; auf KDE, X11 und anderen Stacks ist sie eine Testfunktion.
Wenn sie fehlschlägt, normale Screenshots oder ein benutzerdefiniertes
Erweiterungskommando verwenden.

---

## 7. Headless-Erfassung (CLI)

Nicht-interaktive Erfassung schreibt ein PNG und gibt JSON aus:

```bash
# primärer Bildschirm
mark-shot --capture-to /tmp/shot.png

# Verzeichnis (Dateiname mit Zeitstempel)
mark-shot --capture-to /tmp/shots/

# Region
mark-shot --capture-to /tmp/r.png --region 0,0,1280,720

# ein bestimmtes Display, mit Cursor
mark-shot --capture-to /tmp/w.png --display DP-1 --include-cursor

# mehrere Displays gleichzeitig (je ein PNG)
mark-shot --capture-to /tmp/shots/ --display DP-1 --display DP-2

# Ausgaben auflisten
mark-shot --list-displays
```

Alle Headless-Optionen schließen sich mit einer positionalen Bilddatei
gegenseitig aus. Die vollständige Argumenttabelle finden Sie in der README.

### 7.1 Headless-Fenster-/Komponentenerfassung

Mark Shot kann **bestimmte Fenster – oder eine Komponente (Teilregion) innerhalb
eines Fensters – ohne Öffnen einer Benutzeroberfläche erfassen**, aus einem
Skript, einer Build-Pipeline oder einem Agenten. Der Prozess beendet sich,
sobald die Bilder geschrieben oder zurückgegeben sind, und er erstellt niemals
ein Fenster, zeigt niemals einen Dialog und stiehlt niemals den Fokus, sodass
der Benutzer weiterarbeiten kann, während ein Werkzeug den Desktop erfasst.

Zuerst die Fenster auflisten, um zu sehen, was verfügbar ist:

```bash
mark-shot --list-windows
```

Beispielausgabe (GNOME Wayland):

```json
{"count":2,"platform":"wayland","source":"compositor-script","windows":[
  {"index":0,"id":"0x3c00007","title":"Mark Shot - VSCodium","class":"codium","instance":"codium","x":1920,"y":0,"width":1680,"height":1050,"zOrder":1},
  {"index":1,"title":"Terminal","class":"org.gnome.Terminal","x":67,"y":32,"width":800,"height":600}
]}
```

Jeder Eintrag trägt die Felder, gegen die Selektoren abgeglichen werden:
`index`, `id` (X11-Fenster-ID / vom Backend bereitgestellte ID), `title`,
`class` und `instance`, plus `x`/`y`/`width`/`height` und ein optionales
`zOrder`.

#### 7.1.1 Auswählen von Fenstern (einzeln oder mehrere)

`--window` kann wiederholt werden, um **eine beliebige Anzahl von Fenstern in
einem Aufruf zu erfassen**. Jeder Selektor wird automatisch interpretiert
(`--window-by auto`):

| Selektorwert            | Entspricht                                            |
| :---                    | :---                                                  |
| `0`, `1`, …             | Listen-`index`                                        |
| `0x3c00007`             | Fenster-`id`                                          |
| `VSCodium`              | `class` oder `instance`, dann `title` (exakt, dann Teilstring) |
| `Mark Shot - VSCodium`  | `title`                                               |

Eine einzelne Abgleichregel mit `--window-by id|title|class|index` erzwingen.
Ein Selektor, der mehrere Fenster abgleicht, erfasst **alle davon**.

Eine Komponente (eine Teilregion innerhalb eines Fensters) erfassen, indem
`@x,y,width,height` an den Selektor angehängt wird – der Offset ist relativ zur
oberen linken Ecke des Fensters und wird auf die Fenstergrenzen begrenzt:

```bash
# der obere 100-px-Streifen von Fenster 0
mark-shot --window "0@0,0,1680,100" --capture-destination file --capture-to /tmp/shots/
```

#### 7.1.2 Auswählen, wohin die Bilder gehen

`--capture-destination` entscheidet über die Ausgabe; es kann mit einer
beliebigen Anzahl von `--window`-Selektoren und einer Komponenten-Teilregion
kombiniert werden:

| Ziel | Verhalten |
| :--- | :--- |
| `inline` (Standard) | Base64-PNGs in die JSON-Ausgabe eingebettet. **Es werden keine Dateien geschrieben und die Zwischenablage wird nie berührt.** Die sicherste Wahl für Agenten, die nur die Pixel wollen. |
| `file` | PNG-Dateien nach `--capture-to <Verzeichnis>` geschrieben; erfordert diese Option. |
| `stage` | PNG-Dateien in ein temporäres Staging-Verzeichnis geschrieben (`$TMPDIR/mark-shot-staging`). Gut für einen „für später aufheben"-Workflow. |
| `clipboard` | Bilder in die System-Zwischenablage kopiert; bei mehreren Bildern gewinnt das **letzte**. Der Inhalt überlebt das Beenden der CLI (ein persistentes `wl-copy`- / `xclip`-Owner-Objekt wird gestartet). |

Beispiele:

```bash
# mehrere Fenster, in ein Verzeichnis gespeichert (ein PNG pro Fenster)
mark-shot --window VSCodium --window Terminal --capture-destination file --capture-to /tmp/shots/

# ein Fenster plus eine Komponente eines anderen Fensters, für später gestaged
mark-shot --window "VSCodium@0,0,400,300" --window 1 --capture-destination stage

# Mehrfachauswahl, als base64 zurückgegeben, ohne Dateien oder Zwischenablage zu berühren
mark-shot --window 0 --window "Terminal" --capture-destination inline

# ein Fenster in die Zwischenablage kopieren
mark-shot --window 0 --capture-destination clipboard
```

**Zwischenablage-Richtlinie.** Der interaktive Editor legt Ihre Auswahl
bewusst in die System-Zwischenablage (die Aktion `Copy` / `Ctrl+C`), weil das
der primäre Workflow eines Screenshot-Werkzeugs ist. Headless-Modi (die CLI
und der Enterprise-MCP-Server) folgen der entgegengesetzten Regel: **die
Zwischenablage wird nie verändert, außer `clipboard` wurde explizit als Ziel
gewählt UND Zwischenablage-Schreibvorgänge sind in Einstellungen > Speicher >
Headless-Modus aktiviert** – `inline` (Standard) und `stage` lassen den
aktuellen Inhalt der Zwischenablage des Benutzers unberührt, sodass eine
geplante oder agentengesteuerte Erfassung keine Texte oder Bilder
überschreiben kann, mit denen der Benutzer woanders arbeitet. Wenn eine
`clipboard`-Anfrage abgelehnt wird, weil Headless-Zwischenablage-Schreibvorgänge
deaktiviert sind, fällt die Erfassung auf das konfigurierte
Headless-Standardziel zurück, die JSON-Ausgabe (`"warning"`) und stderr teilen
Ihnen dies mit, und der Prozess beendet sich mit einem von null verschiedenen
Code, sodass Automatisierung es erkennen kann. Das Aktivieren von
Headless-Zwischenablage-Schreibvorgängen in den Einstellungen erfordert die
Eingabe einer Bestätigungs-Passphrase.

Die Ausgabe ist ein JSON-Objekt `{"captures":[...]}` mit einem Eintrag pro
erfasstem Fenster; jeder Eintrag wiederholt den Selektor, die Fensteridentität
und das endgültige Erfassungsrechteck, plus einen `path` (file/stage) oder
`data` (inline) oder keins von beidem (clipboard). Der Exit-Code ist `0` nur
dann, wenn jeder Selektor übereinstimmte und jede Erfassung erfolgreich war;
ein fehlender Treffer oder eine fehlgeschlagene Erfassung ergibt den Exit-Code
`1` mit einem `"error"`-Feld statt eines stillen Erfolgs.

Dieselbe Erfassungs-Pipeline kann programmatisch annotierte Ausgabe erzeugen –
siehe das Kapitel zum MCP-Server der Enterprise-Edition, oder kombinieren Sie
das gespeicherte PNG mit dem interaktiven Editor.

#### 7.1.3 Garantie ohne Fensterstörung

Jeder Headless-Modus ist garantiert unsichtbar und nicht störend:

- **es wird niemals ein Fenster erstellt** – einschließlich Annotation-Editor,
  Erfassungs-Overlay und Tray; die Erfassung nutzt den Headless-Erfassungspfad
  erneut;
- **es wird niemals ein Dialog angezeigt** – einschließlich Fehlerdialogen:
  Fehler gehen an stderr; selbst fehlerhafte Befehlszeilen (z. B. `--window-by`
  ohne `--window`, ein unbekanntes `--capture-destination` oder zusätzliche
  positionale Dateien) beenden sich sofort mit einem von null verschiedenen
  Code und einer stderr-Nachricht, statt einen `QMessageBox` zu öffnen oder in
  die interaktive Benutzeroberfläche zu fallen;
- kein interaktiver Portal-Prompt erscheint (`allowInteractivePortal` ist
  deaktiviert);
- der Prozess beendet sich unmittelbar nach dem Schreiben der Ausgabe;
- die Fensterliste, die vor und nach einer Headless-Operation erfasst wird, ist
  identisch;
- Headless-Modi berühren nie die System-Zwischenablage, außer `clipboard`
  wurde explizit angefordert **und** Zwischenablage-Schreibvorgänge sind in
  Einstellungen > Speicher > Headless-Modus aktiviert.

Wenn keine Fenster erkannt werden (z. B. ein deaktivierter Compositor-Helper
oder eine X11-Sitzung ohne Fensterenumeration), gibt das Kommando eine klare
Fehlermeldung auf stderr aus und beendet sich mit Code `1`, statt still nichts
zu erfassen.

---

## 8. Desktop-Hotkeys & Tray

Der Tray-Modus (`mark-shot --tray`) registriert `Ctrl+Alt+S` für die
Regionserfassung und stellt Menüeinträge für Erfassung / Aufnahme /
Einstellungen / Beenden bereit. Desktop-Hotkeys:

- **GNOME**: Einstellungen → Tastatur → Tastenkürzel → Benutzerdefinierte
  Tastenkürzel → an `mark-shot` binden.
- **KDE**: benutzerdefiniertes Tastenkürzel, gebunden an `mark-shot` (plus die
  KWin-Berechtigung ScreenShot2 für exakte KDE-Erfassung, siehe README).
- **Hyprland**: `bind = SUPER SHIFT, S, exec, mark-shot` und `bind = , Print, exec, mark-shot`.
- **niri**: `binds { Mod+Shift+S { spawn "mark-shot"; } }`.
- **Sway / i3**: `bindsym Mod4+Shift+S exec mark-shot`.

---

## 9. Konfiguration & Backends

- Konfigurationsdatei: `~/.config/mark-shot/config.json` (Linux), beim ersten
  Start erstellt.
- Vollständige Referenz: [Konfiguration](configuration.md).
- Backends: Wayland (PipeWire-Portal / grim / wlroots-Screencopy), X11
  (`QScreen::grabWindow`), Windows (nativer WGC). Die Aufnahme bevorzugt das
  PipeWire-Portal und fällt automatisch zurück.
- Das Einstellungsfenster verfolgt ungespeicherte Änderungen deterministisch:
  jedes Bedienelement (Dropdown, Schalter, Spinbox, Textfeld, Kürzel-Feld,
  Farbwähler) aktualisiert den Indikator für ungespeicherte Änderungen sofort,
  einschließlich der Werte, die über Combo-Box-Popups und den modalen
  Farbdialog gewählt wurden. Das Rückgängigmachen einer Änderung löscht den
  Indikator, sodass das Fenster beim Schließen nur nach wirklich anstehenden
  Änderungen fragt.

Optionale Helfer:

```bash
# OCR (RapidOCR / Tesseract)
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime

# Codescan (zxing-cpp)
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

---

## 10. Checkliste für Funktionstests

Damit einen Build Ende-zu-Ende verifizieren:

1. **Starten** – `run-mark-shot.sh` öffnet das eingefrorene Overlay.
2. **Fenster-Hover** – die Maus über ein Fenster bewegen: der petrolfarbene
   Rahmen folgt; ein einzelner Klick wählt das Fenster; überlappende Fenster
   wählen das oberste.
3. **Manuelle Region** – ein Rechteck ziehen; loslassen; der Editor öffnet sich.
4. **Annotieren** – mit jedem Werkzeug zeichnen (Stift, Linie, Rechteck,
   Ellipse, Pfeil, Textmarker, Text, Nummer, Mosaik, Lupe, Laser);
   Rückgängig/Wiederholen; Select zum Verschieben/Skalieren/Drehen/Löschen;
   auf einen Text doppelklicken, um ihn zu bearbeiten.
5. **Kopieren / Speichern / Anpinnen / Hochladen** – `Ctrl+C`, `Ctrl+S`,
   `Ctrl+P`, `Ctrl+U`.
6. **Startwerkzeuge** – `C` Farbwähler, `R` Lineal, `Q` Codescan, `D`
   Display-Erfassung.
7. **Headless** – `--capture-to`, `--region`, `--display`, `--list-displays`.
8. **Headless-Fenstererfassung** – `--list-windows` listet den Desktop auf;
   `--window` wiederholen, um mehrere Fenster zu erfassen; `--capture-destination`
   in allen vier Modi testen (inline, file, stage, clipboard); einen
   Komponenten-Selektor verifizieren (`--window "0@0,0,400,300"`); bestätigen,
   dass die Fensterliste davor und danach unverändert ist (keine
   Fensterstörung).
9. **Tray + Hotkey** – `mark-shot --tray`, `Ctrl+Alt+S` drücken.
10. **Portable-Besonderheiten** – das Bundle findet seine eigenen
    Qt-Bibliotheken/Plugins/Skripte.

---

## 11. Feedback

Probleme mit `gh issue create` unter Verwendung des gebündelten
[Leitfadens zum Einreichen von Issues](../.doc/submit-issue-via-gh.md)
melden. Ein Debug-Log anhängen, das mit
`mark-shot --debug --debug-log /tmp/mark-shot.log` erfasst wurde.
