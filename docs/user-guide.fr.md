# Guide utilisateur de Mark Shot

Ce manuel couvre l'utilisation quotidienne de Mark Shot, avec un accent sur la
fonctionnalité de **sélection par survol des fenêtres / composants** (le
déplacement de la souris suit et met automatiquement en évidence la fenêtre
sous le curseur ; un clic la sélectionne), le flux de travail d'annotation, la
capture sans interface (headless) et la configuration.

> Les documentations de ce dépôt sont rédigées dans la fourche communautaire et
> sont répliquées vers les dépôts amont et entreprise. L'édition entreprise
> ajoute une section supplémentaire pour son serveur MCP local.

---

## 1. Démarrage rapide

### 1.1 Lancement

Démarrez une session de capture de région :

```bash
mark-shot
```

Appuyez sur un raccourci clavier du bureau (voir § 8) ou lancez-le depuis un
terminal. Une superposition plein écran figée s'ouvre sur le moniteur actif.
Déplacez la souris pour dessiner un rectangle de sélection, puis relâchez pour
entrer dans l'éditeur d'annotation.

### 1.2 Versions portables

Si vous utilisez une version portable (`mark-shot-upstream`,
`mark-shot-community`, `mark-shot-enterprise`), lancez-la avec le lanceur
inclus afin que les bibliothèques Qt, les plugins et les scripts d'aide
inclus soient trouvés :

```bash
portable/mark-shot-community/bin/run-mark-shot.sh
```

Le lanceur ajoute son répertoire `bin/` au début du `PATH`, ce qui est requis
pour les scripts d'aide à la détection de fenêtres
(`mark-shot-window-detection-*`) et les aides OCR / upload.

---

## 2. Sélection par survol des fenêtres / composants

Mark Shot peut détecter les fenêtres du bureau actuel avant que vous choisissiez
une région. Lorsque la superposition de sélection est ouverte, **déplacer la
souris met en évidence la fenêtre sous le curseur** avec un cadre turquoise.
**Un simple clic gauche (sans glisser) sélectionne toute cette fenêtre** comme
région de capture ; vous pouvez ensuite annoter, copier, épingler ou
l'enregistrer directement.

Les fenêtres mises en évidence proviennent d'un script de détection propre à
chaque compositeur qui s'exécute avant l'apparition de la superposition :

| Bureau | Source de détection | Remarques |
| :--- | :--- | :--- |
| GNOME Wayland | extension Shell `mark-shot-scroll-helper@snemc.org` incluse via D-Bus | nécessite que l'extension soit activée (voir § 2.1) |
| KDE Plasma Wayland | script KWin à usage unique via `qdbus6` / `qdbus` + journalctl | nécessite une session KWin |
| Hyprland | `hyprctl -j clients` | |
| niri | `niri msg -j windows` + analyse de la configuration | |
| X11 | énumération XCB intégrée au processus de `_NET_CLIENT_LIST_STACKING` | aucun script requis |
| Windows | `EnumWindows` intégré au processus | aucun script requis |

Seules les **fenêtres de premier niveau** sont suivies. Les widgets individuels
à l'intérieur d'une fenêtre (« composants ») ne sont pas exposés par les
compositeurs Wayland, de sorte que la sélection par survol cible des fenêtres
entières sur toutes les plateformes.

### 2.1 GNOME Wayland : activer l'extension d'aide

```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```

Vérifiez que l'aide D-Bus répond :

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
# -> ('5',)
```

Si l'appel échoue, déconnectez-vous puis reconnectez-vous (ou redémarrez GNOME
Shell sur X11) et réessayez. Sans l'extension, le script d'aide GNOME se
termine avec une erreur et la sélection par survol reste désactivée (la
sélection par glisser normale fonctionne toujours).

### 2.2 Comment l'utiliser

1. Déclenchez une capture (`mark-shot` ou le raccourci clavier du bureau).
2. Sans appuyer sur aucun bouton de la souris, déplacez le curseur sur une
   fenêtre. Un cadre turquoise délimite la fenêtre qui serait sélectionnée.
3. **Cliquez une fois** (appuyez et relâchez sans vous déplacer de plus de
   quelques pixels) pour sélectionner cette fenêtre. Si des fenêtres se
   chevauchent, la fenêtre la plus haute sous le curseur gagne (sensible à
   l'ordre z).
4. Relâcher ouvre l'éditeur d'annotation avec la fenêtre exactement encadrée.
5. Pour effectuer une région **manuelle**, faites simplement glisser un
   rectangle comme d'habitude — le cadre de survol est ignoré dès que le
   glissement dépasse le seuil de clic.

La mise en évidence par survol est désactivée pendant que l'outil de démarrage
Pipette de couleur (`C`) ou Règle (`R`) est actif, et reste disponible pour
le Scan de code (`Q`), la capture d'écran (`D`) et les modes de démarrage
d'enregistrement GIF / Vidéo.

### 2.3 Sélection des fenêtres sur le bon moniteur

La détection des fenêtres s'exécute par cible de capture. Sur une
configuration multi-moniteurs, chaque fenêtre figée ne reçoit que les fenêtres
qui croisent sa propre géométrie, de sorte que le cadre de survol correspond à
ce que vous voyez sur cet écran.

### 2.4 Activation / désactivation

La fonctionnalité est activée par défaut (`windowDetection.enabled = true`).
Basculez-la dans **Réglages → Avancé → Détection de fenêtre activée**, ou
modifiez `~/.config/mark-shot/config.json` :

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

- `command` : le script de détection. Sur GNOME / KDE / Hyprland / niri Wayland,
  le script inclus `mark-shot-window-detection-*` correspondant à votre session
  est choisi automatiquement ; sur X11 et Windows, la plateforme est énumérée
  dans le processus et `command` peut être laissé vide. **Une commande
  personnalisée fournie par l'utilisateur (par exemple un chemin absolu) est
  toujours respectée.**
- `timeoutMs` : temps d'attente maximal pour le script (100–30000 ms, défaut
  1000).
- `env` : variables d'environnement supplémentaires transmises au script. Les
  ajustements propres à chaque compositeur (décalages) sont documentés dans
  les en-têtes des scripts.

### 2.5 Résolution de problèmes

| Symptôme | Vérification |
| :--- | :--- |
| Pas de cadre turquoise sur GNOME Wayland | extension activée ? l'appel `gdbus` ci-dessus doit renvoyer une version |
| Pas de cadre turquoise sur X11 / Windows | aucune — l'énumération de la plateforme est intégrée ; assurez-vous que la session de capture n'utilise pas un outil pointeur de démarrage |
| Le cadre de survol sélectionne la mauvaise fenêtre (en dessous) | données d'ordre z manquantes dans un script de détection personnalisé ; les fenêtres sans `zOrder` sont classées dans la couche inférieure |
| La capture démarre lentement | le script de détection s'exécute avant la superposition ; augmentez `timeoutMs` uniquement si le bureau est lent, ou définissez `enabled:false` pour le sauter |
| Voir les diagnostics | exécutez `mark-shot --debug --debug-log /tmp/mark-shot.log` ; cherchez les lignes `window-detection` |

---

## 3. Sélection de région et outils de démarrage

Avant que la région soit validée, vous pouvez utiliser les outils de la
superposition de démarrage :

| Raccourci | Outil | Comportement |
| :---: | :--- | :--- |
| `C` | Pipette de couleur | Échantillonner un pixel ; la molette redimensionne la loupe ; le clic gauche ouvre un panneau de couleur (formats HEX / RGB / HSL / HSV / Qt) ; le clic droit ou `Esc` quitte |
| `R` | Règle | Le survol lit les coordonnées des pixels ; le glissement gauche mesure un rectangle avec largeur, hauteur, diagonale et aire ; le clic droit ou `Esc` quitte |
| `Q` | Scan de code | Glissez une région autour d'un QR code / code-barres ; le résultat décodé s'ouvre dans une fenêtre copiable |
| `D` | Capture d'écran | Capture toutes les sorties, recadre par écran, affiche des vignettes survolables (copier / modifier / enregistrer) |
| `S` | Arrêter l'enregistrement GIF / vidéo actif | arrête l'enregistrement affiché dans la superposition |

`Esc` annule la session ; le clic droit (sans outil de démarrage) annule
également.

---

## 4. Outils d'annotation

Après la sélection d'une région (ou l'ouverture d'une image locale), l'éditeur
s'ouvre avec la barre d'outils d'annotation. Les outils sont commutés avec les
touches numériques ou la barre d'outils :

| Raccourci | Outil | Description |
| :---: | :--- | :--- |
| `V` | Déplacer / Panoramique | déplacer toute la sélection, faire un panoramique sur le canevas d'une image locale |
| `S` | Sélection | sélectionner, déplacer, redimensionner, faire pivoter, supprimer des annotations existantes |
| `P` | Crayon | traits libres lissés |
| `L` | Ligne | lignes droites |
| `H` | Surligneur | marqueur semi-transparent ; style trait libre ou ligne droite |
| `R` | Rectangle | boîte avec styles `Contour` / `Surlignage` / `Inverser`, coins arrondis |
| `E` | Ellipse | ellipse / cercle |
| `A` | Flèche | flèches classiques (à empennage, KDE, bidirectionnelles) |
| `T` | Texte | texte enrichi ; la molette ou les curseurs redimensionnent ; les poignées diagonales redimensionnent les deux axes, les poignées latérales ajustent le retour à la ligne ; taille exacte en pt, famille de police, gras / italique dans le panneau de police |
| `N` | Numéro | marqueurs numérotés séquentiels (arabe, alpha, romain, chinois, …) |
| `M` | Mosaïque | flou givré acrylique pour masquer le contenu sensible |
| `G` | Laser | traits temporaires qui disparaissent automatiquement |

Conseils de dessin :

- Maintenez `Ctrl` pendant le dessin d'un rectangle / d'une ellipse pour
  contraindre à un carré / cercle.
- Faites défiler la molette pendant qu'un outil est actif pour ajuster la
  largeur du trait, la taille du texte, l'échelle des numéros ou la taille des
  blocs de mosaïque (aperçu en direct).
- Sous `Sélection`, faites défiler pour zoomer le canevas et maintenez le
  bouton du milieu pour faire un panoramique ; appuyez deux fois sur `Ctrl`
  pour réinitialiser.

### 4.1 Modification d'une annotation existante

Passez à **Sélection** (`S`). Cliquez sur une annotation pour afficher ses
poignées :

- faites glisser à l'intérieur pour déplacer ;
- faites glisser les poignées de coin / de bord pour redimensionner ;
- faites glisser la poignée ronde au-dessus du bord supérieur pour faire
  pivoter ;
- appuyez sur `Suppr` / `Retour arrière` pour supprimer ;
- double-cliquez sur un texte pour le modifier sur place.

Le panneau de propriétés (côté droit) modifie l'annotation sélectionnée :
couleur, largeur, style, famille / taille de police / gras / italique du texte.
Plusieurs annotations peuvent être sélectionnées en faisant glisser une boîte
de sélection sous l'outil `Sélection` ; le groupe peut ensuite être déplacé,
redimensionné, pivoté et supprimé ensemble.

### 4.2 Actions

| Raccourci | Action |
| :--- | :--- |
| `Ctrl+C` | copier dans le presse-papiers |
| `Ctrl+S` / `Entrée` | enregistrer (modèle de chemin depuis les réglages) |
| `Ctrl+P` | épingler comme fenêtre autocollante flottante |
| `Ctrl+U` | uploader vers l'hébergeur d'images configuré ; l'URL est copiée |
| `Ctrl+Z` / `Ctrl+Y` | annuler / rétablir |
| `F` | basculer la portée de capture (sélection ↔ plein écran) |

### 4.3 Cadre d'export

Activez **Réglages → Export → Cadre style Mac** pour ajouter un remplissage
transparent, des coins arrondis et une ombre douce aux images enregistrées /
copiées / uploadées.

---

## 5. Fenêtres autocollantes épinglées

| Geste / Raccourci | Comportement |
| :--- | :--- |
| glisser gauche | repositionner l'autocollant |
| molette | mise à l'échelle proportionnelle |
| double clic gauche / `Esc` | fermer |
| clic droit | menu contextuel (pivoter, zoomer, toujours au premier plan, copier le texte, traduire, enregistrer, copier, fermer) |

Le texte OCR à l'intérieur d'une fenêtre épinglée est sélectionnable et
copiable (`Ctrl+C` / menu contextuel). La traduction (point de terminaison
compatible OpenAI) rend le texte traduit sur l'image aux positions de mise en
page d'origine.

---

## 6. Capture à défilement

1. Sélectionnez une région (ou utilisez la poignée de glissement flottante pour
   les très grandes régions).
2. La superposition fait défiler la fenêtre cible ; les images capturées sont
   assemblées en une longue image.
3. GNOME Wayland nécessite l'extension Mark Shot Scroll Helper (§ 2.1).

La capture à défilement est prête pour la production sur niri et les
compositeurs wlroots/Wayland similaires ; sur KDE, X11 et d'autres piles, c'est
une fonctionnalité de test. Si elle échoue, utilisez des captures d'écran
normales ou une commande d'extension personnalisée.

---

## 7. Capture sans interface (CLI)

La capture non interactive écrit un PNG et affiche du JSON :

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

Toutes les options sans interface s'excluent mutuellement avec un fichier image
positionnel. Voir le README pour le tableau complet des arguments.

### 7.1 Capture sans interface de fenêtre / composant

Mark Shot peut capturer **des fenêtres spécifiques — ou un composant
(sous-région) à l'intérieur d'une fenêtre — sans ouvrir aucune interface**, à
partir d'un script, d'un pipeline de compilation ou d'un agent. Le processus se
termine dès que les images sont écrites ou renvoyées, et il ne crée jamais de
fenêtre, n'affiche jamais de boîte de dialogue et ne vole jamais le focus, de
sorte que l'utilisateur peut continuer à travailler pendant qu'un outil capture
le bureau.

Listez d'abord les fenêtres pour voir ce qui est disponible :

```bash
mark-shot --list-windows
```

Exemple de sortie (GNOME Wayland) :

```json
{"count":2,"platform":"wayland","source":"compositor-script","windows":[
  {"index":0,"id":"0x3c00007","title":"Mark Shot - VSCodium","class":"codium","instance":"codium","x":1920,"y":0,"width":1680,"height":1050,"zOrder":1},
  {"index":1,"title":"Terminal","class":"org.gnome.Terminal","x":67,"y":32,"width":800,"height":600}
]}
```

Chaque entrée contient les champs sur lesquels les sélecteurs s'appuient :
`index`, `id` (id de fenêtre X11 / id fourni par le backend), `title`, `class`
et `instance`, plus `x`/`y`/`width`/`height` et un `zOrder` facultatif.

#### 7.1.1 Sélection des fenêtres (une ou plusieurs)

`--window` peut être répété pour capturer **n'importe quel nombre de fenêtres
en un seul appel**. Chaque sélecteur est interprété automatiquement
(`--window-by auto`) :

| Valeur du sélecteur       | Correspond à                                        |
| :---                      | :---                                                |
| `0`, `1`, …               | `index` de la liste                                 |
| `0x3c00007`               | `id` de la fenêtre                                  |
| `VSCodium`                | `class` ou `instance`, puis `title` (exact, puis sous-chaîne) |
| `Mark Shot - VSCodium`    | `title`                                             |

Forcez une règle de correspondance avec `--window-by id|title|class|index`. Un
sélecteur qui correspond à plusieurs fenêtres les capture **toutes**.

Capturez un composant (une sous-région à l'intérieur d'une fenêtre) en ajoutant
`@x,y,width,height` au sélecteur — le décalage est relatif au coin supérieur
gauche de la fenêtre et est limité aux limites de la fenêtre :

```bash
# the top 100px strip of window 0
mark-shot --window "0@0,0,1680,100" --capture-destination file --capture-to /tmp/shots/
```

#### 7.1.2 Choix de la destination des images

`--capture-destination` décide de la sortie ; il peut être combiné avec
n'importe quel nombre de sélecteurs `--window` et une sous-région de composant :

| Destination | Comportement |
| :--- | :--- |
| `inline` (défaut) | PNG en Base64 intégrés dans la sortie JSON. **Aucun fichier n'est écrit et le presse-papiers n'est jamais touché.** Le choix le plus sûr pour les agents qui ne veulent que les pixels. |
| `file` | fichiers PNG écrits dans `--capture-to <directory>` ; requiert cette option. |
| `stage` | fichiers PNG écrits dans un répertoire de staging temporaire (`$TMPDIR/mark-shot-staging`). Idéal pour un flux de travail « conserver pour plus tard ». |
| `clipboard` | images copiées dans le presse-papiers du système ; avec plusieurs images, la **dernière gagne**. Le contenu survit à la fermeture de la CLI (un propriétaire persistant `wl-copy` / `xclip` est lancé). |

Exemples :

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

**Politique du presse-papiers.** L'éditeur interactif place délibérément votre
sélection dans le presse-papiers du système (l'action `Copier` / `Ctrl+C`), car
c'est le flux de travail principal d'un outil de capture d'écran. Les modes
sans interface (la CLI et le serveur MCP entreprise) suivent la règle opposée :
**le presse-papiers n'est jamais modifié, sauf si `clipboard` est explicitement
choisi comme destination ET que les écritures dans le presse-papiers sont
activées dans Réglages > Stockage > Mode sans interface** — `inline` (défaut)
et `stage` laissent le contenu actuel du presse-papiers de l'utilisateur
intact, de sorte qu'une capture planifiée ou pilotée par un agent ne peut pas
écraser le texte ou les images sur lesquels l'utilisateur travaille ailleurs.
Lorsqu'une requête `clipboard` est rejetée parce que les écritures sans
interface dans le presse-papiers sont désactivées, la capture retombe sur la
destination par défaut configurée pour le mode sans interface, la sortie JSON
(`"warning"`) et stderr vous en informent, et le processus se termine avec un
code non nul afin que l'automatisation puisse le détecter. L'activation des
écritures sans interface dans le presse-papiers dans les réglages nécessite de
saisir une phrase de confirmation.

La sortie est un objet JSON `{"captures":[...]}` avec une entrée par fenêtre
capturée ; chaque entrée répète le sélecteur, l'identité de la fenêtre et le
rectangle de capture final, plus un `path` (file/stage) ou `data` (inline) ou
aucun des deux (clipboard). Le code de sortie est `0` uniquement lorsque chaque
sélecteur a trouvé une correspondance et que chaque capture a réussi ; une
correspondance manquante ou une capture en échec produit le code de sortie `1`
avec un champ `"error"` au lieu d'un succès silencieux.

Le même pipeline de capture peut produire une sortie annotée par programme —
voir le chapitre du serveur MCP de l'édition entreprise, ou combinez le PNG
enregistré avec l'éditeur interactif.

#### 7.1.3 Garantie de non-interférence avec les fenêtres

Chaque mode sans interface est garanti invisible et non perturbant :

- **aucune fenêtre n'est jamais créée** — y compris l'éditeur d'annotation, la
  superposition de capture et la zone de notification ; la capture réutilise le
  chemin de capture sans interface ;
- **aucune boîte de dialogue n'est jamais affichée** — y compris les boîtes
  d'erreur : les erreurs vont sur stderr ; même les lignes de commande
  malformées (par exemple `--window-by` sans `--window`, une
  `--capture-destination` inconnue ou des fichiers positionnels
  supplémentaires) se terminent immédiatement avec un code non nul et un
  message sur stderr au lieu d'afficher un `QMessageBox` ou de basculer sur
  l'interface interactive ;
- aucune invite de portail interactive n'apparaît (`allowInteractivePortal`
  est désactivé) ;
- le processus se termine immédiatement après l'écriture de la sortie ;
- la liste des fenêtres capturée avant et après une opération sans interface
  est identique ;
- les modes sans interface ne touchent jamais le presse-papiers du système,
  sauf si `clipboard` a été explicitement demandé **et** que les écritures dans
  le presse-papiers sont activées dans Réglages > Stockage > Mode sans
  interface.

Si aucune fenêtre n'est détectée (par exemple un helper de compositeur
désactivé, ou une session X11 sans énumération de fenêtres), la commande
affiche une erreur claire sur stderr et se termine avec le code `1` au lieu de
capturer silencieusement rien.

---

## 8. Raccourcis clavier du bureau et zone de notification

Le mode zone de notification (`mark-shot --tray`) enregistre `Ctrl+Alt+S`
pour la capture de région et fournit des entrées de menu capture /
enregistrement / réglages / quitter. Raccourcis clavier du bureau :

- **GNOME** : Réglages → Clavier → Raccourcis → Raccourcis personnalisés → lier à `mark-shot`.
- **KDE** : raccourci personnalisé lié à `mark-shot` (plus la permission KWin
  ScreenShot2 pour une capture KDE exacte, voir le README).
- **Hyprland** : `bind = SUPER SHIFT, S, exec, mark-shot` et `bind = , Print, exec, mark-shot`.
- **niri** : `binds { Mod+Shift+S { spawn "mark-shot"; } }`.
- **Sway / i3** : `bindsym Mod4+Shift+S exec mark-shot`.

---

## 9. Configuration et backends

- Fichier de configuration : `~/.config/mark-shot/config.json` (Linux), créé au
  premier lancement.
- Référence complète : [Configuration](configuration.md).
- Backends : Wayland (portail PipeWire / grim / wlroots screencopy), X11
  (`QScreen::grabWindow`), Windows (WGC natif). L'enregistrement privilégie le
  portail PipeWire et retombe automatiquement.

Aides facultatives :

```bash
# OCR (RapidOCR / Tesseract)
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime

# Code scan (zxing-cpp)
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

---

## 10. Liste de contrôle des tests de fonctionnalités

Utilisez-la pour vérifier une compilation de bout en bout :

1. **Lancement** — `run-mark-shot.sh` ouvre la superposition figée.
2. **Survol de fenêtre** — déplacez la souris sur une fenêtre : le cadre
   turquoise suit ; un simple clic sélectionne la fenêtre ; les fenêtres qui se
   chevauchent choisissent la plus haute.
3. **Région manuelle** — faites glisser un rectangle ; relâchez ; l'éditeur
   s'ouvre.
4. **Annotation** — dessinez avec chaque outil (Crayon, Ligne, Rectangle,
   Ellipse, Flèche, Surligneur, Texte, Numéro, Mosaïque, Loupe, Laser) ;
   annuler / rétablir ; Sélection pour déplacer / redimensionner / pivoter /
   supprimer ; double-cliquez sur un texte pour le modifier.
5. **Copier / Enregistrer / Épingler / Uploader** — `Ctrl+C`, `Ctrl+S`,
   `Ctrl+P`, `Ctrl+U`.
6. **Outils de démarrage** — `C` pipette de couleur, `R` règle, `Q` scan de
   code, `D` capture d'écran.
7. **Sans interface** — `--capture-to`, `--region`, `--display`,
   `--list-displays`.
8. **Capture de fenêtre sans interface** — `--list-windows` liste le bureau ;
   répétez `--window` pour capturer plusieurs fenêtres ; testez
   `--capture-destination` dans les quatre modes (inline, file, stage,
   clipboard) ; vérifiez un sélecteur de composant
   (`--window "0@0,0,400,300"`) ; confirmez que la liste des fenêtres avant et
   après est inchangée (non-interférence avec les fenêtres).
9. **Zone de notification + raccourci** — `mark-shot --tray`, appuyez sur
   `Ctrl+Alt+S`.
10. **Spécificités portables** — le bundle trouve ses propres libs / plugins /
    scripts Qt.

---

## 11. Retour d'information

Signalez les problèmes avec `gh issue create` en utilisant le
[guide de soumission de problèmes](../.doc/submit-issue-via-gh.md) inclus.
Joignez un journal de débogage capturé avec
`mark-shot --debug --debug-log /tmp/mark-shot.log`.
