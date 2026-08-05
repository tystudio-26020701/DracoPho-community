<div align="center">
  <img src="../data/icons/DracoPho-logo.png" alt="DracoPho Logo" width="128" />
  <h1>DracoPho</h1>
  <p>
    <a href="https://github.com/tystudio-26020701/DracoPho-community/releases">
      <img src="https://img.shields.io/github/v/release/tystudio-26020701/DracoPho-community?color=6da0f2&labelColor=4a5054&label=release&style=flat-square&logo=github" alt="Release" />
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

Lisez ce README dans d'autres langues :
[简体中文](../README.zh-CN.md) · [繁體中文](./README.zh-TW.md) ·
[日本語](./README.ja.md) · [한국어](./README.ko.md) ·
[Русский](./README.ru.md) · [Italiano](./README.it.md) ·
[العربية](./README.ar.md) · [Français](./README.fr.md) ·
[Deutsch](./README.de.md) · [Español](./README.es.md) ·
[Português](./README.pt.md)

**Tags** : `C++` / `Qt 6` / `Capture d'écran` / `Annotation d'image` / `Épinglage sur le bureau` / `Reconnaissance OCR` / `Capture de défilement` / `Wayland` / `Windows`


<details>
<summary>Démonstration vidéo</summary>
<p align="center">
  <video src="https://github.com/user-attachments/assets/4f86fcee-fef9-409e-98ba-1491ecee06c7" width="100%" controls></video>
</p>
</details>

`mark-shot` est un outil haute performance de capture d'écran et d'annotation développé avec Qt 6. Initialement conçu pour les compositeurs Wayland comme `niri`, il prend désormais en charge les flux de travail standards de capture et d'annotation sous Linux (X11, GNOME, environnements de bureau wlroots/Wayland) ainsi que sous Windows.

Il capture l'écran instantanément et ouvre une couche d'annotation interactive plein écran, offrant aux utilisateurs le recadrage de zone, l'annotation, la copie dans le presse-papiers, l'enregistrement et l'épinglage sur le bureau.

---

## Fonctionnalités clés

### Boîte à outils d'annotation
- **Crayon et surligneur** : prend en charge le dessin fluide à main levée et le surlignage semi-transparent.
- **Outils géométriques vectoriels** : lignes, rectangles et ellipses de haute précision. Le rectangle prend en charge trois styles :
  - `Contour` : rectangle en contour ou rempli, avec coins arrondis optionnels.
  - `Surlignage` : effet surligneur rendu avec `CompositionMode_Multiply` et un remplissage semi-transparent.
  - `Inversion` : inverse les pixels RGB de la zone couverte par le rectangle, tout en conservant le contour externe comme repère visuel.
- **Flèche optimisée** : chemin de flèche classique à six sommets, aux bords lisses et rendu avec anti-crénelage.
- **Texte à double liaison** :
  - Prend en charge le réglage fluide de tailles de police très grandes, avec un zoom progressif via la molette de la souris ou les curseurs de propriétés.
  - Introduit un tampon de largeur physique pour éviter les retours à la ligne inattendus dus aux vibrations de rendu aux niveaux de zoom extrêmes.
  - **Les poignées diagonales** permettent une mise à l'échelle liée et proportionnelle de la taille de police et de la zone de texte ; **les lignes de bordure gauche/droite** ne règlent que la largeur de justification.
- **Pointeur laser de présentation** : destiné aux présentations ou à l'enseignement ; les traits se dissolvent en douceur au fil du temps.
- **Numérotation incrémentale** : cliquez pour placer des marqueurs numérotés en ordre croissant.
- **Mosaïque** : applique un flou verre dépoli sur les zones contenant des informations sensibles.
- **Loupe à deux cadres réglables indépendamment** : le viseur intérieur et la lentille extérieure de la loupe ont chacun leurs poignées de redimensionnement ; la lentille rectangulaire a 8 poignées d'angle/bord par cadre et la lentille circulaire 4 poignées (haut, bas, gauche, droite). Le redimensionnement d'un cadre ajuste l'autre en fonction du taux de zoom, qui reste toujours constant ; le déplacement d'un seul cadre laisse l'autre en place.
- **Scan de codes au démarrage** : avant la sélection, appuyez sur `Q` pour passer en mode scan ; après avoir encadré un code QR ou un code-barres, une fenêtre de résultats copiables s'ouvre.
- **Capture rapide d'écran** : avant la sélection, appuyez sur `D` pour capturer immédiatement tous les écrans de sortie, puis recadrez par écran en vignettes ; survolez une vignette pour copier, éditer ou enregistrer la capture de cet écran.
- **Enregistrement GIF, MP4 et WebP animé** : via les raccourcis d'enregistrement au démarrage ou le menu de la barre d'état système, enregistrez un écran donné ou une zone personnalisée en GIF, MP4 ou WebP animé. L'enregistrement est résistant aux plantages : le MP4 est écrit dans un MKV temporaire puis remuxé à la fin, si bien qu'un enregistrement interrompu reste récupérable. L'enregistrement actif affiche son statut dans la barre d'état système et sur l'image figée ; il peut être arrêté avec `S`, le bouton de la couche, le menu de la barre d'état système ou `--stop-recording`, et envoie des notifications de bureau au début et à la sauvegarde. Sous Wayland, l'enregistrement privilégie le backend PipeWire portal ; lorsque la capture par portal est indisponible, il peut revenir à wlroots screencopy ou à la capture par sondage.
- **Téléversement sur hébergeur d'images** : après la sélection, appuyez sur `Ctrl+U` ou cliquez sur le bouton de téléversement de la barre d'outils pour envoyer la capture à un hébergeur d'images personnalisé (ImgURL, sm.ms, imgbb, litterbox, etc.) ; une fois le téléversement réussi, l'URL est automatiquement copiée dans le presse-papiers. Les paramètres de l'hébergeur peuvent être configurés via `upload.env`, ou un script de téléversement personnalisé peut être branché via `upload.command`.
- **Cadre d'exportation style Mac** : ajoute des marges transparentes, des coins arrondis et une ombre douce aux images enregistrées, copiées, téléversées, ouvertes avec et envoyées aux commandes d'extension.

### Épinglage de fenêtres flottantes (Pin)
- Prend en charge l'épinglage d'une capture ou d'une zone annotée en tant que fenêtre flottante indépendante, sans bordure et toujours au premier plan.
- Prend en charge la sélection directe du texte reconnu par OCR dans la fenêtre épinglée, avec copie du texte de l'image via `Ctrl + C` ou le menu contextuel.
- Prend en charge la traduction du texte OCR via une interface compatible OpenAI appelant un LLM, avec rendu de la traduction superposé sur l'image épinglée à sa position d'origine.
- **Interactions pratiques** :
  - Glisser avec le bouton gauche de la souris pour déplacer librement l'image épinglée.
  - Faire défiler la molette de la souris pour la mettre à l'échelle proportionnellement.
  - Double-cliquer avec le bouton gauche ou appuyer sur `Esc` pour fermer l'image épinglée.
  - Cliquer avec le bouton droit pour ouvrir un menu offrant rotation, copie du texte de l'image, traduction, enregistrer sous, copie ou fermeture.

### Capture de défilement
- Capture des captures longues de pages ou de zones en combinant PipeWire screencast, une couche de défilement interactive et un assembleur d'images.
- Cette fonctionnalité s'adresse principalement à `niri` et aux environnements Wayland aux comportements similaires, où la géométrie de sortie, le minutage de capture et les positions des fenêtres restent plus stables.
- **Poignée flottante pour les grandes zones** : lorsque la zone sélectionnée est trop grande pour que l'espace restant de l'écran puisse afficher le panneau d'aperçu du défilement, le panneau d'aperçu se masque automatiquement et une **poignée flottante** (un petit bouton flottant avec des flèches directionnelles) s'affiche au bord de la zone.
  - **Faire glisser pour ajuster la zone** : maintenez et faites glisser la poignée flottante pour déplacer la zone de capture le long de l'axe de défilement et capturer du contenu au-delà de l'écran initial ;
  - **Cliquer pour changer d'axe** : avant de commencer la capture, cliquez sur la poignée flottante pour basculer directement la direction de défilement (verticale/horizontale).
- **Remarque sur la compatibilité** : la capture de défilement sous KDE, GNOME, X11 et dans d'autres environnements non-`niri` reste une fonctionnalité expérimentale et incomplète. Les stratégies des backends portal, le comportement du shell ou du gestionnaire de fenêtres, le retour de géométrie des fenêtres, le minutage des trames et la gestion des événements de défilement diffèrent d'une pile de bureau à l'autre.
- Si la capture de défilement n'est pas disponible, utilisez le flux de capture normal ou branchez un outil externe de capture longue via les commandes d'extension de DracoPho.
- Pour signaler un problème de capture de défilement, exécutez d'abord `mark-shot --debug --debug-log /path/to/mark-shot.log` et reproduisez le problème, puis joignez le journal à votre problème GitHub. La journalisation peut aussi être activée via `debug.enabled` et `debug.logPath` dans `config.json` ; `DEBUG=1` et `MARK_SHOT_DEBUG_LOG=/path/to/log` restent disponibles.

### Prise en charge multi-serveurs d'affichage
- **Wayland** : utilise PipeWire portal screencast pour l'enregistrement et la capture de défilement expérimentale, en gérant les deux types de trames (mémoire partagée et DMA-BUF) ; utilise `grim` pour la capture d'écran wlroots, `layer-shell-qt` pour la couche native et `wl-copy` pour la persistance du presse-papiers.
- **X11** : utilise `QScreen::grabWindow` pour la capture d'écran, une fenêtre plein écran toujours au premier plan comme couche, et `xclip` pour la persistance du presse-papiers.
- **Windows** : utilise les API natives de capture d'écran et de presse-papiers de Qt pour les flux de base : capture, annotation, copie, enregistrement et épinglage. Les backends propres à Linux tels que PipeWire, xdg-desktop-portal, `grim`, la détection de fenêtres XCB, LayerShellQt et les assistants GNOME Shell sont désactivés à la compilation.
- Le backend du serveur d'affichage Linux est détecté automatiquement à l'exécution via `$XDG_SESSION_TYPE` ; Windows utilise le backend de plateforme natif de Qt.
- **Multi-monitor freeze scope** : par défaut, la sélection d'une région gèle tous les écrans connectés (une seule fenêtre de bureau virtuel lorsque les DPR correspondent sous X11/Windows) ; après avoir validé une sélection sur un écran, les autres écrans restent gelés et non interactifs jusqu'à la fin de la session. La portée **Cursor Screen** ne gèle que l'écran sous le curseur.

### Intégration au bureau
- **Comportement de démarrage configurable**: le lancement de DracoPho n'ouvre plus l'overlay de capture par défaut. Dans Paramètres → Général → **Comportement au démarrage**, combinez les quatre modes : **Icône dans la barre**, **Boule flottante**, **Fenêtre des paramètres** et **Capture directe**. Les nouvelles installations utilisent Icône dans la barre + Boule flottante ; la capture directe est optionnelle.
- **Boule flottante**: un petit widget circulaire toujours au premier plan (en bas à droite par défaut) donnant un accès rapide à la capture, la capture plein écran, l'enregistrement et les paramètres. Un clic ouvre le menu, un double-clic capture, le glisser la déplace (la position est mémorisée) et elle se masque automatiquement pendant une capture.
  Déposée près d'un bord de l'écran, elle s'y aimante et **disparaît à moitié hors de l'écran** (le survol la fait réapparaître, l'éloignement de la souris la masque à nouveau ; fonctionne sur toutes les plateformes) ; après quelques secondes d'inactivité, elle devient semi-transparente et retrouve son opacité totale au survol.
- **Raccourcis du bureau** :
  - `mark-shot.desktop` : configuré comme outil de capture d'écran global du système, invocable directement via un raccourci système.
  - `mark-shot-edit.desktop` : enregistré comme éditeur d'images indépendant, intégrable au menu contextuel « Ouvrir avec » des gestionnaires de fichiers (Dolphin, Nautilus, etc.).
- Fourni avec les icônes vectorielles système haute résolution `dracoPho.svg` et `mark-shot-edit.svg`.

### Autorisation KDE KWin ScreenShot2

Sous KDE Wayland, DracoPho peut utiliser l'interface `org.kde.KWin.ScreenShot2` de KWin pour effectuer des captures de zone précises. KWin traite cette interface comme une interface D-Bus restreinte : le fichier de bureau de l'application doit donc déclarer le champ d'autorisation.

<details>
<summary>Explications sur l'autorisation KDE KWin ScreenShot2 et la configuration du fichier de bureau (cliquer pour déplier)</summary>

Déclarez le champ d'autorisation :
```ini
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

Les paquets des distributions et `cmake --install` installent automatiquement les fichiers de bureau requis. Si vous exécutez directement un binaire issu d'une compilation locale sans installer le projet, créez ou mettez à jour `~/.local/share/applications/mark-shot.desktop` :

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

Si vous liez DracoPho via le service de raccourcis de commandes de KDE, créez également `~/.local/share/applications/net.local.mark-shot.desktop` :

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

Après avoir modifié les fichiers de bureau, il est recommandé de se déconnecter puis de se reconnecter pour que KDE relise le cache des fichiers de bureau. Si la session KDE actuelle renvoie toujours `NoAuthorized`, redémarrez KWin ou redémarrez le système une fois.
</details>

---

## Comparaison de produits

DracoPho édition communautaire est un outil tout-en-un **open source (MIT), multiplateforme (Linux X11/Wayland natif + Windows) et entièrement hors ligne** de capture d'écran, d'annotation, d'épinglage, d'OCR, de traduction et d'enregistrement. Les tableaux ci-dessous sont établis à partir de la documentation officielle et du site officiel de chaque produit (au **août 2026**) et couvrent les outils de capture d'écran les plus populaires — open source et commerciaux — sur toutes les grandes plateformes de bureau. Les capacités sont indiquées fidèlement : **✅ prise en charge intégrée** ; **⭕ partielle (limitée par l'abonnement payant, la plateforme ou un outil/service externe)** ; **❌ non disponible**. Elles sont comptées selon le critère « prêt à l'emploi » — sans être liées à un abonnement payant, à un service cloud ou à une branche expérimentale ; les limitations ⭕ exactes sont expliquées dans les notes détaillées ci-dessous.

### Aperçu des capacités principales

#### I. Capture (pouvez-vous obtenir ce dont vous avez besoin)

| Capacité | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Capture de région | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Capture plein écran / multi-écrans | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Capture de fenêtre | ⭕ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Contenu de fenêtre masquée / réduite | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Capture longue (défilement) | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ⭕ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Capture programmée / différée | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⭕ | ❌ | ✅ |

#### II. Annotation et intelligence (pouvez-vous travailler efficacement après la capture)

| Capacité | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Boîte à outils d'annotation | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Épinglage à l'écran (Pin) | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ |
| Reconnaissance de texte OCR | ✅ | ✅ | ✅ | ⭕ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Traduction | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| Reconnaissance QR / code-barres | ✅ | ✅ | ✅ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ⭕ |
| Pipette (sélecteur de couleur) | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ✅ | ✅ | ✅ | ✅ |

#### III. Sortie et automatisation (pouvez-vous partager / archiver le résultat)

| Capacité | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Enregistrement d'écran | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ |
| Sortie GIF / WebP animé | ✅ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ |
| Téléversement d'images / cloud | ✅ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ |
| CLI sans interface / scripts | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| Boule flottante d'accès rapide | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Historique des captures | ❌ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ | ⭕ | ❌ | ✅ | ✅ | ⭕ | ❌ | ⭕ |

#### IV. Plateforme et écosystème

| Capacité | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Wayland natif Linux | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Multiplateforme (≥2 OS de bureau) | ⭕ | ❌ | ⭕ | ✅ | ✅ | ✅ | ❌ | ⭕ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ |
| Mécanisme de plugins / d'extensions | ✅ | ⭕ | ❌ | ❌ | ⭕ | ✅ | ⭕ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| Open source / gratuit | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ✅ | ⭕ | ✅ | ❌ | ❌ | ✅ | ⭕ | ⭕ |

### Détails des fonctionnalités

**I. Capture** (pouvez-vous obtenir ce dont vous avez besoin)

- **Capture de région / plein écran / multi-écrans** — intégrée aux 15 outils listés ; DracoPho gère en plus correctement les écrans à coordonnées négatives et la mise à l'échelle HiDPI indépendante par écran, sans effets fantômes lors de l'assemblage des captures multi-écrans.
- **Capture de fenêtre** — le mode interactif de DracoPho utilise un calque de sélection plein écran (pas de mode dédié « fenêtre active » en un clic, d'où le ⭕), tandis que son mode sans interface (`--window`) sélectionne précisément les fenêtres par id, titre, classe, **PID ou nom de processus** ; Flameshot n'a aucun mode fenêtre, tous les autres outils en ont un.
- **Contenu de fenêtre masquée / réduite** — sous X11, DracoPho lit le tampon composite de la fenêtre via XComposite, capturant le contenu réel même lorsque la fenêtre est entièrement masquée ou réduite (sans la relever ni voler le focus) ; parmi les outils similaires, seul le `--stack` de scrot sait le faire. Le protocole Wayland empêche tout outil de lire le contenu des fenêtres réduites.
- **Capture longue (défilement)** — DracoPho fonctionne en natif sur niri et GNOME Wayland (extension officielle), KDE/X11 restant expérimental. ShareX, PixPin, PicPick, Snagit, CleanShot X, Shottr, Xnip et iShot l'intègrent nativement ; Greenshot seulement pour les scénarios IE hérités ; Snipaste, Flameshot, ksnip, Spectacle et l'outil Capture d'écran de Windows ne la prennent pas en charge.
- **Capture programmée / différée** — DracoPho attend le nombre de secondes configuré avec un compte à rebours plein écran (Échap pour annuler) avant d’entrer en capture, via `--delay`, le sous-menu « Capture différée » du plateau/ballon flottant ou la page de réglages de capture ; Flameshot, ksnip, Spectacle et la plupart ne proposent qu’un départ différé simple, ShareX une capture programmée, et PixPin, Snipaste et Xnip rien.

**II. Annotation et intelligence** (pouvez-vous travailler efficacement après la capture)

- **Boîte à outils d'annotation** — DracoPho intègre plus de 12 outils : crayon, surligneur, ligne, rectangle, ellipse, flèche, texte, numérotation des étapes, mosaïque, loupe à double cadre, pointeur laser, pipette et règle ; les 15 outils disposent d'une annotation intégrée de profondeur variable.
- **Épinglage à l'écran (Pin)** — DracoPho propose des autocollants sans bordure toujours au premier plan avec mise à l'échelle, rotation, sélection de mots OCR et traduction LLM sur place ; Snipaste, PixPin, ShareX, ksnip, CleanShot X, Shottr, Xnip et iShot l'intègrent aussi ; Flameshot, Spectacle, Greenshot, PicPick, l'outil Capture d'écran de Windows et Snagit ne le proposent pas.
- **Reconnaissance de texte OCR** — DracoPho intègre RapidOCR (modèles PP-OCR hors ligne) avec repli sur Tesseract, prêt à l'emploi ; l'outil Capture d'écran de Windows (Actions de texte locales), ShareX, Snagit, CleanShot X, Shottr, Xnip et iShot l'intègrent ; Snipaste le réserve à un abonnement payant, ksnip via un plugin et Spectacle via une installation externe de Tesseract ; Flameshot, Greenshot et PicPick n'en ont pas.
- **Traduction** — DracoPho intègre la traduction de capture par LLM compatible OpenAI (hors ligne / auto-hébergement pris en charge). Seul iShot intègre la traduction de capture, PixPin la réserve aux membres et aucun autre outil ne la propose. C'est l'une des capacités de différenciation les plus marquantes.
- **Reconnaissance QR / code-barres** — DracoPho scanne les QR, les codes à une dimension et le PDF417 avec `Q` pendant la capture ; ShareX et PixPin la prennent en charge ; Snipaste (payant) ainsi que CleanShot X / Shottr / iShot ne lisent que le QR, via leurs moteurs OCR.
- **Pipette (sélecteur de couleur)** — DracoPho dispose d'une pipette d'écran intégrée avec palette d'historique ; la plupart des outils en ont une, sauf Spectacle et Snagit ; l'outil Capture d'écran de Windows seulement sur les PC IA Copilot+.
- **Masquage IA en un clic** — le masquage automatique ajouté par la capture WeChat en 2026 n'est pas encore intégré (❌) ; la mosaïque / le flou de DracoPho restent manuels. Le masquage IA de Snagit (AI Smart Redact) et le masquage de texte de l'outil Capture d'écran de Windows sont des implémentations comparables ; le masquage automatisé est un complément possible.

**III. Sortie et automatisation** (pouvez-vous partager / archiver le résultat)

- **Enregistrement d'écran** — DracoPho enregistre en MP4 / GIF / WebP animé avec un enregistrement silencieux et sans surveillance de région / d'écran (sans fenêtre, sans dialogue, sans vol de focus) ; ShareX, PixPin, Spectacle (Plasma 6), PicPick, l'outil Capture d'écran de Windows, Snagit, CleanShot X et iShot l'intègrent ; Snipaste, Flameshot, ksnip, Greenshot, Shottr et Xnip non.
- **Sortie GIF / WebP animé** — DracoPho enregistre nativement en **WebP animé** (fichiers plus petits, prise en charge de l'alpha) ; aucun concurrent ne produit de WebP animé nativement. PixPin et iShot enregistrent en GIF, ShareX enregistre en GIF (sauvegarde statique WebP uniquement), Snagit et CleanShot X exportent en GIF ; les autres n'ont pas de sortie animée.
- **Téléversement d'images / cloud** — DracoPho intègre le téléversement vers ImgURL / sm.ms / imgbb / litterbox / commande personnalisée (`Ctrl+U`) ; ShareX a le plus grand nombre de cibles ; Greenshot, PicPick, Snagit et CleanShot X ont un téléversement cloud intégré ; Flameshot seulement Imgur, ksnip Imgur/FTP/scripts, Shottr S3 après activation ; Snipaste, PixPin, l'outil Capture d'écran de Windows, Xnip et iShot n'en ont pas.
- **CLI sans interface / scripts** — DracoPho fournit une chaîne sans interface complète : `--capture-to` pour les captures de région / d'écran, `--list-windows` + `--window` pour les captures de fenêtre / de composant, `--record-region` / `--record-display` pour l'enregistrement sans surveillance et `--record-wait-json` pour la sortie d'état structurée — le tout sans fenêtre, sans dialogue ni vol de focus, avec **ciblage par PID / nom de processus et capture des fenêtres masquées / réduites**. Le serveur MCP de l'édition commerciale réutilise la même chaîne. ShareX (Windows uniquement), Flameshot, ksnip et Spectacle disposent de CLI solides ; la CLI de Snipaste est une fonctionnalité d'abonnement payant ; PixPin ne propose qu'un moteur d'actions JS.
- **Boule flottante d'accès rapide** — DracoPho dispose d'une boule déplaçable qui s'aime aux bords de l'écran (X11/Windows/macOS ; Wayland la laisse flotter librement en raison des limites du protocole) et devient semi-transparente au repos ; seul PixPin propose quelque chose de similaire.
- **Historique des captures** — DracoPho n'a pas encore de panneau d'historique dédié ; ShareX, Snipaste, Flameshot, Snagit et CleanShot X en ont un, ksnip / PicPick / Shottr / iShot ont des alternatives légères (onglets / galerie / réserve épinglée), les autres n'en ont pas.

**IV. Plateforme et écosystème**

- **Wayland natif Linux** — DracoPho prend en charge nativement PipeWire portal, grim, layer-shell, KDE KWin ScreenShot2 et les extensions GNOME ; parmi les outils open source, seul Spectacle (KDE) atteint ce niveau de profondeur, tandis que Flameshot / ksnip restent expérimentaux ou dépendants du portal.
- **Multiplateforme** — Snipaste, Flameshot, ksnip et Snagit couvrent les trois principaux OS de bureau (Windows + macOS + Linux) ; DracoPho couvre Linux + Windows (macOS prévu) ; ShareX, PicPick, l'outil Capture d'écran de Windows, Spectacle, CleanShot X, Shottr, Xnip et iShot sont mono-plateforme.
- **Mécanisme de plugins / d'extensions** — DracoPho fournit un système de plugins Qt avec un marché de plugins GitHub (providers OCR / traduction / scan de codes extensibles) ; ksnip et Greenshot ont des API de plugins ; ShareX, Spectacle, Snagit, CleanShot X et Shottr les remplacent par des actions personnalisées / intégrations.
- **Open source / gratuit** — DracoPho CE est sous licence MIT, entièrement gratuit, sans publicité, sans compte et sans nécessité de connexion réseau ; ShareX, Flameshot, ksnip, Spectacle et Greenshot (Windows) sont aussi open source et gratuits ; Shottr et l'outil Capture d'écran de Windows sont gratuits ; PixPin / Snipaste / PicPick / Xnip / iShot sont des freemium à code source fermé ; Snagit / CleanShot X sont des logiciels commerciaux payants.

> Note : comparaison établie à partir du site officiel et de la documentation officielle de chaque produit (2026-08) ; les capacités évoluent avec chaque version, veuillez vous référer à la documentation la plus récente. FastStone Capture, Nimbus, Lightshot, la capture Sogou et les captures d'écran WeChat / QQ ne figurent pas dans le tableau ci-dessus (le masquage IA en un clic de WeChat en 2026 est leader sur ce segment).

---

## Interface en ligne de commande (CLI)

### Exemples d'utilisation courante

```bash
# Capture l'écran et passe en mode recadrage et annotation
mark-shot

# Capture tous les écrans de sortie en configuration multi-écrans
mark-shot --all-outputs

# Ignore l'étape de sélection et annote directement la capture plein écran
mark-shot --fullscreen

# Après la sélection, outil par défaut Move ; en plein écran, pointeur laser par défaut, avec une couleur par défaut rouge
mark-shot --default-tool move --fullscreen-default-tool laser --default-color '#FF4D4D'

# Ouvre un fichier image local existant et passe directement en mode annotation
mark-shot path/to/image.png

# Ouvre directement une image locale en tant que fenêtre épinglée
mark-shot --pin-image path/to/image.png

# Force l'utilisation d'une fenêtre normale plein écran XDG standard (au lieu de la couche layer-shell Wayland)
mark-shot --xdg-window
```

#### Capture sans interface (non interactive)

Les scripts, l'automatisation CI ou d'autres programmes peuvent appeler `mark-shot` pour effectuer des captures sans ouvrir l'interface d'annotation.
La trame capturée est écrite en PNG et un résumé JSON compact est imprimé sur la sortie standard :

```bash
# Capture l'écran principal et écrit un PNG
mark-shot --capture-to /tmp/shot.png

# Écrit dans un répertoire (nom de fichier horodaté généré automatiquement)
mark-shot --capture-to /tmp/shots/

# Capture une zone logique de l'écran (x,y,largeur,hauteur)
mark-shot --capture-to /tmp/region.png --region 0,0,1280,720

# Capture un écran précis par son nom, avec le curseur de la souris
mark-shot --capture-to /tmp/window.png --display DP-1 --include-cursor

# Capture plusieurs écrans en même temps (--display répétable, un PNG par écran)
mark-shot --capture-to /tmp/shots/ --display DP-1 --display DP-2

# Affiche en JSON les informations de tous les écrans puis quitte
mark-shot --list-displays
```

Exemple de sortie JSON pour un `--capture-to` mono-écran :

```json
{"path":"/tmp/shot.png","width":2560,"height":1440,"output":"DP-1","error":null}
```

Lorsque plusieurs `--display` sont spécifiés, la sortie devient un tableau avec une capture par écran :

```json
{"captures":[{"path":"/tmp/shots/mark-shot-DP-1-20260801-000000.png","width":2560,"height":1440,"output":"DP-1","error":null},
             {"path":"/tmp/shots/mark-shot-DP-2-20260801-000000.png","width":1920,"height":1080,"output":"DP-2","error":null}]}
```

Chaque écran sélectionné est capturé avec sa propre géométrie source : les backends de type portal
renvoient donc précisément cet écran et non tout le bureau virtuel.

La capture sans interface réutilise exactement les mêmes backends de capture que l'interface interactive (QScreen,
xdg-desktop-portal, PipeWire, grim, assistants KWin/GNOME, Windows Graphics Capture) :
la qualité d'image et le comportement de recadrage sont donc parfaitement identiques. Tous les paramètres sans interface
sont mutuellement exclusifs avec le paramètre de fichier image positionnel.

### Description des paramètres CLI

| Option | Description |
| :--- | :--- |
| `[file]` | **Paramètre positionnel** : ouvre un fichier image local existant en mode annotation, au lieu de capturer l'écran courant. |
| `-h`, `--help` | Affiche l'aide et quitte. |
| `-v`, `--version` | Affiche la version actuelle et quitte. |
| `--all-outputs` | Capture tous les écrans de sortie du bureau virtuel, au lieu de seulement l'écran actif. |
| `--xdg-window` | Force l'utilisation d'une fenêtre normale plein écran XDG (xdg-shell) au lieu de la couche Wayland (layer-shell) par défaut. |
| `--fullscreen` | Ignore l'étape de sélection et annote directement la capture plein écran. |
| `--default-tool <tool>` | Définit l'outil d'annotation par défaut après la sélection normale ; sert aussi d'outil par défaut en mode plein écran si `--fullscreen-default-tool` n'est pas défini. |
| `--fullscreen-default-tool <tool>` | Définit l'outil par défaut du mode d'annotation plein écran. |
| `--default-color <color>` | Définit la couleur d'annotation par défaut. Prend en charge `#RRGGBB` et `#RRGGBBAA`. |
| `--tray` | Maintient DracoPho en cours d'exécution dans la barre d'état système et enregistre le raccourci global de capture d'écran lorsque la plateforme le prend en charge. |
| `--capture` | Déclenche de force une capture unique lorsque le lancement automatique dans la barre d'état système est activé dans la configuration. |
| `--pin-image <path>` | Ouvre directement une image locale en tant que fenêtre épinglée, en ignorant les flux de capture et de sélection. |
| `--recording-status` | Affiche en JSON l'état actuel de l'enregistrement via l'instance en cours d'exécution. |
| `--stop-recording` | Demande à l'instance en cours d'exécution d'arrêter l'enregistrement actif. |
| `--record-region <x,y,w,h>` | Enregistre une région de l'écran via l'instance en cours d'exécution ; géométrie au format x,y,largeur,hauteur. |
| `--record-display <id>` | Enregistre un écran par identifiant (voir la sortie de `--list-displays` : un nom d'écran brut tel que `DP-1`, la clé `output:DP-1` ou `all`). |
| `--record-output <path>` | Chemin du fichier de sortie de l'enregistrement (obligatoire avec les drapeaux d'enregistrement). |
| `--record-duration <seconds>` | Durée de l'enregistrement en secondes ; 0 enregistre jusqu'à `--stop-recording`. |
| `--record-fps <n>` | Fréquence d'images de l'enregistrement (15 par défaut). |
| `--record-format <mp4\|gif\|webp>` | Format d'enregistrement : mp4 (par défaut), gif ou webp. |
| `--record-audio` | Inclut l'audio système dans l'enregistrement. |
| `--record-wait-json` | Attend la fin de l'enregistrement, puis affiche le statut final en JSON. |
| `--debug` | Active les journaux de débogage pour cette exécution. |
| `--no-debug` | Désactive les journaux de débogage pour cette exécution, en ignorant le fichier de configuration et les variables d'environnement. |
| `--debug-log <path>` | Écrit les journaux de débogage dans le chemin spécifié ; active le débogage sauf si `--no-debug` est aussi défini. |
| `--capture-to <path>` | Capture sans interface : écrit le PNG dans le fichier ou répertoire spécifié sans ouvrir l'interface ; imprime un résumé JSON sur la sortie standard. |
| `--region <x,y,w,h>` | À utiliser avec `--capture-to` : ne capture que la zone logique de l'écran spécifiée. |
| `--display <name>` | À utiliser avec `--capture-to` : capture l'écran de sortie spécifié par son nom. Répétable pour capturer plusieurs écrans en une fois (un PNG par écran). |
| `--include-cursor` | À utiliser avec `--capture-to` : dessine le curseur de la souris dans la trame capturée. |
| `--output-name <name>` | À utiliser avec `--capture-to` : nom de base du fichier (sans extension) lorsque le chemin de capture est un répertoire. |
| `--list-displays` | Affiche en JSON les informations de tous les écrans puis quitte. |

### Liaison des raccourcis

Liez `mark-shot` comme raccourci de capture d'écran du système :

**niri** (modifiez `~/.config/niri/config.kdl`) :
```kdl
binds {
    Mod+Shift+S { spawn "mark-shot"; }
}
```

**Hyprland** (modifiez `~/.config/hypr/hyprland.conf`) :
```ini
# Lie Super+Shift+S pour lancer la capture par sélection de mark-shot
bind = SUPER SHIFT, S, exec, mark-shot
# Lie la touche Impr. écran pour lancer la capture par sélection de mark-shot
bind = , Print, exec, mark-shot
```

**Sway / i3** (modifiez `~/.config/sway/config` ou `~/.config/i3/config`) :
```ini
# Lie Super+Shift+S pour lancer la capture par sélection de mark-shot
bindsym Mod4+Shift+S exec mark-shot
# Lie la touche Impr. écran pour lancer la capture par sélection de mark-shot
bindsym Print exec mark-shot
```

**GNOME** : ajoutez-le dans Paramètres système → Clavier → Raccourcis clavier → Raccourcis personnalisés.

**Mode barre d'état système** :
```powershell
mark-shot --tray
```

Le mode barre d'état système enregistre par défaut le raccourci global suivant :
- `Ctrl+Alt+S` : lance la capture par sélection.

Le menu de la barre d'état système propose également la capture, la capture plein écran, le démarrage de l'enregistrement, l'état de l'enregistrement, l'arrêt de l'enregistrement, les paramètres et la sortie.


### Commandes d'extension

La barre d'outils d'actions de droite propose un bouton **Extensions** ; le programme lit les commandes personnalisées depuis `~/.config/mark-shot/extensions.json`. Le fichier de configuration peut être un tableau JSON ou un objet JSON contenant un tableau `commands`.

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

`command` est exécuté via `$SHELL -c` sur les systèmes de type Unix et via `%COMSPEC% /C` sous Windows, ce qui prend en charge les expressions shell. Utilisez `{slurp}` pour transmettre la zone actuelle comme chaîne géométrique `x,y largeurxhauteur` à la commande. Utilisez `{image}` ou `{imagePath}` pour transmettre la zone actuellement rendue comme chemin PNG temporaire à la commande, et `{imageUrl}` pour transmettre une URL `file://`. Ces espaces réservés sont automatiquement échappés pour le shell : n'ajoutez pas de guillemets supplémentaires dans la configuration. Si aucun espace réservé d'image n'est utilisé, vous pouvez définir `saveImage` ou `needsImage` sur `true` : le programme ajoutera automatiquement le chemin PNG temporaire à la fin de la commande. `workingDirectory` équivaut à `cwd`. La valeur par défaut de `closeOnStart` est `true` : DracoPho est masqué puis fermé avant le lancement de la commande.

### Fichier de configuration de l'application

Voir la [référence de configuration](../docs/configuration.zh-CN.md).

### Guide d'utilisation

Pour les opérations quotidiennes (sélection en survol de fenêtre, outils d'annotation, outils de démarrage, fenêtres autocollants, captures longues, CLI sans interface
et liste de contrôle d'auto-test), voir le [Guide d'utilisation](../docs/user-guide.zh-CN.md)
([English](../docs/user-guide.md)).

Autres langues :
[简体中文](../docs/user-guide.zh-CN.md) · [繁體中文](../docs/user-guide.zh-TW.md) ·
[日本語](../docs/user-guide.ja.md) · [한국어](../docs/user-guide.ko.md) ·
[Русский](../docs/user-guide.ru.md) · [Italiano](../docs/user-guide.it.md) ·
[العربية](../docs/user-guide.ar.md) · [Français](../docs/user-guide.fr.md) ·
[Deutsch](../docs/user-guide.de.md) · [Español](../docs/user-guide.es.md) ·
[Português](../docs/user-guide.pt.md)

## Compilation et installation

### Guide d'installation

##### Arch Linux (AUR)
Les utilisateurs d'Arch Linux peuvent installer directement via un assistant AUR :
```bash
# Compile et installe à partir des sources
paru -S mark-shot
# ou
yay -S mark-shot

# Installe le paquet binaire précompilé
paru -S mark-shot-bin
# ou
yay -S mark-shot-bin
```

`mark-shot` est compilé à partir des sources ; `mark-shot-bin` télécharge le paquet pacman précompilé depuis GitHub Releases.

##### NixOS
Les utilisateurs de NixOS peuvent l'installer en ajoutant une entrée Flake :
```nix
# flake.nix
mark-shot = {
  url = "github:tystudio-26020701/DracoPho-community";
  inputs.nixpkgs.follows = "nixpkgs";
};

# home-manager
home.packages = with pkgs; [
  # autres applications utilisateur
  inputs.mark-shot.packages.${pkgs.stdenv.hostPlatform.system}.default
]
```

##### Autres distributions (paquets précompilés)
Pour les autres distributions (comme Ubuntu, Debian, Fedora), téléchargez le paquet compilé sur la page Releases puis exécutez les commandes suivantes pour l'installer :
- **Debian / Ubuntu**:
  ```bash
  sudo apt install ./mark-shot_<version>_amd64.deb
  ```
- **Fedora**:
  ```bash
  sudo dnf install ./mark-shot-<version>-1.x86_64.rpm
  ```

> **Ubuntu 26.04 LTS** : DracoPho a été validé et pris en charge sur Ubuntu 26.04 LTS (Resolute).
> Sur Ubuntu 26.04, la compilation à partir des sources peut utiliser directement les paquets Qt 6.10 fournis par la distribution
> (aucune étape `aqtinstall` n'est nécessaire) :
>
> ```bash
> sudo apt install build-essential cmake ninja-build pkg-config \
>   qt6-base-dev qt6-wayland libpipewire-0.3-dev libxcb-cursor0 \
>   xdg-desktop-portal pipewire xclip
> cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
> cmake --build build
> ```
>
> La capture sans interface (`--capture-to`), la capture multi-écrans (paramètre `--display` répétable) et le service
> MCP local fonctionnent dans les sessions Wayland (GNOME) et X11 sous Ubuntu 26.04.

### Dépendances du système

#### Wayland (Arch Linux)

```bash
sudo pacman -S --needed base-devel cmake ninja pkgconf qt6-base qt6-wayland layer-shell-qt pipewire grim wl-clipboard
```

#### X11/GNOME (Ubuntu/Debian)

```bash
# Outils de compilation
sudo apt install build-essential cmake ninja-build pkg-config libpipewire-0.3-dev

# Outils portal et presse-papiers
sudo apt install xdg-desktop-portal pipewire xclip

# Qt 6 (si les dépôts du système ne fournissent pas Qt 6, installez-le dans un répertoire utilisateur via aqtinstall)
pip install aqtinstall
aqt install-qt linux desktop 6.7.3 gcc_64 --outputdir ~/Qt
```

> **Remarque** : sur les systèmes comme Ubuntu 22.04 qui fournissent Qt 5 par défaut, l'installation de Qt 6 dans `~/Qt` n'affecte pas le système. Passez simplement `-DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64` à la compilation.

#### Prise en charge de la saisie chinoise fcitx5 (Qt 6 sous X11)

Qt 6 n'inclut pas de plugin de méthode de saisie fcitx5. Pour utiliser la saisie chinoise fcitx5 sous X11, compilez ce plugin à partir des sources :

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

#### Backend OCR (optionnel)

La fonction de reconnaissance de texte de DracoPho dépend du script Python intégré `mark-shot-ocr`. Ce script prend en charge **RapidOCR** (préféré, basé sur les modèles PP-OCR de PaddleOCR) et **Tesseract** (repli). Sous Linux, le script est installé automatiquement ; sous Windows, une configuration manuelle est nécessaire.

<details>
<summary><b>Linux</b></summary>

```bash
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime
```

Une fois installé, `mark-shot-ocr` est détecté automatiquement, sans configuration supplémentaire.

**Variables d'environnement** (optionnel) :

| Variable | Description | Valeur par défaut |
|------|------|--------|
| `MARK_SHOT_OCR_VERSION` | Version de PaddleOCR (`PP-OCRv5`, `PP-OCRv4`, etc.) | `PP-OCRv5` |
| `MARK_SHOT_OCR_MODEL_TYPE` | Taille du modèle : `mobile` ou `server` | `mobile` |
| `MARK_SHOT_OCR_MODEL_DIR` | Répertoire de stockage des modèles personnalisés | `~/.local/share/mark-shot/models` |
| `MARK_SHOT_OCR_NO_VENV` | Réglez sur `1` pour désactiver la bascule automatique d'environnement virtuel | — |
| `MARK_SHOT_OCR_PYTHON` | Chemin de l'interpréteur Python utilisé pour la ré-exécution | `~/.local/share/mark-shot/ocr-venv/bin/python` |

</details>

<details>
<summary><b>Windows</b></summary>

Les scripts d'assistance intégrés ne sont pas installés automatiquement sous Windows ; effectuez manuellement les étapes suivantes :

**1. Installez Python 3**

Téléchargez et installez Python 3.10 ou une version ultérieure depuis [python.org](https://www.python.org/downloads/). Lors de l'installation, cochez **Add python.exe to PATH**.

**2. Copiez le script d'assistance OCR**

Copiez `scripts/mark-shot-ocr` depuis le [dépôt DracoPho](https://github.com/tystudio-26020701/DracoPho-community) dans un répertoire local, par exemple `%LOCALAPPDATA%\mark-shot\mark-shot-ocr.py`.

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\mark-shot"
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/tystudio-26020701/DracoPho-community/main/scripts/mark-shot-ocr" `
  -OutFile "$env:LOCALAPPDATA\mark-shot\mark-shot-ocr.py"
```

**3. Créez l'environnement virtuel et installez les dépendances**

```powershell
python -m venv "$env:LOCALAPPDATA\mark-shot\ocr-venv"
& "$env:LOCALAPPDATA\mark-shot\ocr-venv\Scripts\pip.exe" install -U pip rapidocr onnxruntime
```

> `onnxruntime` fournit l'inférence CPU. Si vous disposez d'un GPU compatible, vous pouvez installer `onnxruntime-directml` ou `onnxruntime-gpu` pour accélérer la reconnaissance.

**4. Configurez `ocr.command` dans `config.json`**

Ouvrez `%LOCALAPPDATA%\mark-shot\config.json` (créez-le s'il n'existe pas) et définissez `ocr.command` :

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

Remplacez `%LOCALAPPDATA%` par le chemin réellement développé (par exemple `C:\Users\VotreNomUtilisateur\AppData\Local`). L'espace réservé `{image}` est remplacé à l'exécution par le chemin de la capture temporaire ; s'il est omis, DracoPho l'ajoute automatiquement.

> **Astuce** : définir la variable d'environnement `MARK_SHOT_OCR_NO_VENV=1` permet de sauter la détection automatique d'environnement virtuel intégrée au script, puisque le Python de l'environnement virtuel est déjà utilisé directement.

</details>

#### Backend de scan de codes (optionnel)

```bash
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

L'assistant de scan privilégie `zxing-cpp`, qui prend en charge les formats courants : QR Code, Data Matrix, Aztec, PDF417, EAN, UPC, Code 39, Code 93, Code 128, etc. Si `pyzbar` ou OpenCV est installé, il est également utilisé comme backend de repli.

#### Backend de téléversement d'images (optionnel)

Le téléversement d'images utilise par défaut le script Python intégré `mark-shot-upload`, sans dépendances supplémentaires à installer (il n'utilise que la bibliothèque standard de Python 3). Ce script configure les paramètres de l'hébergeur via des variables d'environnement et prend en charge tout service d'hébergement compatible avec le protocole de téléversement multipart/form-data.

<details>
<summary>Variables d'environnement prises en charge par l'assistant intégré</summary>

| Variable d'environnement | Description | Valeur par défaut |
|---------|------|--------|
| `MARK_SHOT_UPLOAD_URL` | **Obligatoire**, endpoint de l'interface de téléversement | — |
| `MARK_SHOT_UPLOAD_FIELD` | Nom du champ fichier | `image` |
| `MARK_SHOT_UPLOAD_API_KEY` | Clé API / jeton | — |
| `MARK_SHOT_UPLOAD_AUTH_HEADER` | Nom de l'en-tête d'authentification | `Authorization` |
| `MARK_SHOT_UPLOAD_AUTH_SCHEME` | Schéma d'authentification (par exemple `Bearer`) ; laisser vide pour utiliser directement la clé API | `Bearer` |
| `MARK_SHOT_UPLOAD_URL_PATH` | Chemin pointé de l'URL dans la réponse JSON (par exemple `data.url`) | Détection automatique |
| `MARK_SHOT_UPLOAD_DELETE_URL_PATH` | Chemin de l'URL de suppression | Détection automatique |
| `MARK_SHOT_UPLOAD_HEADER_xxx` | En-têtes de requête personnalisés (par exemple `MARK_SHOT_UPLOAD_HEADER_X-Custom=foo`) | — |
| `MARK_SHOT_UPLOAD_FIELD_xxx` | Champs de formulaire supplémentaires (par exemple `MARK_SHOT_UPLOAD_FIELD_album=123`) | — |

</details>

<details>
<summary>Exemple de configuration : ImgURL V3</summary>

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

ImgURL V3 utilise l'authentification `Authorization: Bearer <token>` (`AUTH_SCHEME` par défaut `Bearer`, aucune modification nécessaire).

</details>

<details>
<summary>Exemple de configuration : sm.ms</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://sm.ms/api/v2/upload",
    "MARK_SHOT_UPLOAD_FIELD": "smfile",
    "MARK_SHOT_UPLOAD_API_KEY": "VotreJeton",
    "MARK_SHOT_UPLOAD_AUTH_SCHEME": "",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

sm.ms utilise directement le jeton comme valeur Authorization ; `AUTH_SCHEME` doit donc être réglé sur une chaîne vide.

</details>

<details>
<summary>Exemple de configuration : imgbb</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://api.imgbb.com/1/upload?key=VotreAPI_KEY",
    "MARK_SHOT_UPLOAD_FIELD": "image",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

imgbb transmet la clé API via le paramètre de requête de l'URL ; `API_KEY` n'a pas besoin d'être défini.

</details>

<details>
<summary>Exemple de configuration : litterbox (hébergeur temporaire, sans clé API)</summary>

```json
"upload": {
  "command": "curl -sf --max-time 30 -A 'Mozilla/5.0' -F reqtype=fileupload -F time=72h -F fileToUpload=@{image} https://litterbox.catbox.moe/resources/internals/api.php",
  "timeoutMs": 35000
}
```

La réponse de litterbox est une URL en texte brut (pas du JSON) ; DracoPho reconnaît automatiquement la sortie commençant par `http://`/`https://` comme résultat du téléversement.

</details>

<details>
<summary>Commande de téléversement personnalisée</summary>

Si l'assistant intégré ne suffit pas à vos besoins, vous pouvez brancher n'importe quel script de téléversement personnalisé via `upload.command`. La commande doit satisfaire :

1. **Code de sortie** : code de sortie 0 en cas de succès, toute valeur non nulle étant considérée comme un échec
2. **Format de sortie** (au choix) :
   - **JSON** : `{"url":"https://...","deleteUrl":"https://...","errors":[]}` (`url` obligatoire, les autres facultatifs)
   - **URL en texte brut** : la première ligne non vide de la sortie standard commence par `http://` ou `https://`
3. **Espaces réservés** : prend en charge `{image}`, `{imagePath}`, `{imageUrl}` ; si la commande ne contient aucun espace réservé, DracoPho ajoute automatiquement le chemin de l'image temporaire à la fin de la commande

```json
"upload": {
  "command": "/path/to/your-uploader.sh --file {image} --json",
  "timeoutMs": 30000,
  "env": {
    "UPLOADER_API_KEY": "xxx"
  }
}
```

Les variables d'environnement de `upload.env` sont également transmises à la commande personnalisée, ce qui facilite la réutilisation de la configuration.

</details>

#### Windows

Installez Qt 6, CMake et Ninja correspondant à votre compilateur, ainsi qu'un compilateur prenant en charge C++17, par exemple MSVC ou MinGW. La compilation sous Windows ne nécessite pas Qt DBus, PipeWire, X11/XCB, LayerShellQt, `grim`, `wl-copy` ni `xclip`.

```powershell
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
cmake --build build-windows
```

La prise en charge Windows actuelle couvre la capture normale et l'annotation d'images. La capture de défilement, la détection de fenêtres spécifique aux compositeurs et les raccourcis de bureau Linux ne sont pas disponibles sous Windows. Les scripts Python intégrés (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`) ne sont pas installés automatiquement ; veuillez les configurer manuellement en vous référant aux sections [OCR 后端](#ocr-后端可选), [扫码后端](#扫码后端可选) et Traduction ci-dessus.

### Compilation

```bash
# Utilise le Qt 6 du système
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Si Qt 6 est installé dans un répertoire utilisateur, spécifiez en plus CMAKE_PREFIX_PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64

# Effectue la compilation
cmake --build build
```

Ou utilisez nix :

```bash
nix build
```

LayerShellQt est détecté automatiquement. S'il est trouvé, la prise en charge complète de la couche layer-shell Wayland est activée ; sinon, la compilation réussit quand même et l'application revient automatiquement à une fenêtre plein écran standard à l'exécution.

### Installation et intégration

```bash
cmake --install build --prefix "$HOME/.local"
```

Cette commande installe l'exécutable, les scripts d'assistance (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`, `mark-shot-upload`), les raccourcis de bureau et les icônes.

### Extension de capture de défilement GNOME Wayland

La capture de défilement sous GNOME Wayland nécessite l'activation de l'extension **DracoPho Scroll Helper**. Sans cette extension, DracoPho ne peut pas capturer silencieusement et en continu la zone sélectionnée, ni afficher le panneau d'aperçu de défilement natif de GNOME ; le bouton de capture de défilement est donc désactivé sur GNOME Wayland.

Les fichiers de l'extension se trouvent dans `../packaging/gnome-extension/mark-shot-scroll-helper@snemc.org` du dépôt du projet.

<details>
<summary><b>Déplier/replier le guide d'installation et d'activation de l'extension de capture de défilement GNOME Wayland</b></summary>

##### Méthode A : installation via le paquet de la distribution
Si DracoPho a été installé via un paquet de distribution (par exemple `.deb` ou `.rpm`), l'extension est déjà installée avec le système. Activez-la pour l'utilisateur courant en exécutant :
```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```
*Si l'extension est introuvable, déconnectez-vous puis reconnectez-vous et réessayez.*

##### Méthode B : installation depuis le répertoire source du dépôt
Si vous avez compilé DracoPho manuellement à partir des sources, copiez d'abord l'extension dans le répertoire d'extensions GNOME de l'utilisateur :
```bash
# Définit l'UUID de l'extension
UUID=mark-shot-scroll-helper@snemc.org

# Crée le répertoire d'extensions au niveau utilisateur
mkdir -p "$HOME/.local/share/gnome-shell/extensions"

# Copie les fichiers de l'extension depuis le dépôt du projet
cp -r "packaging/gnome-extension/$UUID" "$HOME/.local/share/gnome-shell/extensions/"

# Active l'extension (vous devrez peut-être redémarrer GNOME Shell ou vous déconnecter et vous reconnecter pour qu'elle prenne effet)
gnome-extensions enable "$UUID"
```

Vérifiez que l'interface D-Bus de l'assistant est disponible :

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
```

Le résultat attendu est `('4.2',)`. Après l'activation de l'extension, redémarrez `mark-shot`.

</details>

---

## Guide des raccourcis et des gestes interactifs

### Raccourcis de changement d'outil

| Raccourci | Outil cible | Description de la fonction |
| :---: | :--- | :--- |
| **V** | Déplacer / naviguer (Move / Pan) | En mode image existante, permet de déplacer et de faire glisser le canevas de l'image. |
| **S** | Sélection (Select) | Sélectionne, puis déplace, redimensionne ou supprime les annotations vectorielles dessinées. |
| **P** | Crayon (Pen) | Dessin de courbes libres. |
| **L** | Ligne (Line) | Dessine des lignes vectorielles droites. |
| **H** | Surligneur (Highlighter) | Surlignage semi-transparent, idéal pour marquer les points importants. |
| **R** | Rectangle (Rectangle) | Dessine des contours rectangulaires. |
| **E** | Ellipse (Ellipse) | Dessine des contours elliptiques. |
| **A** | Flèche (Arrow) | Dessine la flèche classique à six sommets, fine, allongée et à angle aigu. |
| **T** | Texte (Text) | Saisit et organise du texte enrichi, avec prise en charge d'une taille de 1000 px et d'une liaison par glisser-déposer. |
| **N** | Numéro (Number) | Étiquettes de numérotation incrémentale automatique. |
| **M** | Mosaïque (Mosaic) | Applique un flou verre dépoli aux zones sensibles. |
| **G** | Pointeur laser (Laser) | Traces temporaires pour l'enseignement ou la présentation, qui se dissolvent automatiquement en douceur. |

### Outils d'assistance de l'écran de démarrage

| Raccourci | Outil | Description de la fonction |
| :---: | :--- | :--- |
| **C** | Pipette (Color Picker) | Échantillonne les pixels de la capture avant de sélectionner la zone de capture. La molette de la souris règle la taille de la loupe, un clic gauche ouvre le panneau de couleurs, avec copie aux formats HEX, RGB, HSL, HSV et Qt. Clic droit ou Esc pour revenir à la sélection normale. |
| **R** | Règle (Ruler) | Mesure les coordonnées avant de sélectionner la zone de capture. Le survol affiche le pixel courant, le glisser-déposer gauche dessine un rectangle de mesure gradué en pixels et affiche la largeur, la hauteur, la diagonale et la surface. Clic droit ou Esc pour revenir à la sélection normale. |
| **Q** | Scan de codes (Code Scanner) | Passe en mode scan de codes QR et de codes-barres. Après avoir encadré une zone, le contenu du code est reconnu et affiché dans une fenêtre copiable. Clic droit ou Esc pour revenir à la sélection normale. |
| **D** | Capture d'écran (Display Capture) | Capture immédiatement tous les écrans de sortie, recadre par écran et affiche des vignettes ; survolez une vignette pour copier, éditer ou enregistrer. |

### Raccourcis d'opérations globales

| Raccourci | Action déclenchée |
| :---: | :--- |
| **Esc** | Quitte et ferme immédiatement la fenêtre d'annotation. |
| **Ctrl + C** | Valide toutes les éditions de texte et copie la capture/zone annotée actuelle dans le presse-papiers du système. |
| **Ctrl + S** ou **Entrée / Retour** | Valide toutes les éditions de texte et enregistre la capture actuelle. |
| **Ctrl + P** | Épingle la zone actuelle en fenêtre flottante. |
| **Ctrl + U** | Téléverse la capture actuelle vers un hébergeur d'images personnalisé ; une fois le téléversement réussi, l'URL est automatiquement copiée dans le presse-papiers. |
| **Ctrl + Z** | Annule la dernière opération d'annotation. |
| **Ctrl + Y** ou **Ctrl + Shift + Z** | Rétablit l'opération d'annotation qui a été annulée. |
| **Retour arrière** ou **Suppr** | Lorsque l'outil **Sélection (Select)** est actif et qu'une annotation est sélectionnée, supprime l'annotation sélectionnée. |
| **F** | Bascule la portée de la capture actuelle (bascules entre le mode zone sélectionnée et le mode plein écran). |

### Techniques d'interaction avancées

- **Contrainte des formes** : pendant le dessin d'un **Rectangle (Rectangle)** ou d'une **Ellipse (Ellipse)**, maintenez la touche `Ctrl` pour forcer un carré ou un cercle parfait.
- **Bascule rapide vers l'outil Sélection** : pendant l'annotation, un clic droit sur une zone vide du canevas bascule immédiatement vers l'outil **Sélection (Select)**.
- **Changement rapide de couleur en double-cliquant droit** : un double-clic droit sur une zone vide du canevas ouvre la roue chromatique pour changer rapidement la couleur de l'outil d'annotation actif.
- **Réglage fluide à la molette** : lorsque l'outil d'annotation correspondant est actif, la molette de la souris ajuste en temps réel la largeur du trait, la taille de police, la taille des étiquettes numérotées ou la taille de la grille de mosaïque de l'outil actif.
- **Déplacement et zoom du canevas** : en mode **Sélection (Select)**, ou lors de l'édition d'un fichier local, la molette de la souris zoome le canevas sans discontinuité et le glisser avec le bouton central de la souris déplace le canevas. Un double-clic sur `Ctrl` réinitialise le zoom et le déplacement.

### Interactions propres à la fenêtre épinglée

| Geste / raccourci | Effet produit |
| :--- | :--- |
| **Bouton gauche maintenu et glissé** | Déplace et place librement l'image épinglée sur le bureau. |
| **Molette de la souris vers le haut/bas** | Agrandit/réduit la fenêtre épinglée proportionnellement et sans à-coups. |
| **Double-clic gauche** | Ferme immédiatement cette fenêtre épinglée. |
| **Clic droit** | Ouvre le menu de fonctions (rotation, copie du texte de l'image, traduction, enregistrement, copie, fermeture, etc.). |
| **Touche Esc** | Ferme la fenêtre épinglée actuellement focalisée. |

---

## Notes de version

Voir les [notes de version](../docs/releases.zh-CN.md).

## Retour et échanges

### Soumettre un problème
Si vous rencontrez un problème à l'exécution ou avez une suggestion de nouvelle fonctionnalité, nous recommandons d'utiliser l'outil en ligne de commande GitHub CLI (`gh`) pour soumettre des problèmes. Nous fournissons un script qui collecte automatiquement les informations sur l'environnement et les génère ; voir le [Guide de soumission de problème](../.doc/submit-issue-via-gh.md) pour plus de détails.

---

## Licence

Ce projet est publié sous la **licence MIT** ; voir le fichier [LICENSE](../LICENSE).

## Remerciements

DracoPho s'appuie sur la communauté open source ; nous leur adressons nos plus sincères remerciements :

- **Le projet amont original [jswysnemc/mark-shot](https://github.com/jswysnemc/mark-shot), son auteur et tous ses contributeurs.** Cette édition communautaire est développée à partir du projet amont original ; sa conception remarquable et ses contributions continues en sont le fondement, et nous les remercions chaleureusement pour leur excellent travail.
- **[serendipitywgy](https://github.com/serendipitywgy)** : merci pour les améliorations de compatibilité inter-environnements de bureau, l'action de barre d'outils de copie OCR et la fonction de pré-sélection intelligente des cadres rectangulaires, contribuées via `serendipitywgy/mark-shot`.
- **Tous les projets open source dont dépend DracoPho**, notamment Qt 6, PipeWire, xdg-desktop-portal, layer-shell-qt, wl-clipboard, xclip, grim, RapidOCR, onnxruntime, Tesseract, ZXing-C++ et autres.

Cette édition communautaire est maintenue par [Beijing Taiyin Zhaowu Technology Co., Ltd.](https://github.com/tystudio-26020701/DracoPho-community) et ses contributeurs, et est publiée sous la **licence MIT**.
