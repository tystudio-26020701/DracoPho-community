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

Lea este README en otros idiomas：
[简体中文](../README.zh-CN.md) · [繁體中文](./README.zh-TW.md) · [日本語](./README.ja.md) · [한국어](./README.ko.md) · [Русский](./README.ru.md) · [Italiano](./README.it.md) · [العربية](./README.ar.md) · [Français](./README.fr.md) · [Deutsch](./README.de.md) · [Español](./README.es.md) · [Português](./README.pt.md)

**Etiquetas**: `C++` / `Qt 6` / `屏幕截图` / `图像标注` / `桌面贴图` / `OCR 识别` / `滚动长截图` / `Wayland` / `Windows`


<details>
<summary>Vídeo de demostración</summary>
<p align="center">
  <video src="https://github.com/user-attachments/assets/4f86fcee-fef9-409e-98ba-1491ecee06c7" width="100%" controls></video>
</p>
</details>

`mark-shot` es una herramienta de captura y anotación de alto rendimiento desarrollada con Qt 6. El proyecto se diseñó inicialmente para administradores de ventanas de Wayland como `niri`, y actualmente admite flujos de trabajo habituales de captura y anotación en entornos Linux (X11, GNOME, escritorios wlroots/Wayland) y Windows.

Permite capturar la pantalla al instante y abrir una capa de anotación a pantalla completa adaptativa, ofreciendo al usuario funciones como recorte de regiones, anotación, copiado al portapapeles, guardado y fijación de imágenes en el escritorio.

---

## Características principales

### Caja de herramientas de anotación
- **Lápiz y resaltador**: admite trazado de líneas libres suaves y superposición de color de resaltado semitransparente.
- **Herramientas de geometría vectorial**: rutas de alta precisión para líneas, rectángulos y elipses. El rectángulo admite tres estilos conmutables:
  - `描边`: el rectángulo de trazo o relleno original, con esquinas redondeadas opcionales.
  - `高亮`: un efecto de cobertura tipo resaltador implementado con `CompositionMode_Multiply` y relleno semitransparente.
  - `反色`: invierte los canales RGB de los píxeles del área cubierta por el rectángulo, conservando el contorno exterior como pista visual.
- **Flecha optimizada**: utiliza la clásica trayectoria de flecha de seis vértices, con bordes suaves y renderizado antialias.
- **Texto doblemente vinculado**:
  - Admite el ajuste continuo de tamaños de fuente muy grandes, con escala suave mediante la rueda del ratón o el control deslizante de propiedades.
  - Introduce un diseño de búfer de anchura física para evitar saltos de línea inesperados causados por el temblor del renderizado en proporciones de escala extremadamente altas.
  - Los **puntos de control en las esquinas** permiten escalar proporcionalmente y de forma vinculada el tamaño de fuente y el cuadro de texto; las **líneas de control laterales** ajustan únicamente la anchura del límite de composición.
- **Lápiz láser de presentación**: ideal para presentaciones o clases; los trazos se desvanecen suavemente con el tiempo.
- **Números de paso autoincrementales**: un clic coloca marcadores de paso numéricos en orden creciente.
- **Mosaico**: permite difuminar información sensible con un efecto de desenfoque de vidrio esmerilado.
- **Lupa con dos marcos ajustables de forma independiente**: el marco de encuadre interior y la lente exterior de la lupa tienen cada uno sus propios tiradores de redimensionado; la lente rectangular tiene 8 tiradores de esquina/lado por marco, y la circular 4 tiradores (arriba, abajo, izquierda y derecha) por marco. Al ajustar cualquiera de los marcos, el otro se vincula según el factor de ampliación, que permanece siempre constante; al desplazar un único marco, el otro conserva su posición.
- **Escaneo de códigos en la fase de inicio**: pulse `Q` antes de la selección para entrar en el modo de escaneo; tras encuadrar un código QR o un código de barras se abrirá una ventana con el resultado reconocido que se puede copiar.
- **Captura rápida de monitores**: pulse `D` antes de la selección para capturar al instante todas las pantallas de salida y recortarlas por monitor en miniaturas; al pasar el ratón sobre una miniatura podrá copiar, editar o guardar la captura de ese monitor.
- **Grabación de GIF, MP4 y WebP animado**: mediante el atajo de grabación de la fase de inicio o el menú de la bandeja, puede grabar un monitor concreto o una región personalizada como GIF, MP4 o WebP animado. La grabación es a prueba de fallos: el MP4 se escribe en un MKV temporal y se remuxa al finalizar, por lo que una grabación interrumpida sigue siendo recuperable. Las grabaciones activas muestran su estado en la bandeja y en el fotograma congelado, y pueden detenerse con `S`, el botón de la capa, el menú de la bandeja o `--stop-recording`; se envían notificaciones de escritorio al iniciar y al guardar. En Wayland, la grabación usa preferentemente el backend del portal de PipeWire; cuando la captura por portal no está disponible, se recurre a wlroots screencopy o a la captura por sondeo.
- **Subida a alojamiento de imágenes**: tras la selección, pulse `Ctrl+U` o el botón de subida de la barra de herramientas para subir la captura actual a un alojamiento de imágenes personalizado (como ImgURL, sm.ms, imgbb, litterbox, etc.); la URL se copia automáticamente al portapapeles cuando la subida se completa. Los parámetros del alojamiento se pueden configurar mediante `upload.env`, o se puede integrar cualquier script de subida personalizado mediante `upload.command`.
- **Marco de exportación estilo Mac**: añade márgenes transparentes, esquinas redondeadas y sombras suaves a las imágenes guardadas, copiadas, subidas, abiertas con «Abrir con» y a las de los comandos de extensión.

### Fijación de imágenes flotantes (Pin)
- Admite fijar la captura o el área anotada en la pantalla como una ventana de imagen flotante independiente, sin bordes y siempre en primer plano.
- Admite seleccionar directamente el texto reconocido por OCR dentro de la ventana de imagen flotante y copiarlo con `Ctrl + C` o con el menú contextual.
- Admite llamar a un LLM mediante interfaces compatibles con OpenAI para traducir el texto OCR y renderizar la traducción sobre la imagen flotante, superpuesta en la posición del texto original.
- **Interacción cómoda**:
  - Arrastre con el botón izquierdo del ratón para desplazar libremente la imagen flotante.
  - Use la rueda del ratón para escalar la imagen flotante de forma proporcional.
  - Haga doble clic con el botón izquierdo o pulse `Esc` para cerrar la imagen flotante.
  - Haga clic con el botón derecho para abrir el menú, que permite rotar en varios ángulos, copiar el texto de la imagen, traducir, guardar como, copiar o cerrar.

### Captura con desplazamiento
- Captura páginas o regiones largas mediante screencast de PipeWire, una capa de desplazamiento interactiva y un compositor de imágenes.
- Esta función está pensada principalmente para `niri` y entornos Wayland de comportamiento similar, donde la geometría de salida, el tiempo de captura y la posición de las ventanas se mantienen más fácilmente estables.
- **Tirador flotante para selecciones grandes**: cuando la región seleccionada es tan grande que el espacio restante de la pantalla no basta para mostrar el panel de vista previa del desplazamiento, el panel se oculta automáticamente y aparece en el borde de la selección un **tirador de arrastre flotante** (un botón flotante con flechas de dirección).
  - **Ajustar la selección arrastrando**: puede mantener pulsado y arrastrar el tirador flotante para desplazar la selección de captura a lo largo del eje de desplazamiento y capturar contenido más allá del alcance inicial de la pantalla;
  - **Cambiar el eje al hacer clic**: antes de iniciar la captura, un clic en el tirador flotante cambia directamente la dirección de desplazamiento (vertical/horizontal).
- **Nota de compatibilidad**: la captura con desplazamiento en KDE, GNOME, X11 y otros entornos que no son `niri` sigue siendo una función experimental y no está pulida. Estas pilas de escritorio difieren en la política del backend de portal, el comportamiento del Shell o del administrador de ventanas, la retroalimentación de la geometría de las ventanas, el tiempo de los fotogramas y el manejo de los eventos de desplazamiento.
- Si la captura con desplazamiento no funciona, use el flujo de captura normal o integre una herramienta externa de captura larga mediante los comandos de extensión de Mark Shot.
- Si necesita informar de un problema con la captura con desplazamiento, ejecute primero `mark-shot --debug --debug-log /path/to/mark-shot.log`, reproduzca el problema y adjunte el registro al informe en GitHub. También puede activarla mediante `debug.enabled` y `debug.logPath` en `config.json`; `DEBUG=1` y `MARK_SHOT_DEBUG_LOG=/path/to/log` siguen estando disponibles.

### Compatibilidad entre servidores de visualización
- **Wayland**: usa el screencast del portal de PipeWire para la grabación y la captura con desplazamiento experimental, y gestiona las dos rutas de fotogramas: memoria compartida y DMA-BUF; usa `grim` para la captura wlroots, `layer-shell-qt` para crear la capa nativa y `wl-copy` para mantener el portapapeles persistente.
- **X11**: usa `QScreen::grabWindow` para la captura, una ventana a pantalla completa siempre en primer plano como capa y `xclip` para el portapapeles persistente.
- **Windows**: usa las API nativas de captura y portapapeles de Qt para los flujos básicos de captura, anotación, copiado, guardado y fijación de imágenes. Los backends específicos de Linux, como PipeWire, xdg-desktop-portal, `grim`, la detección de ventanas XCB, LayerShellQt y el helper de GNOME Shell, se desactivan en tiempo de compilación.
- El backend del servidor de visualización de Linux se detecta automáticamente en tiempo de ejecución mediante `$XDG_SESSION_TYPE`; en Windows se usa el backend de plataforma nativo de Qt.
- **Multi-monitor freeze scope**: de forma predeterminada, la selección de región congela todos los monitores conectados (una sola ventana de escritorio virtual cuando las DPR coinciden en X11/Windows); tras confirmar una selección en un monitor, los demás monitores permanecen congelados y no interactivos hasta que finalice la sesión. El alcance **Cursor Screen** congela únicamente el monitor bajo el cursor.

### Integración con el escritorio
- **Comportamiento de inicio configurable**: al iniciar Mark Shot ya no se abre la superposición de captura por defecto. En Configuración → General → **Comportamiento de inicio** puedes combinar los cuatro modos: **Icono de bandeja**, **Bola flotante**, **Ventana de configuración** y **Captura directa**. Las instalaciones nuevas vienen con Icono de bandeja + Bola flotante; la captura directa es opcional.
- **Bola flotante**: un pequeño widget circular siempre encima (por defecto abajo a la derecha) que da acceso rápido a captura, captura de pantalla completa, grabación y configuración. Un clic abre el menú, doble clic captura, arrastrar la mueve (recuerda la posición) y se oculta automáticamente durante la captura.
  Al soltarla cerca del borde de la pantalla se acopla y **la mitad del lado del borde se desliza fuera de la pantalla** (pasar el ratón la revela y alejarlo la vuelve a ocultar; funciona en todas las plataformas) y, tras unos segundos de inactividad, se desvanece a semitransparente, volviendo a su opacidad total al pasar el ratón.
- **Accesos directos de escritorio**:
  - `mark-shot.desktop`: configurado como herramienta de captura global del sistema, invocable directamente con atajos de teclado del sistema.
  - `mark-shot-edit.desktop`: registrado como editor de imágenes independiente, integrable en el menú contextual «Abrir con» de los administradores de archivos (como Dolphin o Nautilus).
- Incluye los iconos vectoriales del sistema de alta resolución `mark-shot.svg` y `mark-shot-edit.svg`.

### Autorización de KDE KWin ScreenShot2

En KDE Wayland, Mark Shot puede usar la interfaz `org.kde.KWin.ScreenShot2` de KWin para realizar capturas precisas de regiones. KWin trata esta interfaz como una interfaz D-Bus restringida, por lo que el archivo de escritorio de la aplicación debe declarar el campo de autorización.

<details>
<summary>Instrucciones de autorización de KDE KWin ScreenShot2 y configuración del archivo de escritorio (haga clic para expandir)</summary>

Declare el campo de autorización:
```ini
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

Los paquetes de las distribuciones y `cmake --install` instalan automáticamente los archivos de escritorio necesarios. Si ejecuta directamente los artefactos de una compilación local sin instalar el proyecto, cree o actualice `~/.local/share/applications/mark-shot.desktop`:

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

Si vincula Mark Shot mediante el servicio de atajos de comandos de KDE, también debe crear `~/.local/share/applications/net.local.mark-shot.desktop`:

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

Tras modificar los archivos de escritorio, se recomienda cerrar sesión e iniciarla de nuevo para que KDE vuelva a leer la caché de archivos de escritorio. Si la sesión de KDE actual sigue devolviendo `NoAuthorized`, reinicie KWin o reinicie el sistema una vez.
</details>


---

## Comparación de productos

Mark Shot Community Edition es una herramienta todo en uno de captura, anotación, fijación de imágenes (pin), OCR, traducción y grabación de pantalla, **de código abierto (MIT), multiplataforma (Linux X11/Wayland nativo + Windows) y totalmente offline**. Las tablas siguientes se han elaborado a partir de la documentación oficial y los sitios web de cada producto (a fecha de **agosto de 2026**) y abarcan las herramientas de captura más populares —de código abierto y comerciales— de todas las principales plataformas de escritorio. Las capacidades se indican con honestidad: **✅ soporte integrado**; **⭕ soporte parcial (limitado por el nivel de pago, la plataforma o una herramienta/servicio externo)**; **❌ no compatible**. Se cuentan siempre según el criterio «listo para usar», sin vincularlas a membresías de pago, servicios en la nube ni ramas experimentales; las limitaciones concretas de cada ⭕ se explican en las notas detalladas siguientes.

### Matriz de capacidades principales

#### I. Captura (¿puede obtener lo que necesita?)

| Capacidad | Mark Shot CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Captura de región | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Captura a pantalla completa / multimonitor | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Captura de ventana | ⭕ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Contenido de ventana oculta / minimizada | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Captura con desplazamiento / captura larga | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ⭕ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Captura programada / con retardo | ❌ | ✅ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⭕ | ❌ | ✅ |

#### II. Anotación e inteligencia (¿puede trabajar de forma eficiente tras la captura?)

| Capacidad | Mark Shot CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Conjunto de herramientas de anotación | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Fijar imagen (Pin) a la pantalla | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ |
| Reconocimiento de texto OCR | ✅ | ✅ | ✅ | ⭕ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Traducción | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| Reconocimiento de códigos QR / de barras | ✅ | ✅ | ✅ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ⭕ |
| Selector de color | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ✅ | ✅ | ✅ | ✅ |

#### III. Salida y automatización (¿puede compartir / archivar el resultado?)

| Capacidad | Mark Shot CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Grabación de pantalla | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ |
| Salida GIF animado / WebP | ✅ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ |
| Subida de imágenes / nube | ✅ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ |
| CLI sin interfaz / scripting | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| Bola flotante de acceso rápido | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Historial de capturas | ❌ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ | ⭕ | ❌ | ✅ | ✅ | ⭕ | ❌ | ⭕ |

#### IV. Plataforma y ecosistema

| Capacidad | Mark Shot CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Linux Wayland nativo | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Multiplataforma (≥2 SO de escritorio) | ⭕ | ❌ | ⭕ | ✅ | ✅ | ✅ | ❌ | ⭕ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ |
| Mecanismo de plugins / extensiones | ✅ | ⭕ | ❌ | ❌ | ⭕ | ✅ | ⭕ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| Código abierto / gratuito | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ✅ | ⭕ | ✅ | ❌ | ❌ | ✅ | ⭕ | ⭕ |

### Notas detalladas de las funciones

**I. Captura** (¿puede obtener lo que necesita?)

- **Captura de región / pantalla completa / multimonitor** — integrada en las 15 herramientas enumeradas; Mark Shot además gestiona correctamente las pantallas con coordenadas negativas y el escalado HiDPI por pantalla, sin duplicados fantasma al unir las capturas multimonitor.
- **Captura de ventana** — El modo interactivo de Mark Shot usa una capa de selección a pantalla completa (sin un modo dedicado de «ventana activa» de un clic, de ahí el ⭕), mientras que su modo sin interfaz (`--window`) selecciona las ventanas con precisión por id, título, clase, **PID o nombre de proceso**; Flameshot no tiene ningún modo de ventana y todas las demás herramientas incluyen uno.
- **Contenido de ventana oculta / minimizada** — en X11, Mark Shot lee el búfer compuesto propio de la ventana mediante XComposite y captura el contenido real incluso cuando la ventana está totalmente oculta o minimizada (sin elevarla ni robar el foco); entre las herramientas similares, solo `--stack` de scrot puede hacerlo. El protocolo de Wayland impide que cualquier herramienta lea el contenido de las ventanas minimizadas.
- **Captura con desplazamiento / captura larga** — Mark Shot funciona de forma nativa en niri y GNOME Wayland (con la extensión oficial), siendo KDE/X11 una característica experimental. ShareX, PixPin, PicPick, Snagit, CleanShot X, Shottr, Xnip e iShot la incluyen de serie; Greenshot solo para escenarios heredados de Internet Explorer; Snipaste, Flameshot, ksnip, Spectacle y Snipping Tool de Windows no la soportan.
- **Captura programada / con retardo** — aún no disponible en Mark Shot (❌); la soportan la mayoría de las herramientas convencionales excepto PixPin, Snipaste y Xnip: una mejora natural para el futuro.

**II. Anotación e inteligencia** (¿puede trabajar de forma eficiente tras la captura?)

- **Conjunto de herramientas de anotación** — Mark Shot incluye más de 12 herramientas: lápiz, resaltador, línea, rectángulo, elipse, flecha, texto, números de paso, mosaico, lupa de doble marco, puntero láser, selector de color y regla; las 15 herramientas incluyen anotación integrada con distinta profundidad.
- **Fijar imagen (Pin) a la pantalla** — Mark Shot ofrece imágenes fijadas sin bordes y siempre en primer plano, con escala, rotación, selección de palabras mediante OCR y traducción con LLM sobre la propia imagen; Snipaste, PixPin, ShareX, ksnip, CleanShot X, Shottr, Xnip e iShot también la incluyen; Flameshot, Spectacle, Greenshot, PicPick, Snipping Tool y Snagit no.
- **Reconocimiento de texto OCR** — Mark Shot incluye RapidOCR (modelos PP-OCR, offline) con Tesseract como respaldo, listo para usar; Snipping Tool de Windows (acciones de texto locales), ShareX, Snagit, CleanShot X, Shottr, Xnip e iShot lo incluyen; Snipaste lo limita a un nivel de pago, ksnip a través de un plugin y Spectacle mediante una instalación externa de Tesseract; Flameshot, Greenshot y PicPick no lo tienen.
- **Traducción** — Mark Shot incluye traducción de capturas con LLM mediante interfaces compatibles con OpenAI (admite funcionamiento offline / servicios propios). Solo iShot incluye traducción de capturas, PixPin la limita a la membresía y ninguna otra herramienta la ofrece. Sigue siendo uno de los diferenciadores más destacados.
- **Reconocimiento de códigos QR / de barras** — Mark Shot escanea códigos QR, de una dimensión y PDF417 con la tecla `Q` durante la captura; ShareX y PixPin lo soportan; Snipaste (de pago) y CleanShot X / Shottr / iShot leen únicamente códigos QR mediante sus motores de OCR.
- **Selector de color** — Mark Shot incluye un selector de color de pantalla integrado con paleta de historial; la mayoría de las herramientas incluyen uno, excepto Spectacle y Snagit; Snipping Tool de Windows solo en los equipos con IA Copilot+.
- **Enmascarado automático con IA en un clic** — el enmascarado automático de la captura de WeChat de 2026 aún no está integrado (❌); el mosaico/desenfoque de Mark Shot es manual. Snagit (AI Smart Redact) y el enmascarado de texto de Snipping Tool son implementaciones comparables; el enmascarado automático es una posible vía a seguir.

**III. Salida y automatización** (¿puede compartir / archivar el resultado?)

- **Grabación de pantalla** — Mark Shot graba MP4 / GIF / WebP animado con grabación silenciosa y no supervisada de regiones/monitores (sin ventanas, sin diálogos y sin robar el foco); ShareX, PixPin, Spectacle (Plasma 6), PicPick, Snipping Tool, Snagit, CleanShot X e iShot la incluyen; Snipaste, Flameshot, ksnip, Greenshot, Shottr y Xnip no.
- **Salida GIF animado / WebP** — Mark Shot graba de forma nativa **WebP animado** (archivos más pequeños, soporte de transparencia alfa); ningún competidor produce WebP animado de forma nativa. PixPin e iShot graban GIF, ShareX graba GIF (WebP solo como guardado estático), Snagit y CleanShot X exportan GIF; el resto no tiene salida animada.
- **Subida de imágenes / nube** — Mark Shot incluye subida a ImgURL / sm.ms / imgbb / litterbox / comandos personalizados (`Ctrl+U`); ShareX es el que más destinos ofrece; Greenshot, PicPick, Snagit y CleanShot X tienen subida a la nube integrada; Flameshot solo Imgur, ksnip Imgur/FTP/scripts, Shottr S3 tras la activación; Snipaste, PixPin, Snipping Tool, Xnip e iShot no tienen ninguna.
- **CLI sin interfaz / scripting** — Mark Shot ofrece una cadena headless completa: capturas de región/monitor con `--capture-to`, capturas de ventanas/componentes con `--list-windows` + `--window`, grabación no supervisada con `--record-region` / `--record-display` y `--record-wait-json` para esperar y emitir un estado estructurado; todo ello sin ventanas, sin diálogos y sin robar el foco, además de **poder especificar el PID / nombre de proceso** y capturar ventanas ocultas o minimizadas. El servidor MCP de la edición comercial reutiliza la misma cadena. ShareX (solo Windows), Flameshot, ksnip y Spectacle tienen CLIs sólidas; la CLI de Snipaste es una función de nivel de pago; PixPin solo ofrece un motor de acciones basado en JS.
- **Bola flotante de acceso rápido** — Mark Shot incluye una bola arrastrable que se adhiere a los bordes de la pantalla (X11/Windows/macOS; en Wayland se mantiene flotando libremente por limitaciones del protocolo) y se desvanece cuando está inactiva; solo PixPin ofrece algo similar.
- **Historial de capturas** — Mark Shot aún no tiene un panel de historial dedicado; ShareX, Snipaste, Flameshot, Snagit y CleanShot X sí lo tienen, ksnip / PicPick / Shottr / iShot ofrecen alternativas ligeras (pestañas / galería / imágenes fijadas guardadas), y el resto ninguna.

**IV. Plataforma y ecosistema**

- **Linux Wayland nativo** — Mark Shot soporta de forma nativa el portal de PipeWire, grim, layer-shell, KDE KWin ScreenShot2 y las extensiones de GNOME; entre las herramientas de código abierto, solo Spectacle (KDE) alcanza ese nivel, mientras que Flameshot / ksnip son experimentales o dependen del portal.
- **Multiplataforma** — Snipaste, Flameshot, ksnip y Snagit cubren los tres principales sistemas de escritorio (Windows + macOS + Linux); Mark Shot cubre Linux + Windows (macOS previsto); ShareX, PicPick, Snipping Tool, Spectacle, CleanShot X, Shottr, Xnip e iShot son de una sola plataforma.
- **Mecanismo de plugins / extensiones** — Mark Shot ofrece un sistema de plugins Qt con un mercado de plugins en GitHub (con proveedores de OCR, traducción y escaneo de códigos extensibles); ksnip y Greenshot tienen API de plugins; ShareX, Spectacle, Snagit, CleanShot X y Shottr los sustituyen con acciones personalizadas / integraciones.
- **Código abierto / gratuito** — Mark Shot CE tiene licencia MIT, es totalmente gratuito, sin anuncios, sin cuentas y sin necesidad de conexión a internet; ShareX, Flameshot, ksnip, Spectacle y Greenshot (Windows) también son gratuitos/de código abierto; Shottr y Snipping Tool son gratuitos; PixPin / Snipaste / PicPick / Xnip / iShot son freemium de código cerrado; Snagit / CleanShot X son software comercial de pago.

> Nota: elaborado a partir de los sitios web oficiales y la documentación de cada producto (2026-08); el alcance de las funciones cambia con cada versión, por lo que debe consultarse la documentación más reciente. FastStone Capture, Nimbus, Lightshot, Sogou Capture y la captura de WeChat / QQ no figuran en la tabla anterior (el enmascarado automático con IA en un clic de WeChat de 2026 lidera el segmento).

---

## Interfaz de línea de comandos (CLI)

### Ejemplos de uso habituales

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

#### Captura sin interfaz (no interactiva)

Los scripts, la automatización de CI u otros programas pueden invocar `mark-shot` para realizar capturas sin abrir la interfaz de anotación.
El fotograma capturado se escribe en un PNG y se imprime en la salida estándar una línea de resumen JSON compacto:

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

Ejemplo de salida JSON de `--capture-to` con un único monitor:

```json
{"path":"/tmp/shot.png","width":2560,"height":1440,"output":"DP-1","error":null}
```

Cuando se especifican varios `--display`, la salida pasa a ser una matriz con una captura por pantalla:

```json
{"captures":[{"path":"/tmp/shots/mark-shot-DP-1-20260801-000000.png","width":2560,"height":1440,"output":"DP-1","error":null},
             {"path":"/tmp/shots/mark-shot-DP-2-20260801-000000.png","width":1920,"height":1080,"output":"DP-2","error":null}]}
```

Cada monitor seleccionado se captura usando su propia geometría de origen, por lo que los backends tipo portal devuelven con precisión ese monitor y no todo el escritorio virtual.

La captura sin interfaz reutiliza todos los backends de captura de la interfaz interactiva (QScreen,
xdg-desktop-portal, PipeWire, grim, helpers de KWin/GNOME y Windows Graphics Capture),
por lo que la calidad de imagen y el comportamiento de recorte de regiones son idénticos. Todos los parámetros de captura sin interfaz son mutuamente excluyentes con el parámetro posicional de archivo de imagen.

### Descripción de los parámetros de la CLI

| Opción | Descripción |
| :--- | :--- |
| `[file]` | **Parámetro posicional**: abre un archivo de imagen local existente en modo de anotación, en lugar de capturar la pantalla actual. |
| `-h`, `--help` | Muestra la ayuda y sale. |
| `-v`, `--version` | Muestra la información de la versión y sale. |
| `--all-outputs` | Captura todas las pantallas de salida del escritorio virtual, en lugar de solo la pantalla activa actual. |
| `--xdg-window` | Fuerza el uso de una ventana normal a pantalla completa XDG estándar (xdg-shell) en lugar de la capa de Wayland predeterminada (layer-shell). |
| `--fullscreen` | Omite el paso de selección y anota directamente la captura de pantalla completa. |
| `--default-tool <tool>` | Especifica la herramienta de anotación predeterminada tras completar una selección normal; también actúa como herramienta predeterminada del modo de pantalla completa si no se establece `--fullscreen-default-tool`. |
| `--fullscreen-default-tool <tool>` | Especifica la herramienta predeterminada del modo de anotación a pantalla completa. |
| `--default-color <color>` | Especifica el color de anotación predeterminado. Admite `#RRGGBB` y `#RRGGBBAA`. |
| `--tray` | Mantiene a Mark Shot en ejecución en la bandeja del sistema y registra el atajo global de captura cuando la plataforma lo permite. |
| `--capture` | Fuerza una única captura cuando el inicio automático desde la bandeja está habilitado en la configuración. |
| `--pin-image <path>` | Abre directamente una imagen local como ventana de imagen flotante, omitiendo el flujo de captura y selección. |
| `--recording-status` | Muestra por la instancia en ejecución el estado actual de la grabación en JSON. |
| `--stop-recording` | Solicita a la instancia en ejecución que detenga la grabación activa actual. |
| `--record-region <x,y,w,h>` | Graba una región de la pantalla a través de la instancia en ejecución; geometría como x,y,ancho,alto. |
| `--record-display <id>` | Graba un monitor por id (consulte la salida de `--list-displays`: un nombre de pantalla bruto como `DP-1`, la clave `output:DP-1` o `all`). |
| `--record-output <path>` | Ruta del archivo de salida de la grabación (obligatoria con los indicadores de grabación). |
| `--record-duration <seconds>` | Duración de la grabación en segundos; 0 graba hasta `--stop-recording`. |
| `--record-fps <n>` | Velocidad de fotogramas de la grabación (15 por defecto). |
| `--record-format <mp4\|gif\|webp>` | Formato de grabación: mp4 (predeterminado), gif o webp. |
| `--record-audio` | Incluye el audio del sistema en la grabación. |
| `--record-wait-json` | Espera a que la grabación termine y luego imprime el estado final como JSON. |
| `--debug` | Habilita el registro de depuración para esta ejecución. |
| `--no-debug` | Deshabilita el registro de depuración para esta ejecución, anulando el archivo de configuración y las variables de entorno. |
| `--debug-log <path>` | Escribe el registro de depuración en la ruta especificada; habilita el registro de depuración a menos que también se establezca `--no-debug`. |
| `--capture-to <path>` | Captura sin interfaz: escribe el PNG en el archivo o directorio especificado sin abrir la interfaz; imprime un resumen JSON en la salida estándar. |
| `--region <x,y,w,h>` | Se usa con `--capture-to`: captura solo la región lógica de pantalla especificada. |
| `--display <name>` | Se usa con `--capture-to`: captura la pantalla de salida especificada por nombre de monitor. Puede repetirse para capturar varios monitores a la vez (un PNG por pantalla). |
| `--include-cursor` | Se usa con `--capture-to`: dibuja el puntero del ratón en el fotograma capturado. |
| `--output-name <name>` | Se usa con `--capture-to`: nombre base de archivo (sin extensión) cuando la ruta de captura es un directorio. |
| `--list-displays` | Muestra en JSON la información de todos los monitores actuales y sale. |

### Asignación de atajos de teclado

Vincule `mark-shot` como atajo de captura del sistema:

**niri** (modifique `~/.config/niri/config.kdl`):
```kdl
binds {
    Mod+Shift+S { spawn "mark-shot"; }
}
```

**Hyprland** (modifique `~/.config/hypr/hyprland.conf`):
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bind = SUPER SHIFT, S, exec, mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bind = , Print, exec, mark-shot
```

**Sway / i3** (modifique `~/.config/sway/config` o `~/.config/i3/config`):
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bindsym Mod4+Shift+S exec mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bindsym Print exec mark-shot
```

**GNOME**: añádalo en Configuración del sistema → Teclado → Atajos de teclado → Atajos personalizados.

**Modo bandeja**:
```powershell
mark-shot --tray
```

El modo bandeja registra por defecto los siguientes atajos globales:
- `Ctrl+Alt+S`: inicia una captura de región.

El menú de la bandeja también ofrece acciones como captura, captura a pantalla completa, iniciar grabación, estado de grabación, detener grabación, configuración y salir.


### Comandos de extensión

La barra de acciones de la derecha ofrece un botón **Extensions**; el programa lee los comandos personalizados del usuario desde `~/.config/mark-shot/extensions.json`. El archivo de configuración puede ser una matriz JSON o un objeto JSON con una matriz `commands`.

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

`command` se ejecuta mediante `$SHELL -c` en sistemas tipo Unix y mediante `%COMSPEC% /C` en Windows, por lo que admite expresiones de shell. Use `{slurp}` para pasar la selección actual al comando como una cadena de geometría `x,y widthxheight`. Use `{image}` o `{imagePath}` para pasar la selección renderizada actual como una ruta de PNG temporal, o `{imageUrl}` para pasar una URL `file://`. Estos marcadores de posición se escapan automáticamente para la cita de shell; no añada comillas adicionales en la configuración. Si no usa un marcador de posición de imagen, puede establecer `saveImage` o `needsImage` en `true` y el programa añadirá automáticamente la ruta del PNG temporal al final del comando. `workingDirectory` equivale a `cwd`. El valor predeterminado de `closeOnStart` es `true`: Mark Shot se oculta y se cierra antes de iniciar el comando.

### Archivo de configuración de la aplicación

Vea la [Referencia de configuración](../docs/configuration.zh-CN.md).

### Manual del usuario

Para las operaciones cotidianas (selección de ventanas al pasar el ratón, herramientas de anotación, herramientas de inicio, ventanas de imágenes fijadas, captura larga, CLI headless
y la lista de autocomprobación de funciones), consulte el [Manual del usuario](../docs/user-guide.zh-CN.md)
([English](../docs/user-guide.md)).

Otras versiones de idioma:
[简体中文](../docs/user-guide.zh-CN.md) · [繁體中文](../docs/user-guide.zh-TW.md) · [日本語](../docs/user-guide.ja.md) · [한국어](../docs/user-guide.ko.md) · [Русский](../docs/user-guide.ru.md) · [Italiano](../docs/user-guide.it.md) · [العربية](../docs/user-guide.ar.md) · [Français](../docs/user-guide.fr.md) · [Deutsch](../docs/user-guide.de.md) · [Español](../docs/user-guide.es.md) · [Português](../docs/user-guide.pt.md)

## Compilación e instalación

### Guía de instalación

##### Arch Linux (AUR)
Los usuarios de Arch Linux pueden instalarlo directamente mediante un asistente de AUR:
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

`mark-shot` se compila desde el código fuente; `mark-shot-bin` descarga e instala el paquete pacman precompilado desde las Releases de GitHub.

##### NixOS
Los usuarios de NixOS pueden instalarlo añadiendo un Flake input
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

##### Otras distribuciones (paquetes precompilados)
Para otras distribuciones (como Ubuntu, Debian y Fedora), descargue el paquete compilado desde la página de Releases y ejecute los siguientes comandos para instalarlo:
- **Debian / Ubuntu**:
  ```bash
  sudo apt install ./mark-shot_<version>_amd64.deb
  ```
- **Fedora**:
  ```bash
  sudo dnf install ./mark-shot-<version>-1.x86_64.rpm
  ```

> **Ubuntu 26.04 LTS**: Mark Shot ha sido verificado y es compatible con Ubuntu 26.04 LTS (Resolute).
> En Ubuntu 26.04, compilar desde el código fuente puede usar directamente los paquetes Qt 6.10 de la propia distribución
> (sin necesidad del paso de `aqtinstall`):
>
> ```bash
> sudo apt install build-essential cmake ninja-build pkg-config \
>   qt6-base-dev qt6-wayland libpipewire-0.3-dev libxcb-cursor0 \
>   xdg-desktop-portal pipewire xclip
> cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
> cmake --build build
> ```
>
> La captura headless (`--capture-to`), la captura de varios monitores (`--display` repetible) y el servicio
> MCP local funcionan en sesiones Wayland (GNOME) y X11 de Ubuntu 26.04.

### Dependencias del sistema

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

> **Nota**: en sistemas con Qt 5 incluido, como Ubuntu 22.04, instalar Qt 6 en `~/Qt` no afecta al sistema. Simplemente pase `-DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64` al compilar.

#### Soporte de entrada en chino con fcitx5 (Qt 6 en entornos X11)

Qt 6 no incluye el plugin de método de entrada fcitx5. Si necesita usar fcitx5 para la entrada en chino en un entorno X11, debe compilar el plugin desde el código fuente:

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

#### Backend de OCR (opcional)

La función de reconocimiento de texto de Mark Shot depende del script de Python integrado `mark-shot-ocr`. Dicho script admite **RapidOCR** (preferido, basado en los modelos PaddleOCR PP-OCR) y **Tesseract** (respaldo). En Linux el script se instala automáticamente; en Windows debe configurarse manualmente.

<details>
<summary><b>Linux</b></summary>

```bash
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime
```

Tras la instalación, `mark-shot-ocr` se detecta automáticamente y no requiere configuración adicional.

**Variables de entorno** (opcionales):

| Variable | Descripción | Valor predeterminado |
|------|------|--------|
| `MARK_SHOT_OCR_VERSION` | Versión de PaddleOCR (`PP-OCRv5`, `PP-OCRv4`, etc.) | `PP-OCRv5` |
| `MARK_SHOT_OCR_MODEL_TYPE` | Tamaño del modelo: `mobile` o `server` | `mobile` |
| `MARK_SHOT_OCR_MODEL_DIR` | Directorio de almacenamiento de modelos personalizado | `~/.local/share/mark-shot/models` |
| `MARK_SHOT_OCR_NO_VENV` | Si se establece en `1`, desactiva el cambio automático al entorno virtual | — |
| `MARK_SHOT_OCR_PYTHON` | Ruta del intérprete de Python usado para el re-ejecución (re-exec) | `~/.local/share/mark-shot/ocr-venv/bin/python` |

</details>

<details>
<summary><b>Windows</b></summary>

El script auxiliar integrado no se instala automáticamente en Windows; complete los siguientes pasos manualmente:

**1. Instale Python 3**

Descargue e instale Python 3.10 o superior desde [python.org](https://www.python.org/downloads/). Durante la instalación, marque **Add python.exe to PATH**.

**2. Copie el script auxiliar de OCR**

Copie `../scripts/mark-shot-ocr` del [repositorio de Mark Shot](https://github.com/tystudio-26020701/mark-shot-community) a un directorio local, por ejemplo `%LOCALAPPDATA%\mark-shot\mark-shot-ocr.py`.

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\mark-shot"
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/tystudio-26020701/mark-shot-community/main/scripts/mark-shot-ocr" `
  -OutFile "$env:LOCALAPPDATA\mark-shot\mark-shot-ocr.py"
```

**3. Cree el entorno virtual e instale las dependencias**

```powershell
python -m venv "$env:LOCALAPPDATA\mark-shot\ocr-venv"
& "$env:LOCALAPPDATA\mark-shot\ocr-venv\Scripts\pip.exe" install -U pip rapidocr onnxruntime
```

> `onnxruntime` proporciona inferencia por CPU. Si tiene una GPU compatible, puede instalar `onnxruntime-directml` o `onnxruntime-gpu` para acelerar el reconocimiento.

**4. Configure `ocr.command` en `config.json`**

Abra `%LOCALAPPDATA%\mark-shot\config.json` (créelo si no existe) y establezca `ocr.command`:

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

Reemplace `%LOCALAPPDATA%` por la ruta real expandida (por ejemplo, `C:\Users\TuNombreDeUsuario\AppData\Local`). El marcador de posición `{image}` se sustituye en tiempo de ejecución por la ruta de la captura temporal; si se omite, Mark Shot lo añade automáticamente.

> **Consejo**: establezca la variable de entorno `MARK_SHOT_OCR_NO_VENV=1` para omitir la detección automática del entorno virtual integrada en el script, ya que se está usando directamente el Python del entorno virtual.

</details>

#### Backend de escaneo de códigos (opcional)

```bash
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

El helper de escaneo usa preferentemente `zxing-cpp`, que admite formatos habituales como QR Code, Data Matrix, Aztec, PDF417, EAN, UPC, Code 39, Code 93 y Code 128. Si están instalados `pyzbar` u OpenCV, también se usan como backends de respaldo.

#### Backend de subida a alojamiento de imágenes (opcional)

La función de subida a alojamiento de imágenes usa por defecto el script de Python integrado `mark-shot-upload`, sin necesidad de instalar dependencias adicionales (solo usa la biblioteca estándar de Python 3). El script se configura mediante variables de entorno y admite cualquier servicio de alojamiento compatible con el protocolo de subida multipart/form-data.

<details>
<summary>Variables de entorno admitidas por el helper integrado</summary>

| Variable de entorno | Descripción | Valor predeterminado |
|---------|------|--------|
| `MARK_SHOT_UPLOAD_URL` | **Requerido**: endpoint de la interfaz de subida del alojamiento | — |
| `MARK_SHOT_UPLOAD_FIELD` | Nombre del campo de archivo | `image` |
| `MARK_SHOT_UPLOAD_API_KEY` | Clave de API / Token | — |
| `MARK_SHOT_UPLOAD_AUTH_HEADER` | Nombre de la cabecera de autenticación | `Authorization` |
| `MARK_SHOT_UPLOAD_AUTH_SCHEME` | Esquema de autenticación (por ejemplo, `Bearer`); si se deja vacío, se usa directamente la clave de API | `Bearer` |
| `MARK_SHOT_UPLOAD_URL_PATH` | Ruta de puntos de la URL en la respuesta JSON (por ejemplo, `data.url`) | Detección automática |
| `MARK_SHOT_UPLOAD_DELETE_URL_PATH` | Ruta de la URL de borrado | Detección automática |
| `MARK_SHOT_UPLOAD_HEADER_xxx` | Cabeceras de solicitud personalizadas (por ejemplo, `MARK_SHOT_UPLOAD_HEADER_X-Custom=foo`) | — |
| `MARK_SHOT_UPLOAD_FIELD_xxx` | Campos de formulario adicionales (por ejemplo, `MARK_SHOT_UPLOAD_FIELD_album=123`) | — |

</details>

<details>
<summary>Ejemplo de configuración: ImgURL V3</summary>

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

ImgURL V3 usa autenticación `Authorization: Bearer <token>` (`AUTH_SCHEME` es `Bearer` por defecto, no es necesario modificarlo).

</details>

<details>
<summary>Ejemplo de configuración: sm.ms</summary>

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

sm.ms usa directamente el Token como valor de Authorization, por lo que `AUTH_SCHEME` se establece como una cadena vacía.

</details>

<details>
<summary>Ejemplo de configuración: imgbb</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://api.imgbb.com/1/upload?key=你的API_KEY",
    "MARK_SHOT_UPLOAD_FIELD": "image",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

imgbb pasa la clave de API mediante un parámetro de consulta de la URL, por lo que no es necesario establecer `API_KEY`.

</details>

<details>
<summary>Ejemplo de configuración: litterbox (alojamiento temporal de imágenes, sin necesidad de clave de API)</summary>

```json
"upload": {
  "command": "curl -sf --max-time 30 -A 'Mozilla/5.0' -F reqtype=fileupload -F time=72h -F fileToUpload=@{image} https://litterbox.catbox.moe/resources/internals/api.php",
  "timeoutMs": 35000
}
```

La respuesta de litterbox es una URL de texto plano (no JSON); Mark Shot reconoce automáticamente la salida que comienza por `http://`/`https://` como resultado de la subida.

</details>

<details>
<summary>Comando de subida personalizado</summary>

Si el helper integrado no cubre sus necesidades, puede integrar cualquier script de subida personalizado mediante `upload.command`. El comando debe cumplir:

1. **Código de salida**: el código de salida es 0 en caso de éxito; cualquier valor distinto de cero se considera un fallo
2. **Formato de salida** (una de dos):
   - **JSON**: `{"url":"https://...","deleteUrl":"https://...","errors":[]}` (`url` es obligatorio; el resto es opcional)
   - **URL de texto plano**: la primera línea no vacía de stdout comienza por `http://` o `https://`
3. **Marcadores de posición**: admite `{image}`, `{imagePath}` y `{imageUrl}`; si el comando no contiene ningún marcador de posición, Mark Shot añade automáticamente la ruta de la imagen temporal al final del comando

```json
"upload": {
  "command": "/path/to/your-uploader.sh --file {image} --json",
  "timeoutMs": 30000,
  "env": {
    "UPLOADER_API_KEY": "xxx"
  }
}
```

Las variables de entorno de `upload.env` también se pasan al comando personalizado, lo que facilita reutilizar la configuración.

</details>

#### Windows

Instale Qt 6, CMake y Ninja acordes con el compilador actual, además de un compilador compatible con C++17, como MSVC o MinGW. La compilación en Windows no necesita Qt DBus, PipeWire, X11/XCB, LayerShellQt, `grim`, `wl-copy` ni `xclip`.

```powershell
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
cmake --build build-windows
```

El alcance actual de soporte en Windows es la captura normal y la anotación de imágenes. La captura con desplazamiento, la detección de ventanas específica del compositor y los accesos directos de escritorio de Linux no están disponibles en Windows. Los scripts auxiliares de Python integrados (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`) no se instalan automáticamente; consulte el [Backend de OCR](#ocr-后端可选), el [Backend de escaneo de códigos](#扫码后端可选) y el apartado de traducción para configurarlos manualmente.

### Compilación y construcción

```bash
# 使用系统 Qt 6
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 如果 Qt 6 安装在用户目录，额外指定 CMAKE_PREFIX_PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64

# 执行编译
cmake --build build
```

O use nix

```bash
nix build
```

LayerShellQt se detecta automáticamente. Si se encuentra, se habilita el soporte completo de layer-shell de Wayland; si no, la compilación se completa con normalidad y en tiempo de ejecución se degrada automáticamente a una ventana estándar a pantalla completa.

### Instalación e integración

```bash
cmake --install build --prefix "$HOME/.local"
```

Este comando instala el ejecutable, los scripts auxiliares (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`, `mark-shot-upload`), los accesos directos de escritorio y los iconos.

### Extensión de captura con desplazamiento para GNOME Wayland

La captura con desplazamiento en GNOME Wayland requiere habilitar la extensión **Mark Shot Scroll Helper**. Sin esta extensión, Mark Shot no puede capturar de forma silenciosa y continua la región seleccionada ni dibujar el panel nativo de vista previa de desplazamiento de GNOME, por lo que el botón de captura con desplazamiento se deshabilita en GNOME Wayland.

Los archivos de la extensión se encuentran en la ruta `../packaging/gnome-extension/mark-shot-scroll-helper@snemc.org` del repositorio del proyecto.

<details>
<summary><b>Expandir/plegar la guía de instalación y activación de la extensión de captura con desplazamiento para GNOME Wayland</b></summary>

##### Opción A: instalación mediante el paquete de la distribución
Si instaló Mark Shot mediante un paquete de la distribución (como `.deb` o `.rpm`), la extensión ya viene instalada por defecto. Puede ejecutar el siguiente comando para habilitarla para el usuario actual:
```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```
*Si indica que no se encuentra la extensión, cierre sesión, vuelva a iniciarla y pruebe de nuevo.*

##### Opción B: instalación desde el directorio de código fuente del repositorio
Si compiló desde el código fuente o lo construyó localmente, primero debe copiar la extensión a la ruta de extensiones de GNOME del usuario:
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

Verifique que la interfaz D-Bus del helper está disponible:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
```

El resultado esperado es `('4.2',)`. Tras habilitar la extensión, reinicie `mark-shot`.

</details>

---

## Guía de atajos de teclado y gestos de interacción

### Atajos de cambio de herramienta

| Atajo | Herramienta de destino | Descripción de la función |
| :---: | :--- | :--- |
| **V** | Mover / Navegar (Move / Pan) | En el modo de imagen existente, se usa para desplazar y arrastrar el lienzo de la imagen. |
| **S** | Seleccionar (Select) | Selecciona y mueve, escala o elimina las anotaciones vectoriales ya dibujadas. |
| **P** | Lápiz (Pen) | Dibujo de curvas libres. |
| **L** | Línea (Line) | Dibuja líneas vectoriales rectas. |
| **H** | Resaltador (Highlighter) | Cobertura de resaltado semitransparente, ideal para marcar puntos importantes. |
| **R** | Rectángulo (Rectangle) | Dibuja marcos rectangulares. |
| **E** | Elipse (Ellipse) | Dibuja marcos elípticos. |
| **A** | Flecha (Arrow) | Dibuja la clásica flecha de seis vértices, afilada, larga y de ángulo agudo. |
| **T** | Texto (Text) | Introduce y compone texto enriquecido, con tamaños de fuente de hasta 1000 px y vinculación por arrastre. |
| **N** | Número (Number) | Etiquetas de paso numéricas autoincrementales. |
| **M** | Mosaico (Mosaic) | Difumina las regiones sensibles con efecto de vidrio esmerilado. |
| **G** | Lápiz láser (Laser) | Trazos temporales para clases o presentaciones que se desvanecen suavemente solos. |

### Herramientas auxiliares de la pantalla de inicio

| Atajo | Herramienta | Descripción de la función |
| :---: | :--- | :--- |
| **C** | Selector de color (Color Picker) | Muestrea los píxeles de la captura antes de seleccionar el área de captura. La rueda del ratón ajusta el tamaño de la lupa; un clic con el botón izquierdo abre el panel de colores, donde puede copiar formatos como HEX, RGB, HSL, HSV y Qt. El botón derecho o Esc vuelven a la selección normal. |
| **R** | Regla (Ruler) | Mide coordenadas antes de seleccionar el área de captura. Al pasar el ratón muestra el píxel actual; arrastrando con el botón izquierdo dibuja un rectángulo de medición con escala de píxeles y muestra la anchura, la altura, la diagonal y el área. El botón derecho o Esc vuelven a la selección normal. |
| **Q** | Escáner de códigos (Code Scanner) | Entra en el modo de escaneo de códigos QR y códigos de barras. Tras encuadrar un área, reconoce el contenido del código y muestra el resultado en una ventana que se puede copiar. El botón derecho o Esc vuelven a la selección normal. |
| **D** | Captura de monitor (Display Capture) | Captura al instante todas las pantallas de salida, las recorta por monitor y muestra miniaturas; al pasar el ratón sobre una miniatura puede copiar, editar o guardar. |

### Atajos de operación globales

| Atajo | Acción |
| :---: | :--- |
| **Esc** | Sale inmediatamente y cierra la ventana de anotación. |
| **Ctrl + C** | Confirma todas las ediciones de texto y copia la captura actual / la selección anotada al portapapeles del sistema. |
| **Ctrl + S** o **Enter / Return** | Confirma todas las ediciones de texto y guarda la captura actual. |
| **Ctrl + P** | Fija la selección actual como ventana de imagen flotante. |
| **Ctrl + U** | Sube la captura actual a un alojamiento de imágenes personalizado; la URL se copia automáticamente al portapapeles al completarse la subida. |
| **Ctrl + Z** | Deshace la última operación de anotación. |
| **Ctrl + Y** o **Ctrl + Shift + Z** | Rehace la operación de anotación deshecha. |
| **Backspace** o **Delete** | Cuando la herramienta **Seleccionar (Select)** está activa y hay una anotación seleccionada, elimina la anotación seleccionada. |
| **F** | Cambia el alcance de la captura actual (alterna entre modo de selección y modo de pantalla completa). |

### Técnicas avanzadas de interacción

- **Restricción de formas al dibujar**: al dibujar un **rectángulo (Rectangle)** o una **elipse (Ellipse)**, mantenga pulsada la tecla `Ctrl` para forzar un cuadrado o un círculo perfecto.
- **Cambio rápido a la herramienta de selección**: durante la anotación, un clic con el botón derecho en una zona vacía del lienzo cambia al instante a la herramienta **Seleccionar (Select)**.
- **Cambio rápido de color con doble clic derecho**: un doble clic con el botón derecho en una zona vacía del lienzo abre la paleta de colores circular para cambiar rápidamente el color de la herramienta de anotación actual.
- **Ajuste continuo con la rueda**: con la herramienta de anotación correspondiente activa, la rueda del ratón ajusta en tiempo real el grosor de línea, el tamaño de fuente, el tamaño de la etiqueta de número o el tamaño de la cuadrícula de mosaico de la herramienta actual.
- **Desplazamiento y zoom del lienzo**: en el modo de la herramienta **Seleccionar (Select)**, o al editar archivos locales, la rueda del ratón aplica un zoom sin interrupciones al lienzo y arrastrar con el botón central del ratón desplaza el lienzo. Haga doble clic en `Ctrl` para restablecer el zoom y el desplazamiento.

### Interacción específica de la ventana de imagen flotante

| Gesto / Atajo | Efecto |
| :--- | :--- |
| **Mantener pulsado el botón izquierdo y arrastrar** | Desplaza y coloca libremente la posición de la imagen fijada en el escritorio. |
| **Rueda del ratón hacia arriba/abajo** | Amplía/reduce la ventana de imagen flotante de forma proporcional y continua. |
| **Doble clic con el botón izquierdo** | Cierra rápidamente esa ventana de imagen flotante. |
| **Clic con el botón derecho** | Abre el menú de funciones (incluye rotar, copiar el texto de la imagen, traducir, guardar, copiar, cerrar, etc.). |
| **Tecla Esc** | Cierra la ventana de imagen flotante que tiene el foco. |

---

## Notas de las versiones

Vea las [Notas de las versiones](../docs/releases.zh-CN.md).

## Comentarios y contacto

### Envío de issues
Si encuentra problemas al ejecutar la aplicación o tiene sugerencias de nuevas funciones, le recomendamos usar la herramienta de línea de comandos GitHub CLI (`gh`) para enviar issues. Ofrecemos un script que recopila la información del entorno y genera el informe automáticamente con una sola acción; consulte la [Guía de envío de issues](../.doc/submit-issue-via-gh.md) para más detalles.

---

## Notas sobre la licencia

Este proyecto es de código abierto bajo la **licencia MIT**; consulte el archivo [LICENSE](../LICENSE) para más detalles.

## Agradecimientos

Mark Shot se apoya sobre los hombros de la comunidad de código abierto, a la que expresamos nuestro más sincero agradecimiento:

- **El proyecto upstream original [jswysnemc/mark-shot](https://github.com/jswysnemc/mark-shot) y su autor y todos sus colaboradores.** Esta edición comunitaria se desarrolla a partir del proyecto upstream original; su excelente diseño y su continua contribución son la base de todo esto. Agradecemos de corazón su magnífico trabajo.
- **[serendipitywgy](https://github.com/serendipitywgy)**: gracias por contribuir a través de `serendipitywgy/mark-shot` con mejoras de compatibilidad entre escritorios, la acción de copiado por OCR en la barra de herramientas y la función de preselección inteligente del marco rectangular.
- **Todos los proyectos de código abierto de los que depende Mark Shot**, entre ellos Qt 6, PipeWire, xdg-desktop-portal, layer-shell-qt, wl-clipboard, xclip, grim, RapidOCR, onnxruntime, Tesseract y ZXing-C++.

Esta edición comunitaria es mantenida por [北京太殷造物科技有限公司 (Beijing Taiyin Zhaowu Technology Co., Ltd.)](https://github.com/tystudio-26020701/mark-shot-community) y sus colaboradores, y es de código abierto bajo la **licencia MIT**.
