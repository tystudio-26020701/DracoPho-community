# Guida utente di Mark Shot

Questo manuale illustra l'uso quotidiano di Mark Shot, con particolare attenzione
alla funzione **hover selection di finestre / componenti** (spostando il mouse si
traccia e si evidenzia automaticamente la finestra sotto il cursore; un clic la
seleziona), al flusso di annotazione, alla cattura headless e alla configurazione.

> La documentazione in questo repository è redatta nel fork della community e
> rispecchiata nei repository upstream ed enterprise. L'edizione enterprise
> aggiunge un'ulteriore sezione per il suo server MCP locale.

---

## 1. Avvio rapido

### 1.1 Avvio

Avvia una sessione di cattura di una regione:

```bash
mark-shot
```

Premi un tasto di scelta rapida del desktop (vedi § 8) oppure eseguilo da un
terminale. Un overlay a schermo intero e congelato si apre sul display attivo.
Sposta il mouse per disegnare un rettangolo di selezione, quindi rilascia per
entrare nell'editor di annotazione.

### 1.2 Build portabili

Se utilizzi un bundle portabile (`mark-shot-upstream`, `mark-shot-community`,
`mark-shot-enterprise`), avvialo con il launcher incluso così che le librerie
Qt, i plugin e gli script di supporto inclusi nel bundle vengano trovati:

```bash
portable/mark-shot-community/bin/run-mark-shot.sh
```

Il launcher antepone la propria directory `bin/` a `PATH`, requisito per gli
script di supporto per il rilevamento delle finestre
(`mark-shot-window-detection-*`) e per gli helper OCR / upload.

---

## 2. Selezione per hover di finestre / componenti

Mark Shot può rilevare le finestre del desktop corrente prima che tu scelga una
regione. Mentre l'overlay di selezione è aperto, **spostare il mouse evidenzia
la finestra sotto il cursore** con una cornice color verde acqua. **Un clic
sinistro semplice (senza trascinamento) seleziona l'intera finestra** come
regione di cattura; puoi quindi annotare, copiare, fissare o salvare
direttamente.

Le finestre evidenziate provengono da uno script di rilevamento specifico per
compositor che viene eseguito prima che appaia l'overlay:

| Desktop | Sorgente di rilevamento | Note |
| :--- | :--- | :--- |
| GNOME Wayland | estensione Shell `mark-shot-scroll-helper@snemc.org` inclusa nel bundle, tramite D-Bus | richiede che l'estensione sia abilitata (vedi § 2.1) |
| KDE Plasma Wayland | script KWin monouso tramite `qdbus6` / `qdbus` + journalctl | richiede una sessione KWin |
| Hyprland | `hyprctl -j clients` | |
| niri | `niri msg -j windows` + analisi della configurazione | |
| X11 | enumerazione XCB in-process di `_NET_CLIENT_LIST_STACKING` | nessuno script necessario |
| Windows | `EnumWindows` in-process | nessuno script necessario |

Vengono tracciate solo le **finestre di primo livello**. I singoli widget
all'interno di una finestra ("componenti") non sono esposti dai compositor
Wayland, quindi la selezione per hover punta a intere finestre su ogni
piattaforma.

### 2.1 GNOME Wayland: abilita l'estensione di supporto

```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```

Verifica che l'helper D-Bus risponda:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
# -> ('5',)
```

Se la chiamata fallisce, esci e rientra nella sessione (oppure riavvia GNOME
Shell su X11) e riprova. Senza l'estensione lo script di supporto GNOME termina
con un errore e la selezione per hover resta disattivata (la normale selezione
tramite trascinamento funziona comunque).

### 2.2 Come si usa

1. Avvia una cattura (`mark-shot` o il tasto di scelta rapida del desktop).
2. Senza premere alcun pulsante del mouse, sposta il cursore su una finestra.
   Una cornice color verde acqua delinea la finestra che verrebbe selezionata.
3. **Fai clic una volta** (premi e rilascia senza spostarti di più di qualche
   pixel) per selezionare quella finestra. Se le finestre si sovrappongono,
   vince quella più in alto sotto il cursore (consapevole dell'ordine z).
4. Rilascia per entrare nell'editor di annotazione con la finestra incorniciata
   esattamente.
5. Per creare invece una regione **manuale**, trascina semplicemente un
   rettangolo come al solito — la cornice di hover viene ignorata appena il
   trascinamento supera la soglia del clic.

L'evidenziazione di hover è disabilitata mentre è attivo lo strumento di avvio
Color Picker (`C`) o Ruler (`R`) e resta disponibile per Code Scanner (`Q`),
cattura display (`D`) e le modalità di avvio registrazione GIF / Video.

### 2.3 Selezione delle finestre sul monitor corretto

Il rilevamento delle finestre viene eseguito per ogni destinazione di cattura.
In una configurazione multi-monitor, ogni finestra congelata riceve solo le
finestre che intersecano la propria geometria, quindi la cornice di hover
corrisponde a ciò che vedi su quel display.

### 2.4 Attivazione / disattivazione

La funzione è abilitata per impostazione predefinita (`windowDetection.enabled
= true`). Puoi attivarla/disattivarla in **Impostazioni → Avanzate → Window
Detection Enabled**, oppure modificare `~/.config/mark-shot/config.json`:

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

- `command`: lo script di rilevamento. Su GNOME / KDE / Hyprland / niri Wayland
  viene scelto automaticamente lo script `mark-shot-window-detection-*` incluso
  nel bundle e corrispondente alla tua sessione; su X11 e Windows la piattaforma
  viene enumerata in-process e `command` può essere lasciato vuoto. **Un comando
  personalizzato fornito dall'utente (ad esempio un percorso assoluto) viene
  sempre rispettato.**
- `timeoutMs`: attesa massima per lo script (100–30000 ms, default 1000).
- `env`: variabili d'ambiente extra passate allo script. Le regolazioni
  specifiche per compositor (offset) sono documentate nelle intestazioni degli
  script.

### 2.5 Risoluzione dei problemi

| Sintomo | Controllo |
| :--- | :--- |
| Nessuna cornice verde acqua su GNOME Wayland | l'estensione è abilitata? la chiamata `gdbus` qui sopra deve restituire una versione |
| Nessuna cornice verde acqua su X11 / Windows | nessuno — l'enumerazione della piattaforma è integrata; assicurati che la sessione di cattura non usi uno strumento puntatore di avvio |
| La cornice di hover seleziona la finestra sbagliata (sottostante) | dati dell'ordine z mancanti da uno script di rilevamento personalizzato; le finestre senza `zOrder` sono classificate come livello inferiore |
| La cattura parte lentamente | lo script di rilevamento viene eseguito prima dell'overlay; alza `timeoutMs` solo se il desktop è lento, oppure imposta `enabled:false` per saltarlo |
| Vedi la diagnostica | esegui `mark-shot --debug --debug-log /tmp/mark-shot.log`; cerca le righe `window-detection` |

---

## 3. Selezione della regione e strumenti di avvio

Prima di confermare la regione puoi usare gli strumenti dell'overlay di avvio:

| Tasto di scelta rapida | Strumento | Comportamento |
| :---: | :--- | :--- |
| `C` | Color Picker | Campiona un pixel; la rotellina ridimensiona la lente; il clic sinistro apre un pannello colore (formati HEX / RGB / HSL / HSV / Qt); il clic destro o `Esc` esce |
| `R` | Ruler | L'hover legge le coordinate dei pixel; il trascinamento con il tasto sinistro misura un rettangolo con larghezza, altezza, diagonale e area; il clic destro o `Esc` esce |
| `Q` | Code Scanner | Trascina una regione attorno a un QR / codice a barre; il risultato decodificato si apre in una finestra da cui è possibile copiare |
| `D` | Display Capture | Cattura tutti gli output, ritaglia per display, mostra miniature su cui passare il mouse (copia / modifica / salva) |
| `S` | Ferma la registrazione GIF / video attiva | interrompe la registrazione mostrata nell'overlay |

`Esc` annulla la sessione; anche il clic destro (senza strumento di avvio)
annulla.

---

## 4. Strumenti di annotazione

Dopo aver selezionato una regione (o aperto un'immagine locale) si apre
l'editor con la barra degli strumenti di annotazione. Gli strumenti si cambiano
con i tasti numerici o con la barra degli strumenti:

| Tasto di scelta rapida | Strumento | Descrizione |
| :---: | :--- | :--- |
| `V` | Move / Pan | sposta l'intera selezione, sposta la tela di un'immagine locale |
| `S` | Select | seleziona, sposta, ridimensiona, ruota, elimina le annotazioni esistenti |
| `P` | Pen | tratti a mano libera uniformi |
| `L` | Line | linee rette |
| `H` | Highlighter | evidenziatore semitrasparente; stile a mano libera o a linea retta |
| `R` | Rectangle | rettangolo con stili `Stroke` / `Highlight` / `Invert`, angoli arrotondati |
| `E` | Ellipse | ellisse / cerchio |
| `A` | Arrow | frecce classiche (con piumaggio, KDE, bidirezionali) |
| `T` | Text | testo formattato; la rotellina o i cursori ridimensionano; le maniglie diagonali ridimensionano entrambi gli assi, le maniglie laterali regolano l'avvolgimento; dimensione esatta in pt, famiglia di font, grassetto / corsivo nel pannello font |
| `N` | Number | marcatori numerici sequenziali (arabo, alfa, romano, cinese, …) |
| `M` | Mosaic | sfocatura a effetto acrilico per nascondere i contenuti sensibili |
| `G` | Laser | tratti temporanei che si dissolvono automaticamente |

Suggerimenti per il disegno:

- Tieni premuto `Ctrl` mentre disegni un rettangolo / un'ellisse per vincolarli
  a un quadrato / un cerchio.
- Scorri la rotellina mentre uno strumento è attivo per regolare lo spessore
  del tratto, la dimensione del testo, la scala dei numeri o la dimensione dei
  blocchi del mosaico (anteprima dal vivo).
- Sotto `Select`, scorri per fare zoom sulla tela e tieni premuto il pulsante
  centrale per spostarla; premi due volte `Ctrl` per resettare.

### 4.1 Modifica di un'annotazione esistente

Passa a **Select** (`S`). Fai clic su un'annotazione per mostrarne le maniglie:

- trascina all'interno per spostarla;
- trascina le maniglie degli angoli / dei bordi per ridimensionarla;
- trascina la maniglia rotonda sopra il bordo superiore per ruotarla;
- premi `Delete` / `Backspace` per rimuoverla;
- fai doppio clic sul testo per modificarlo sul posto.

Il pannello delle proprietà (lato destro) modifica l'annotazione selezionata:
colore, spessore, stile, famiglia / dimensione / grassetto / corsivo del font
del testo. È possibile selezionare più annotazioni trascinando un riquadro di
selezione sotto lo strumento `Select`; il gruppo può quindi essere spostato,
ridimensionato, ruotato ed eliminato insieme.

### 4.2 Azioni

| Scorciatoia | Azione |
| :--- | :--- |
| `Ctrl+C` | copia negli appunti |
| `Ctrl+S` / `Enter` | salva (modello di percorso dalle impostazioni) |
| `Ctrl+P` | fissa come finestra adesiva flottante |
| `Ctrl+U` | carica sul servizio di immagini configurato; l'URL viene copiato |
| `Ctrl+Z` / `Ctrl+Y` | annulla / ripeti |
| `F` | alterna l'ambito di cattura (selezione ↔ schermo intero) |

### 4.3 Cornice di esportazione

Abilita **Impostazioni → Esporta → Mac-style frame** per aggiungere padding
trasparente, angoli arrotondati e un'ombra morbida alle immagini salvate /
copiate / caricate.

---

## 5. Adesivi con finestra fissata

| Gesto / Scorciatoia | Comportamento |
| :--- | :--- |
| trascinamento con tasto sinistro | riposiziona l'adesivo |
| rotellina | ridimensiona proporzionalmente |
| doppio clic sinistro / `Esc` | chiudi |
| clic destro | menu contestuale (ruota, zoom, sempre in primo piano, copia testo, traduci, salva, copia, chiudi) |

Il testo OCR all'interno di una finestra fissata è selezionabile e copiabile
(`Ctrl+C` / menu contestuale). La traduzione (endpoint compatibile con
OpenAI) riporta il testo tradotto sull'immagine nelle posizioni di layout
originali.

---

## 6. Screenshot a scorrimento

1. Seleziona una regione (oppure usa la maniglia flottante di trascinamento per
   regioni molto grandi).
2. L'overlay fa scorrere la finestra di destinazione; i fotogrammi catturati
   vengono cuciti in un'immagine lunga.
3. GNOME Wayland richiede l'estensione Mark Shot Scroll Helper (§ 2.1).

La cattura a scorrimento è pronta per la produzione su niri e su compositor
wlroots/Wayland simili; su KDE, X11 e altri stack è una funzione sperimentale.
Se fallisce, usa gli screenshot normali o un comando personalizzato
dell'estensione.

---

## 7. Cattura headless (CLI)

La cattura non interattiva scrive un PNG e stampa un JSON:

```bash
# schermo primario
mark-shot --capture-to /tmp/shot.png

# directory (nome file con timestamp)
mark-shot --capture-to /tmp/shots/

# regione
mark-shot --capture-to /tmp/r.png --region 0,0,1280,720

# un display specifico, con cursore
mark-shot --capture-to /tmp/w.png --display DP-1 --include-cursor

# più display contemporaneamente (un PNG ciascuno)
mark-shot --capture-to /tmp/shots/ --display DP-1 --display DP-2

# elenca gli output
mark-shot --list-displays
```

Tutte le opzioni headless si escludono a vicenda con un file immagine
posizionale. Vedi il README per la tabella completa degli argomenti.

### 7.1 Cattura headless di finestre / componenti

Mark Shot può catturare **finestre specifiche — oppure un componente
(sottoregione) all'interno di una finestra — senza aprire alcuna UI**, da uno
script, una pipeline di build o un agente. Il processo termina non appena le
immagini vengono scritte o restituite, non crea mai una finestra, non apre mai
una finestra di dialogo e non ruba mai il focus, quindi l'utente può continuare
a lavorare mentre uno strumento cattura il desktop.

Per prima cosa elenca le finestre per vedere cosa è disponibile:

```bash
mark-shot --list-windows
```

Esempio di output (GNOME Wayland):

```json
{"count":2,"platform":"wayland","source":"compositor-script","windows":[
  {"index":0,"id":"0x3c00007","title":"Mark Shot - VSCodium","class":"codium","instance":"codium","x":1920,"y":0,"width":1680,"height":1050,"zOrder":1},
  {"index":1,"title":"Terminal","class":"org.gnome.Terminal","x":67,"y":32,"width":800,"height":600}
]}
```

Ogni voce contiene i campi a cui i selettori fanno corrispondenza: `index`, `id`
(id finestra X11 / id fornito dal backend), `title`, `class` e `instance`, più
`x`/`y`/`width`/`height` e un `zOrder` opzionale.

#### 7.1.1 Selezione delle finestre (singole o multiple)

`--window` può essere ripetuto per catturare **qualsiasi numero di finestre in
una singola chiamata**. Ogni selettore viene interpretato automaticamente
(`--window-by auto`):

| Valore del selettore     | Corrisponde a                                  |
| :---                     | :---                                           |
| `0`, `1`, …              | `index` dell'elenco                            |
| `0x3c00007`              | `id` della finestra                            |
| `VSCodium`               | `class` o `instance`, poi `title` (esatto, poi sottostringa) |
| `Mark Shot - VSCodium`   | `title`                                        |

Forza una singola regola di corrispondenza con `--window-by id|title|class|index`.
Un selettore che corrisponde a più finestre le cattura **tutte**.

Cattura un componente (una sottoregione all'interno di una finestra)
aggiungendo `@x,y,width,height` al selettore — l'offset è relativo all'angolo
superiore sinistro della finestra e viene limitato ai bordi della finestra:

```bash
# la striscia superiore di 100px della finestra 0
mark-shot --window "0@0,0,1680,100" --capture-destination file --capture-to /tmp/shots/
```

#### 7.1.2 Scelta della destinazione delle immagini

`--capture-destination` decide l'output; può essere combinato con un numero
qualsiasi di selettori `--window` e con una sottoregione componente:

| Destinazione | Comportamento |
| :--- | :--- |
| `inline` (default) | PNG in Base64 incorporati nell'output JSON. **Non viene scritto alcun file e gli appunti non vengono mai toccati.** La scelta più sicura per gli agenti che vogliono solo i pixel. |
| `file` | I file PNG vengono scritti in `--capture-to <directory>`; richiede tale opzione. |
| `stage` | I file PNG vengono scritti in una directory di staging temporanea (`$TMPDIR/mark-shot-staging`). Adatto a un flusso di lavoro "tieni per dopo". |
| `clipboard` | Le immagini vengono copiate negli appunti di sistema; con più immagini **vince l'ultima**. Il contenuto sopravvive alla chiusura del CLI (viene avviato un proprietario persistente `wl-copy` / `xclip`). |

Esempi:

```bash
# più finestre, salvate in una directory (un PNG per finestra)
mark-shot --window VSCodium --window Terminal --capture-destination file --capture-to /tmp/shots/

# una finestra più un componente di un'altra finestra, messe in staging per dopo
mark-shot --window "VSCodium@0,0,400,300" --window 1 --capture-destination stage

# selezione multipla, restituita come base64 senza toccare file o appunti
mark-shot --window 0 --window "Terminal" --capture-destination inline

# copia una finestra negli appunti
mark-shot --window 0 --capture-destination clipboard
```

**Criteri per gli appunti.** L'editor interattivo mette deliberatamente la tua
selezione negli appunti di sistema (l'azione `Copy` / `Ctrl+C`), perché è il
flusso di lavoro principale di uno strumento di screenshot. Le modalità
headless (il CLI e il server MCP enterprise) seguono la regola opposta:
**gli appunti non vengono mai modificati a meno che non venga scelto
esplicitamente `clipboard` come destinazione E le scritture negli appunti
siano abilitate in Impostazioni > Archiviazione > Modalità headless** —
`inline` (default) e `stage` lasciano intatto l'attuale contenuto degli appunti
dell'utente, quindi una cattura pianificata o guidata da un agente non può
sovrascrivere testo o immagini con cui l'utente sta lavorando altrove. Quando
una richiesta `clipboard` viene rifiutata perché le scritture headless negli
appunti sono disabilitate, la cattura ripiega sulla destinazione headless
predefinita configurata, l'output JSON (`"warning"`) e stderr te lo
comunicano, e il processo termina con un codice non zero così l'automazione può
rilevarlo. Abilitare le scritture headless negli appunti nelle impostazioni
richiede di digitare una passphrase di conferma.

L'output è un oggetto JSON `{"captures":[...]}` con una voce per ogni finestra
catturata; ogni voce ripete il selettore, l'identità della finestra e il
rettangolo di cattura finale, più un `path` (file/stage) oppure `data`
(inline) oppure nessuno dei due (clipboard). Il codice di uscita è `0` solo
quando ogni selettore ha corrisposto e ogni cattura è riuscita; una
corrispondenza mancante o una cattura fallita produce il codice di uscita `1`
con un campo `"error"` invece di un successo silenzioso.

La stessa pipeline di cattura può produrre un output annotato in modo
programmatico — vedi il capitolo sul server MCP dell'edizione enterprise,
oppure combina il PNG salvato con l'editor interattivo.

#### 7.1.3 Garanzia di non interferenza con le finestre

Ogni modalità headless è garantita invisibile e non invasiva:

- **non viene mai creata una finestra** — inclusi l'editor di annotazione,
  l'overlay di cattura e la tray; la cattura riutilizza il percorso di cattura
  headless;
- **non viene mai mostrata una finestra di dialogo** — incluse quelle di
  errore: gli errori vanno su stderr; persino le righe di comando malformate
  (ad esempio `--window-by` senza `--window`, una `--capture-destination`
  sconosciuta o file posizionali extra) terminano immediatamente con un codice
  non zero e un messaggio su stderr invece di aprire una `QMessageBox` o di
  ricadere nella UI interattiva;
- non compare alcun prompt del portale interattivo (`allowInteractivePortal` è
  disabilitato);
- il processo termina immediatamente dopo aver scritto l'output;
- l'elenco delle finestre catturato prima e dopo un'operazione headless è
  identico;
- le modalità headless non toccano mai gli appunti di sistema a meno che non
  venga richiesto esplicitamente `clipboard` **e** le scritture negli appunti
  siano abilitate in Impostazioni > Archiviazione > Modalità headless.

Se non viene rilevata alcuna finestra (ad esempio un helper del compositor
disabilitato o una sessione X11 senza enumerazione delle finestre), il comando
stampa un errore chiaro su stderr e termina con il codice `1` invece di
catturare silenziosamente nulla.

---

## 8. Tasti di scelta rapida del desktop e tray

La modalità tray (`mark-shot --tray`) registra `Ctrl+Alt+S` per la cattura
della regione e fornisce le voci di menu cattura / registrazione / impostazioni
/ esci. Tasti di scelta rapida del desktop:

- **GNOME**: Impostazioni → Tastiera → Scorciatoie → Scorciatoie personalizzate → collega a `mark-shot`.
- **KDE**: scorciatoia personalizzata collegata a `mark-shot` (più il permesso
  KWin ScreenShot2 per una cattura KDE esatta, vedi README).
- **Hyprland**: `bind = SUPER SHIFT, S, exec, mark-shot` e `bind = , Print, exec, mark-shot`.
- **niri**: `binds { Mod+Shift+S { spawn "mark-shot"; } }`.
- **Sway / i3**: `bindsym Mod4+Shift+S exec mark-shot`.

---

## 9. Configurazione e backend

- File di configurazione: `~/.config/mark-shot/config.json` (Linux), creato al
  primo avvio.
- Riferimento completo: [Configurazione](configuration.md).
- Backend: Wayland (portale PipeWire / grim / screencopy wlroots), X11
  (`QScreen::grabWindow`), Windows (WGC nativo). La registrazione preferisce il
  portale PipeWire e ripiega automaticamente.

Helper opzionali:

```bash
# OCR (RapidOCR / Tesseract)
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime

# Scansione codici (zxing-cpp)
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

---

## 10. Checklist di verifica delle funzionalità

Usa questa checklist per verificare una build da cima a fondo:

1. **Avvio** — `run-mark-shot.sh` apre l'overlay congelato.
2. **Hover sulle finestre** — sposta il mouse su una finestra: la cornice verde
   acqua lo segue; un singolo clic seleziona la finestra; con finestre
   sovrapposte viene scelta quella più in alto.
3. **Regione manuale** — trascina un rettangolo; rilascia; si apre l'editor.
4. **Annota** — disegna con ogni strumento (Pen, Line, Rectangle, Ellipse,
   Arrow, Highlighter, Text, Number, Mosaic, Magnifier, Laser); annulla/ripeti;
   Select per spostare/ridimensionare/ruotare/eliminare; doppio clic su un
   testo per modificarlo.
5. **Copia / Salva / Fissa / Carica** — `Ctrl+C`, `Ctrl+S`, `Ctrl+P`, `Ctrl+U`.
6. **Strumenti di avvio** — `C` color picker, `R` righello, `Q` scansione
   codici, `D` cattura display.
7. **Headless** — `--capture-to`, `--region`, `--display`, `--list-displays`.
8. **Cattura headless di finestre** — `--list-windows` elenca il desktop;
   ripeti `--window` per catturare più finestre; prova `--capture-destination`
   in tutte e quattro le modalità (inline, file, stage, clipboard); verifica un
   selettore di componente (`--window "0@0,0,400,300"`); conferma che l'elenco
   delle finestre prima e dopo sia invariato (non interferenza con le finestre).
9. **Tray + tasto di scelta rapida** — `mark-shot --tray`, premi `Ctrl+Alt+S`.
10. **Specifiche portabili** — il bundle trova le proprie lib/plugin/script Qt.

---

## 11. Feedback

Segnala i problemi con `gh issue create` usando la [guida all'invio dei
problemi](../../.doc/submit-issue-via-gh.md) inclusa nel bundle. Allega un log
di debug catturato con `mark-shot --debug --debug-log /tmp/mark-shot.log`.
