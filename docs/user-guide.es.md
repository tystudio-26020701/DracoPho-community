# Guía de usuario de Mark Shot

Este manual cubre el funcionamiento diario de Mark Shot, con un enfoque en la
característica de **selección por flotación sobre ventanas / componentes**
(mover el mouse automáticamente rastrea y resalta la ventana bajo el cursor; un
clic la selecciona), el flujo de anotación, la captura sin interfaz
(headless) y la configuración.

> Los documentos de este repositorio se redactan en el fork comunitario y se
> reflejan en los repositorios upstream y empresarial. La edición empresarial
> añade una sección extra para su servidor MCP local.

---

## 1. Inicio rápido

### 1.1 Lanzamiento

Inicie una sesión de captura de región:

```bash
mark-shot
```

Pulse una tecla de acceso rápido del escritorio (véase § 8) o ejecútelo desde
una terminal. Se abre una superposición (overlay) congelada a pantalla completa
en la pantalla enfocada. Mueva el mouse para dibujar un rectángulo de selección
y luego suelte el botón para entrar en el editor de anotaciones.

### 1.2 Compilaciones portátiles

Si usa un paquete portátil (`mark-shot-upstream`, `mark-shot-community`,
`mark-shot-enterprise`), ejecútelo con el lanzador incluido para que se
encuentren las bibliotecas Qt, los complementos y los scripts auxiliares
incluidos:

```bash
portable/mark-shot-community/bin/run-mark-shot.sh
```

El lanzador antepone su directorio `bin/` a `PATH`, lo cual es necesario para
los scripts auxiliares de detección de ventanas
(`mark-shot-window-detection-*`) y los auxiliares de OCR / carga.

---

## 2. Selección por flotación sobre ventanas / componentes

Mark Shot puede detectar las ventanas del escritorio actual antes de que elija
una región. Mientras la superposición de selección está abierta, **mover el
mouse resalta la ventana bajo el cursor** con un marco verde azulado (teal).
**Un clic izquierdo simple (sin arrastre) selecciona esa ventana completa**
como región de captura; puede anotarla, copiarla, fijarla o guardarla
directamente.

Las ventanas resaltadas provienen de un script de detección por compositor que
se ejecuta antes de que aparezca la superposición:

| Escritorio | Fuente de detección | Notas |
| :--- | :--- | :--- |
| GNOME Wayland | extensión de Shell `mark-shot-scroll-helper@snemc.org` incluida a través de D-Bus | requiere que la extensión esté habilitada (véase § 2.1) |
| KDE Plasma Wayland | script KWin de una sola ejecución mediante `qdbus6` / `qdbus` + journalctl | requiere una sesión de KWin |
| Hyprland | `hyprctl -j clients` | |
| niri | `niri msg -j windows` + análisis de configuración | |
| X11 | enumeración XCB dentro del proceso de `_NET_CLIENT_LIST_STACKING` | no se necesita script |
| Windows | `EnumWindows` dentro del proceso | no se necesita script |

Solo se rastrean las **ventanas de nivel superior**. Los widgets individuales
dentro de una ventana ("componentes") no los expone el compositor de Wayland,
por lo que la selección por flotación apunta a ventanas completas en todas las
plataformas.

### 2.1 GNOME Wayland: habilite la extensión auxiliar

```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```

Verifique que el auxiliar de D-Bus responda:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
# -> ('5',)
```

Si la llamada falla, cierre la sesión y vuelva a iniciarla (o reinicie GNOME
Shell en X11) y reintente. Sin la extensión, el script auxiliar de GNOME sale
con un error y la selección por flotación permanece desactivada (la selección
normal por arrastre sigue funcionando).

### 2.2 Cómo usarla

1. Active una captura (`mark-shot` o la tecla de acceso rápido del escritorio).
2. Sin pulsar ningún botón del mouse, mueva el cursor sobre una ventana. Un
   marco verde azulado (teal) delimita la ventana que se seleccionaría.
3. **Haga clic una vez** (pulsar y soltar sin mover más de unos pocos píxeles)
   para seleccionar esa ventana. Si las ventanas se superponen, gana la ventana
   más alta en el cursor (consciente del orden z).
4. Al soltar, entra en el editor de anotaciones con la ventana exactamente
   enmarcada.
5. Para hacer una región **manual** en su lugar, simplemente arrastre un
   rectángulo como de costumbre: el marco de flotación se ignora en cuanto el
   arrastre supera el umbral de clic.

El resaltado por flotación está desactivado mientras la herramienta de inicio
Selector de color (`C`) o Regla (`R`) está activa, y permanece disponible para
el Escáner de código (`Q`), la captura de pantalla (`D`) y los modos de inicio
de grabación GIF / Video.

### 2.3 Selección de ventanas en el monitor correcto

La detección de ventanas se ejecuta por objetivo de captura. En una
configuración de varios monitores, cada ventana congelada recibe solo las
ventanas que se cruzan con su propia geometría, por lo que el marco de flotación
coincide con lo que ve en esa pantalla.

### 2.4 Habilitación / deshabilitación

La característica está habilitada por defecto (`windowDetection.enabled = true`).
Actívela o desactívela en **Configuración → Avanzado → Detección de ventanas
habilitada**, o edite `~/.config/mark-shot/config.json`:

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

- `command`: el script de detección. En GNOME / KDE / Hyprland / niri Wayland se
  elige automáticamente el script incluido `mark-shot-window-detection-*` que
  coincide con su sesión; en X11 y Windows la plataforma se enumera dentro del
  proceso y `command` puede dejarse vacío. **Siempre se respeta un comando
  personalizado proporcionado por el usuario (por ejemplo, una ruta absoluta).**
- `timeoutMs`: espera máxima del script (100–30000 ms, por defecto 1000).
- `env`: variables de entorno adicionales que se pasan al script. Los ajustes
  por compositor (desplazamientos) se documentan en los encabezados de los
  scripts.

### 2.5 Solución de problemas

| Síntoma | Comprobación |
| :--- | :--- |
| Sin marco teal en GNOME Wayland | ¿extensión habilitada? la llamada `gdbus` anterior debe devolver una versión |
| Sin marco teal en X11 / Windows | ninguna: la enumeración de la plataforma está integrada; asegúrese de que la sesión de captura no esté usando una herramienta de puntero de inicio |
| El marco de flotación elige la ventana equivocada (la de debajo) | faltan datos de orden z de un script de detección personalizado; las ventanas sin `zOrder` se clasifican como la capa inferior |
| La captura tarda en comenzar | el script de detección se ejecuta antes de la superposición; aumente `timeoutMs` solo si el escritorio es lento, o establezca `enabled:false` para omitirlo |
| Ver diagnósticos | ejecute `mark-shot --debug --debug-log /tmp/mark-shot.log`; busque las líneas `window-detection` |

---

## 3. Selección de región y herramientas de inicio

Antes de confirmar la región puede usar las herramientas de la superposición de
inicio:

| Tecla de acceso rápido | Herramienta | Comportamiento |
| :---: | :--- | :--- |
| `C` | Selector de color | Muestrear un píxel; la rueda redimensiona la lupa; clic izquierdo abre un panel de color (formatos HEX / RGB / HSL / HSV / Qt); clic derecho o `Esc` sale |
| `R` | Regla | Al flotar lee las coordenadas de píxeles; arrastrar a la izquierda mide un rectángulo con ancho, alto, diagonal y área; clic derecho o `Esc` sale |
| `Q` | Escáner de código | Arrastre una región alrededor de un QR / código de barras; el resultado decodificado se abre en una ventana copiable |
| `D` | Captura de pantalla | Captura todas las salidas, recorta por pantalla, muestra miniaturas sobre las que se puede flotar (copiar / editar / guardar) |
| `S` | Detener grabación de GIF / video activa | detiene la grabación mostrada en la superposición |

`Esc` cancela la sesión; el clic derecho (sin herramienta de inicio) también
cancela.

---

## 4. Herramientas de anotación

Después de seleccionar una región (o de abrir una imagen local) se abre el
editor con la barra de herramientas de anotación. Las herramientas se cambian
con las teclas numéricas o con la barra de herramientas:

| Tecla de acceso rápido | Herramienta | Descripción |
| :---: | :--- | :--- |
| `V` | Mover / Desplazar | mover toda la selección, desplazar el lienzo de una imagen local |
| `S` | Seleccionar | seleccionar, mover, escalar, rotar, eliminar anotaciones existentes |
| `P` | Lápiz | trazos suaves a mano alzada |
| `L` | Línea | líneas rectas |
| `H` | Resaltador | marcador semitransparente; estilo a mano alzada o de línea recta |
| `R` | Rectángulo | caja con estilos `Trazo` / `Resaltado` / `Invertir`, esquinas redondeadas |
| `E` | Elipse | elipse / círculo |
| `A` | Flecha | flechas clásicas (emplumadas, KDE, bidireccionales) |
| `T` | Texto | texto enriquecido; la rueda o los controles deslizantes redimensionan; las asas diagonales escalan ambos ejes, las laterales ajustan el ajuste de línea; tamaño exacto en puntos, familia de fuente, negrita / cursiva en el panel de fuentes |
| `N` | Número | marcadores numerados secuenciales (arábicos, alfa, romanos, chinos, …) |
| `M` | Mosaico | desenfoque de vidrio esmerilado acrílico para ocultar contenido sensible |
| `G` | Láser | trazos temporales que se disuelven automáticamente |

Consejos de dibujo:

- Mantenga `Ctrl` mientras dibuja un rectángulo / elipse para limitarlo a un
  cuadrado / círculo.
- Desplace la rueda mientras una herramienta está activa para ajustar el ancho
  del trazo, el tamaño del texto, la escala de los números o el tamaño de los
  bloques de mosaico (vista previa en vivo).
- En `Seleccionar`, desplace para hacer zoom en el lienzo y mantenga el botón
  central para desplazarse; pulse `Ctrl` dos veces para restablecer.

### 4.1 Edición de una anotación existente

Cambie a **Seleccionar** (`S`). Haga clic en una anotación para mostrar sus
asas:

- arrastre dentro para mover;
- arrastre las asas de las esquinas / bordes para redimensionar;
- arrastre el asa redonda sobre el borde superior para rotar;
- pulse `Delete` / `Backspace` para eliminar;
- haga doble clic en el texto para editarlo en el lugar.

El panel de propiedades (lado derecho) edita la anotación seleccionada: color,
ancho, estilo, familia / tamaño de fuente del texto, negrita / cursiva. Se
pueden seleccionar varias anotaciones arrastrando un cuadro de selección con la
herramienta `Seleccionar`; luego el grupo se puede mover, redimensionar, rotar y
eliminar en conjunto.

### 4.2 Acciones

| Atajo | Acción |
| :--- | :--- |
| `Ctrl+C` | copiar al portapapeles |
| `Ctrl+S` / `Enter` | guardar (plantilla de ruta de la configuración) |
| `Ctrl+P` | fijar como ventana adhesiva flotante |
| `Ctrl+U` | subir al alojamiento de imágenes configurado; la URL se copia |
| `Ctrl+Z` / `Ctrl+Y` | deshacer / rehacer |
| `F` | alternar el alcance de la captura (selección ↔ pantalla completa) |

### 4.3 Marco de exportación

Active **Configuración → Exportar → Marco estilo Mac** para añadir relleno
transparente, esquinas redondeadas y una sombra suave a las imágenes
guardadas / copiadas / subidas.

---

## 5. Ventanas adhesivas fijadas

| Gesto / Atajo | Comportamiento |
| :--- | :--- |
| arrastrar a la izquierda | reposicionar la ventana adhesiva |
| rueda | escalar proporcionalmente |
| doble clic izquierdo / `Esc` | cerrar |
| clic derecho | menú contextual (rotar, zoom, siempre encima, copiar texto, traducir, guardar, copiar, cerrar) |

El texto OCR dentro de una ventana fijada es seleccionable y copiable (`Ctrl+C`
/ menú contextual). La traducción (punto de conexión compatible con OpenAI)
representa el texto traducido de nuevo sobre la imagen en las posiciones de
diseño originales.

---

## 6. Captura de pantalla con desplazamiento

1. Seleccione una región (o use el asa de arrastre flotante para regiones muy
   grandes).
2. La superposición desplaza la ventana objetivo; los fotogramas capturados se
   unen en una imagen larga.
3. GNOME Wayland requiere la extensión Mark Shot Scroll Helper (§ 2.1).

La captura con desplazamiento está lista para producción en niri y compositores
wlroots/Wayland similares; en KDE, X11 y otras pilas es una característica de
prueba. Si falla, use capturas de pantalla normales o un comando de extensión
personalizado.

---

## 7. Captura sin interfaz (CLI)

La captura no interactiva escribe un PNG e imprime JSON:

```bash
# pantalla principal
mark-shot --capture-to /tmp/shot.png

# directorio (nombre de archivo con marca de tiempo)
mark-shot --capture-to /tmp/shots/

# región
mark-shot --capture-to /tmp/r.png --region 0,0,1280,720

# una pantalla específica, con cursor
mark-shot --capture-to /tmp/w.png --display DP-1 --include-cursor

# varias pantallas a la vez (un PNG por cada una)
mark-shot --capture-to /tmp/shots/ --display DP-1 --display DP-2

# enumerar salidas
mark-shot --list-displays
```

Todas las opciones sin interfaz son mutuamente excluyentes con un archivo de
imagen posicional. Consulte el README para ver la tabla completa de argumentos.

### 7.1 Captura sin interfaz de ventanas / componentes

Mark Shot puede capturar **ventanas específicas — o un componente (subregión)
dentro de una ventana — sin abrir ninguna interfaz de usuario**, desde un
script, una canalización de compilación o un agente. El proceso sale en cuanto
las imágenes se escriben o se devuelven, y nunca crea una ventana, nunca muestra
un diálogo y nunca roba el foco, por lo que el usuario puede seguir trabajando
mientras una herramienta captura el escritorio.

Primero enumere las ventanas para ver qué hay disponible:

```bash
mark-shot --list-windows
```

Salida de ejemplo (GNOME Wayland):

```json
{"count":2,"platform":"wayland","source":"compositor-script","windows":[
  {"index":0,"id":"0x3c00007","title":"Mark Shot - VSCodium","class":"codium","instance":"codium","x":1920,"y":0,"width":1680,"height":1050,"zOrder":1},
  {"index":1,"title":"Terminal","class":"org.gnome.Terminal","x":67,"y":32,"width":800,"height":600}
]}
```

Cada entrada lleva los campos con los que coinciden los selectores: `index`, `id`
(id de ventana de X11 / id proporcionado por el backend), `title`, `class` e
`instance`, además de `x`/`y`/`width`/`height` y un `zOrder` opcional.

#### 7.1.1 Selección de ventanas (una o varias)

`--window` se puede repetir para capturar **cualquier número de ventanas en una
sola llamada**. Cada selector se interpreta automáticamente (`--window-by auto`):

| Valor del selector | Coincide con |
| :--- | :--- |
| `0`, `1`, … | `index` de la lista |
| `0x3c00007` | `id` de la ventana |
| `VSCodium` | `class` o `instance`, luego `title` (exacto, luego subcadena) |
| `Mark Shot - VSCodium` | `title` |

Fuerce una regla de coincidencia con `--window-by id|title|class|index`. Un
selector que coincide con varias ventanas captura **todas ellas**.

Capture un componente (una subregión dentro de una ventana) añadiendo
`@x,y,width,height` al selector: el desplazamiento es relativo a la esquina
superior izquierda de la ventana y se limita a los límites de la ventana:

```bash
# la franja superior de 100px de la ventana 0
mark-shot --window "0@0,0,1680,100" --capture-destination file --capture-to /tmp/shots/
```

#### 7.1.2 Elección de dónde van las imágenes

`--capture-destination` decide la salida; puede combinarse con cualquier número
de selectores `--window` y una subregión de componente:

| Destino | Comportamiento |
| :--- | :--- |
| `inline` (por defecto) | PNG en Base64 incrustados en la salida JSON. **No se escribe ningún archivo y nunca se toca el portapapeles.** La opción más segura para agentes que solo quieren los píxeles. |
| `file` | Archivos PNG escritos en `--capture-to <directorio>`; requiere esa opción. |
| `stage` | Archivos PNG escritos en un directorio de almacenamiento temporal (`$TMPDIR/mark-shot-staging`). Bueno para un flujo de trabajo de "guardar para más tarde". |
| `clipboard` | Imágenes copiadas al portapapeles del sistema; con varias imágenes, **gana la última**. El contenido sobrevive a la salida del CLI (se inicia un propietario persistente `wl-copy` / `xclip`). |

Ejemplos:

```bash
# varias ventanas, guardadas en un directorio (un PNG por ventana)
mark-shot --window VSCodium --window Terminal --capture-destination file --capture-to /tmp/shots/

# una ventana más un componente de otra ventana, almacenadas para más tarde
mark-shot --window "VSCodium@0,0,400,300" --window 1 --capture-destination stage

# multiselección, devuelta como base64 sin tocar archivos ni portapapeles
mark-shot --window 0 --window "Terminal" --capture-destination inline

# copiar una ventana al portapapeles
mark-shot --window 0 --capture-destination clipboard
```

**Política de portapapeles.** El editor interactivo coloca deliberadamente su
selección en el portapapeles del sistema (la acción `Copiar` / `Ctrl+C`),
porque ese es el flujo de trabajo principal de una herramienta de captura de
pantalla. Los modos sin interfaz (el CLI y el servidor MCP empresarial) siguen
la regla opuesta: **el portapapeles nunca se modifica a menos que se elija
explícitamente `clipboard` como destino Y las escrituras en el portapapeles
estén habilitadas en Configuración > Almacenamiento > Modo sin interfaz** —
`inline` (por defecto) y `stage` dejan intacto el contenido actual del
portapapeles del usuario, por lo que una captura programada o dirigida por un
agente no puede sobrescribir texto o imágenes con los que el usuario esté
trabajando en otro lugar. Cuando una solicitud `clipboard` se rechaza porque las
escrituras en el portapapeles sin interfaz están deshabilitadas, la captura
vuelve al destino sin interfaz predeterminado configurado, la salida JSON
(`"warning"`) y stderr se lo indican, y el proceso sale con un código distinto
de cero para que la automatización pueda detectarlo. Habilitar las escrituras
en el portapapeles sin interfaz en la configuración requiere escribir una frase
de confirmación.

La salida es un objeto JSON `{"captures":[...]}` con una entrada por ventana
capturada; cada entrada repite el selector, la identidad de la ventana y el
rectángulo de captura final, además de un `path` (file/stage), un `data`
(inline) o ninguno (clipboard). El código de salida es `0` solo cuando cada
selector coincidió y cada captura tuvo éxito; una coincidencia ausente o una
captura fallida produce el código de salida `1` con un campo `"error"` en lugar
de un éxito silencioso.

La misma canalización de captura puede producir salida anotada mediante
programación: consulte el capítulo del servidor MCP de la edición empresarial, o
combine el PNG guardado con el editor interactivo.

#### 7.1.3 Garantía de no interferencia con ventanas

Se garantiza que cada modo sin interfaz es invisible y no perturbador:

- **nunca se crea ninguna ventana** — incluidos el editor de anotaciones, la
  superposición de captura y la bandeja; la captura reutiliza la ruta de captura
  sin interfaz;
- **nunca se muestra ningún diálogo** — incluidos los diálogos de error: los
  errores van a stderr; incluso las líneas de comando malformadas (por ejemplo
  `--window-by` sin `--window`, un `--capture-destination` desconocido o
  archivos posicionales extra) salen inmediatamente con un código distinto de
  cero y un mensaje en stderr en lugar de mostrar un `QMessageBox` o caer en la
  interfaz interactiva;
- no aparece ningún aviso de portal interactivo (`allowInteractivePortal` está
  deshabilitado);
- el proceso sale inmediatamente después de escribir la salida;
- la lista de ventanas capturada antes y después de una operación sin interfaz
  es idéntica;
- los modos sin interfaz nunca tocan el portapapeles del sistema a menos que se
  solicite explícitamente `clipboard` **y** las escrituras en el portapapeles
  estén habilitadas en Configuración > Almacenamiento > Modo sin interfaz.

Si no se detecta ninguna ventana (por ejemplo, un auxiliar de compositor
deshabilitado o una sesión de X11 sin enumeración de ventanas), el comando
imprime un error claro en stderr y sale con el código `1` en lugar de capturar
nada en silencio.

---

## 8. Teclas de acceso rápido del escritorio y bandeja

El modo de bandeja (`mark-shot --tray`) registra `Ctrl+Alt+S` para la captura
de región y proporciona entradas de menú de captura / grabación / configuración
/ salir. Teclas de acceso rápido del escritorio:

- **GNOME**: Configuración → Teclado → Atajos → Atajos personalizados →
  vincular a `mark-shot`.
- **KDE**: atajo personalizado vinculado a `mark-shot` (más el permiso KWin
  ScreenShot2 para una captura KDE exacta, consulte el README).
- **Hyprland**: `bind = SUPER SHIFT, S, exec, mark-shot` y `bind = , Print, exec, mark-shot`.
- **niri**: `binds { Mod+Shift+S { spawn "mark-shot"; } }`.
- **Sway / i3**: `bindsym Mod4+Shift+S exec mark-shot`.

---

## 9. Configuración y backends

- Archivo de configuración: `~/.config/mark-shot/config.json` (Linux), creado
  en el primer inicio.
- Referencia completa: [Configuración](configuration.md).
- Backends: Wayland (portal PipeWire / grim / wlroots screencopy), X11
  (`QScreen::grabWindow`), Windows (WGC nativo). La grabación prefiere el
  portal PipeWire y vuelve a fallar automáticamente.

Auxiliares opcionales:

```bash
# OCR (RapidOCR / Tesseract)
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime

# Escaneo de código (zxing-cpp)
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

---

## 10. Lista de comprobación de pruebas de funciones

Úsela para verificar una compilación de extremo a extremo:

1. **Inicio** — `run-mark-shot.sh` abre la superposición congelada.
2. **Flotación sobre ventanas** — mueva el mouse sobre una ventana: el marco
   teal la sigue; un solo clic selecciona la ventana; las ventanas superpuestas
   eligen la más alta.
3. **Región manual** — arrastre un rectángulo; suelte; se abre el editor.
4. **Anotar** — dibuje con cada herramienta (Lápiz, Línea, Rectángulo, Elipse,
   Flecha, Resaltador, Texto, Número, Mosaico, Lupa, Láser); deshacer/rehacer;
   Seleccionar para mover/redimensionar/rotar/eliminar; doble clic en un texto
   para editarlo.
5. **Copiar / Guardar / Fijar / Subir** — `Ctrl+C`, `Ctrl+S`, `Ctrl+P`, `Ctrl+U`.
6. **Herramientas de inicio** — `C` selector de color, `R` regla, `Q` escaneo
   de código, `D` captura de pantalla.
7. **Sin interfaz** — `--capture-to`, `--region`, `--display`, `--list-displays`.
8. **Captura de ventanas sin interfaz** — `--list-windows` enumera el
   escritorio; repita `--window` para capturar varias ventanas; pruebe
   `--capture-destination` en los cuatro modos (inline, file, stage, clipboard);
   verifique un selector de componente (`--window "0@0,0,400,300"`); confirme
   que la lista de ventanas antes y después no cambia (sin interferencia con
   ventanas).
9. **Bandeja + tecla de acceso rápido** — `mark-shot --tray`, pulse `Ctrl+Alt+S`.
10. **Particularidades portátiles** — el paquete encuentra sus propias
    libs/plugins/scripts de Qt.

---

## 11. Comentarios

Informe de los problemas con `gh issue create` usando la
[guía de envío de incidencias](../.doc/submit-issue-via-gh.md) incluida.
Adjunte un registro de depuración capturado con
`mark-shot --debug --debug-log /tmp/mark-shot.log`.
