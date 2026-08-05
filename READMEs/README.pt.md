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

Leia este README em outros idiomas：
[简体中文](../README.zh-CN.md) · [繁體中文](./README.zh-TW.md) · [日本語](./README.ja.md) · [한국어](./README.ko.md) · [Русский](./README.ru.md) · [Italiano](./README.it.md) · [العربية](./README.ar.md) · [Français](./README.fr.md) · [Deutsch](./README.de.md) · [Español](./README.es.md) · [Português](./README.pt.md)

**Etiquetas**：`C++` / `Qt 6` / `屏幕截图` / `图像标注` / `桌面贴图` / `OCR 识别` / `滚动长截图` / `Wayland` / `Windows`


<details>
<summary>Vídeo de demonstração</summary>
<p align="center">
  <video src="https://github.com/user-attachments/assets/4f86fcee-fef9-409e-98ba-1491ecee06c7" width="100%" controls></video>
</p>
</details>

`mark-shot` é uma ferramenta de captura de tela e anotação de alto desempenho desenvolvida com Qt 6. O projeto foi inicialmente projetado para gerenciadores de janelas Wayland como o `niri` e, atualmente, oferece suporte aos fluxos de trabalho comuns de captura de tela e anotação em Linux (X11, GNOME, desktops wlroots/Wayland) e também em Windows.

Ele captura a tela instantaneamente e abre uma camada de anotação adaptável em tela cheia, oferecendo ao usuário funções como recorte de área, anotação, cópia para a área de transferência, salvamento e fixação na área de trabalho.

---

## Recursos principais

### Caixa de ferramentas de anotação
- **Pincel e marcador**: suporte ao desenho de linhas livres suaves e a sobreposições de realce semitransparentes.
- **Ferramentas vetoriais geométricas**: caminhos de alta precisão para linhas retas, retângulos e elipses. Os retângulos suportam três estilos alternáveis:
  - `描边`: o retângulo original com contorno ou preenchimento, com cantos arredondados opcionais.
  - `高亮`: efeito de sobreposição estilo marcador implementado com `CompositionMode_Multiply` e preenchimento semitransparente.
  - `反色`: inverte o RGB dos pixels na área coberta pelo retângulo, mantendo o contorno externo como indicação visual.
- **Seta otimizada**: usa o clássico caminho de seta com seis vértices, com bordas suaves e renderização antisserrilhada.
- **Texto duplo com vínculo dinâmico**:
  - Suporte ao ajuste contínuo de tamanhos de fonte muito grandes, com zoom suave pela roda do mouse ou pelo controle deslizante de propriedades.
  - Foi introduzido um design de buffer de largura física para evitar quebras de linha inesperadas em escalas de zoom muito altas, causadas pela trepidação de renderização.
  - Os **pontos de controle diagonais** permitem o redimensionamento proporcional conjunto do tamanho da fonte e da caixa de texto; as **linhas de controle laterais** ajustam apenas a largura dos limites do texto.
- **Caneta laser de apresentação**: adequada para demonstrações ou aulas; os traços se dissolvem suavemente com o tempo.
- **Números de etapa autoincrementais**: um clique coloca marcadores numéricos de etapa que aumentam sequencialmente.
- **Mosaico**: suporte ao desfoque de área estilo vidro fosco para informações sensíveis.
- **Lupa com ajuste independente das duas molduras**: a moldura interna de enquadramento e a lente externa da lupa possuem cada uma suas próprias alças de redimensionamento — a lente retangular tem 8 alças por moldura (cantos e lados) e a lente circular tem 4 alças por moldura (superior, inferior, esquerda e direita). Ao ajustar qualquer uma das molduras, a outra a acompanha de acordo com a ampliação, e a taxa de zoom permanece sempre inalterada; ao mover apenas uma moldura, a outra permanece no lugar.
- **Leitura de código na fase inicial**: pressione `Q` antes de fazer a seleção para entrar no modo de leitura de código; depois de selecionar a área de um QR Code ou de um código de barras, é aberta uma janela com o resultado reconhecido, que pode ser copiado.
- **Captura rápida de monitor**: pressione `D` antes de fazer a seleção para capturar imediatamente todas as telas de saída e recortá-las em miniaturas por monitor; passe o mouse sobre uma miniatura para copiar, editar ou salvar a captura daquele monitor.
- **Gravação de GIF, MP4 e WebP animado**: por meio da tecla de atalho de gravação na fase inicial ou do menu da bandeja, você pode gravar um monitor específico ou uma área personalizada como GIF, MP4 ou WebP animado. A gravação é à prova de falhas: o MP4 é gravado em um MKV temporário e remuxado ao final, portanto uma gravação interrompida permanece recuperável. A gravação ativa exibe seu status na bandeja e no quadro congelado, pode ser interrompida com `S`, pelo botão da camada de sobreposição, pelo menu da bandeja ou com `--stop-recording`, e envia notificações de desktop ao iniciar e ao salvar. No Wayland, a gravação usa preferencialmente o backend PipeWire portal; quando a captura por portal não está disponível, ela pode recuar para wlroots screencopy ou captura por sondagem.
- **Upload para hospedagem de imagens**: após a seleção, pressione `Ctrl+U` ou clique no botão de upload da barra de ferramentas para enviar a captura atual a um serviço de hospedagem personalizado (como ImgURL, sm.ms, imgbb, litterbox etc.); após o envio, a URL é copiada automaticamente para a área de transferência. Os parâmetros podem ser configurados por meio de `upload.env`, ou qualquer script de upload personalizado pode ser integrado por meio de `upload.command`.
- **Moldura de exportação estilo Mac**: adiciona margens transparentes, cantos arredondados e sombras suaves às imagens ao salvar, copiar, enviar, abrir com e nos comandos de extensão.

### Fixação flutuante na área de trabalho (Pin)
- Suporte para fixar na tela a captura ou a área anotada como uma janela flutuante independente, sem bordas e sempre no topo.
- Suporte para selecionar diretamente o texto reconhecido por OCR na janela de fixação e copiar o texto da imagem com `Ctrl + C` ou pelo menu de contexto.
- Suporte para chamar um LLM por meio de uma interface compatível com OpenAI para traduzir o texto do OCR e renderizar a tradução sobreposta na imagem fixada, na posição correspondente à original.
- **Interação conveniente**:
  - Arraste com o botão esquerdo do mouse para mover livremente a posição da imagem fixada.
  - Role a roda do mouse para redimensionar a imagem fixada proporcionalmente.
  - Clique duas vezes com o botão esquerdo ou pressione `Esc` para fechar a imagem fixada.
  - Clique com o botão direito para abrir o menu, com suporte a rotação em vários ângulos, copiar texto da imagem, traduzir, salvar como, copiar ou fechar.

### Captura de rolagem
- Capture páginas ou áreas longas por meio do screencast do PipeWire, de uma camada de sobreposição de rolagem interativa e de um montador de imagens.
- Esse recurso é voltado principalmente para o `niri` e ambientes Wayland de comportamento semelhante; nesses ambientes, a geometria de saída, o tempo de captura e a posição das janelas permanecem mais estáveis.
- **Alça flutuante para seleções grandes**: quando a área de captura selecionada é tão grande que o espaço restante da tela não comporta o painel de visualização da rolagem, o painel é ocultado automaticamente e uma **alça flutuante de arrastar** (um botão flutuante com setas de direção) é exibida na borda da seleção.
  - **Arrastar para ajustar a seleção**: mantenha pressionada e arraste a alça flutuante para mover a área de captura ao longo do eixo de rolagem, capturando conteúdo além da extensão inicial da tela;
  - **Clique para alternar o eixo**: antes de iniciar a captura, um clique na alça flutuante alterna a direção da rolagem (vertical/horizontal).
- **Nota de compatibilidade**: a captura de rolagem em KDE, GNOME, X11 e outros ambientes que não são o `niri` ainda é um recurso experimental e incompleto. As políticas de backends de portal, o comportamento do Shell ou do gerenciador de janelas, o feedback da geometria das janelas, o tempo dos quadros e o tratamento dos eventos de rolagem dessas pilhas de desktop apresentam diferenças.
- Se a captura de rolagem não estiver disponível, use o fluxo normal de captura ou integre uma ferramenta externa de captura longa por meio dos comandos de extensão do DracoPho.
- Se precisar relatar um problema com a captura de rolagem, execute primeiro `mark-shot --debug --debug-log /path/to/mark-shot.log` e reproduza o problema; depois, anexe o log à issue do GitHub. A depuração também pode ser ativada em `config.json` por meio de `debug.enabled` e `debug.logPath`; `DEBUG=1` e `MARK_SHOT_DEBUG_LOG=/path/to/log` continuam funcionando.

### Suporte a diferentes servidores de exibição
- **Wayland**: usa o screencast do PipeWire portal para dar suporte à gravação e à captura de rolagem experimental, lidando com os dois tipos de caminho de quadros (memória compartilhada e DMA-BUF); usa `grim` para a captura de tela no wlroots, `layer-shell-qt` para criar camadas de sobreposição nativas e `wl-copy` para persistir a área de transferência.
- **X11**: usa `QScreen::grabWindow` para capturar a tela, uma janela em tela cheia sempre no topo como camada de sobreposição e `xclip` para persistir a área de transferência.
- **Windows**: usa as APIs nativas do Qt de captura de tela e de área de transferência para dar suporte aos fluxos básicos de captura, anotação, cópia, salvamento e fixação. Backends específicos do Linux, como PipeWire, xdg-desktop-portal, `grim`, detecção de janelas XCB, LayerShellQt e o GNOME Shell helper, são desativados em tempo de compilação.
- O backend do servidor de exibição do Linux é detectado automaticamente em tempo de execução por meio de `$XDG_SESSION_TYPE`; no Windows, é usado o backend de plataforma nativo do Qt.
- **Multi-monitor freeze scope**: por padrão, a seleção de região congela todos os monitores conectados (uma única janela de área de trabalho virtual quando as DPRs coincidem no X11/Windows); após confirmar uma seleção em um monitor, os demais monitores permanecem congelados e não interativos até o fim da sessão. O escopo **Cursor Screen** congela apenas o monitor sob o cursor.

### Integração com a área de trabalho
- **Comportamento de inicialização configurável**: iniciar o DracoPho não abre mais a sobreposição de captura por padrão. Em Configurações → Geral → **Comportamento na inicialização** você pode combinar os quatro modos: **Ícone na bandeja**, **Bola flutuante**, **Janela de configurações** e **Captura direta**. Instalações novas usam Ícone na bandeja + Bola flutuante; a captura direta é opcional.
- **Bola flutuante**: um pequeno widget circular sempre no topo (canto inferior direito por padrão) que oferece acesso rápido à captura, captura de tela cheia, gravação e configurações. Um clique abre o menu, duplo clique captura, arrastar move (a posição é lembrada) e ela se oculta automaticamente durante a captura.
  Ao soltá-la perto da borda da tela, ela se encaixa e **metade da bola desliza para fora da tela** (passe o mouse para revelá-la e afaste-o para escondê-la novamente; funciona em todas as plataformas) e, após alguns segundos ociosa, ela esmaece para semitransparente, voltando à opacidade total ao passar o mouse.
- **Atalhos da área de trabalho**:
  - `mark-shot.desktop`: configurado como ferramenta global de captura de tela do sistema, com suporte à chamada direta por teclas de atalho do sistema.
  - `mark-shot-edit.desktop`: registrado como um editor de imagens independente, pode ser integrado ao menu de contexto "Abrir com" de gerenciadores de arquivos (como Dolphin, Nautilus).
- Inclui os ícones vetoriais de sistema de alta resolução `dracoPho.svg` e `mark-shot-edit.svg`.

### Autorização do KDE KWin ScreenShot2

No KDE Wayland, o DracoPho pode usar a interface `org.kde.KWin.ScreenShot2` do KWin para realizar capturas de área precisas. O KWin trata essa interface como uma interface D-Bus restrita; portanto, o arquivo .desktop do aplicativo deve declarar o campo de autorização.

<details>
<summary>Notas sobre a autorização do KDE KWin ScreenShot2 e a configuração do arquivo .desktop (clique para expandir)</summary>

Declare o campo de autorização:
```ini
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

Os pacotes das distribuições e o `cmake --install` instalam automaticamente os arquivos .desktop necessários. Se você estiver executando diretamente o artefato de build local sem instalar o projeto, crie ou atualize `~/.local/share/applications/mark-shot.desktop`:

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

Se o DracoPho for vinculado por meio do serviço de atalhos de comando do KDE, também será necessário criar `~/.local/share/applications/net.local.mark-shot.desktop`:

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

Depois de modificar os arquivos .desktop, é recomendável fazer logout e login novamente para que o KDE releia o cache dos arquivos .desktop. Se a sessão atual do KDE ainda retornar `NoAuthorized`, reinicie o KWin ou reinicie o sistema uma vez.
</details>


---

## Comparação de produtos

A Edição Comunitária do DracoPho é uma ferramenta integrada de captura de tela, anotação, fixação (pin), OCR, tradução e gravação, **open source (MIT), multiplataforma (nativa em Linux X11/Wayland + Windows)** e **totalmente offline**. As tabelas abaixo foram compiladas com base na documentação e nos sites oficiais de cada produto (até **agosto de 2026**) e abrangem as ferramentas de captura de tela mais populares — open source e comerciais — em todas as principais plataformas de desktop. As capacidades são marcadas com honestidade: **✅ suporte integrado**; **⭕ suporte parcial (limitado por assinatura paga, plataforma ou ferramenta/serviço externo)**; **❌ não disponível**. Elas são sempre contadas pelo critério "pronto para uso" (out of the box) — sem vínculo com assinaturas pagas, serviços em nuvem ou ramificações experimentais; as limitações exatas de cada ⭕ são explicadas nas notas detalhadas abaixo.

### Matriz de capacidades principais

#### I. Captura (você consegue capturar o que precisa)

| Capacidade | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Captura de região | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Captura em tela cheia / vários monitores | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Captura de janela | ⭕ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Conteúdo de janela oculta / minimizada | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Captura de rolagem / longa | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ⭕ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Captura programada / com atraso | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⭕ | ❌ | ✅ |

#### II. Anotação e inteligência (você consegue trabalhar com eficiência após a captura)

| Capacidade | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Conjunto de ferramentas de anotação | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Fixar na tela (Pin) | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ |
| Reconhecimento de texto OCR | ✅ | ✅ | ✅ | ⭕ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Tradução | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| Reconhecimento de QR / código de barras | ✅ | ✅ | ✅ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ⭕ |
| Conta-gotas (seletor de cores) | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ✅ | ✅ | ✅ | ✅ |

#### III. Saída e automação (você consegue compartilhar/arquivar o resultado)

| Capacidade | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Gravação de tela | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ |
| Saída em GIF / WebP animado | ✅ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ |
| Upload de imagens / nuvem | ✅ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ |
| CLI headless / scripts | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| Atalho pela bola flutuante | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Histórico de capturas | ❌ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ | ⭕ | ❌ | ✅ | ✅ | ⭕ | ❌ | ⭕ |

#### IV. Plataforma e ecossistema

| Capacidade | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Wayland nativo no Linux | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Multiplataforma (≥2 SO de desktop) | ⭕ | ❌ | ⭕ | ✅ | ✅ | ✅ | ❌ | ⭕ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ |
| Mecanismo de plug-ins / extensões | ✅ | ⭕ | ❌ | ❌ | ⭕ | ✅ | ⭕ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| Open source / gratuito | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ✅ | ⭕ | ✅ | ❌ | ❌ | ✅ | ⭕ | ⭕ |

### Notas detalhadas sobre as funcionalidades

**I. Captura** (você consegue capturar o que precisa)

- **Captura de região / tela cheia / vários monitores** — integrada em todas as 15 ferramentas listadas; o DracoPho, adicionalmente, lida corretamente com monitores de coordenadas negativas e com a escala HiDPI por monitor, sem "fantasmas" ao unir capturas de vários monitores.
- **Captura de janela** — o modo interativo do DracoPho usa uma camada de seleção em tela cheia (sem um modo dedicado de um clique de "janela ativa"; por isso ⭕), enquanto o modo headless (`--window`) seleciona janelas com precisão por id, título, classe, **PID ou nome do processo**; o Flameshot não tem nenhum modo de janela, e todas as outras ferramentas incluem um.
- **Conteúdo de janela oculta / minimizada** — no X11, o DracoPho lê o buffer composto da própria janela via XComposite, capturando o conteúdo real mesmo quando a janela está totalmente oculta ou minimizada (sem trazê-la à frente e sem roubar o foco); entre as ferramentas similares, apenas o `--stack` do `scrot` consegue fazer isso. O protocolo do Wayland impede que qualquer ferramenta leia o conteúdo de janelas minimizadas.
- **Captura de rolagem / longa** — o DracoPho funciona nativamente no niri e no GNOME Wayland (extensão oficial), com KDE/X11 como experimental. ShareX, PixPin, PicPick, Snagit, CleanShot X, Shottr, Xnip e iShot a incluem nativamente; o Greenshot apenas para cenários legados de IE; Snipaste, Flameshot, ksnip, Spectacle e a Ferramenta de Recorte do Windows não a incluem.
- **Captura programada / com atraso** — o DracoPho aguarda o número de segundos configurado com uma sobreposição de contagem regressiva em tela cheia (Esc para cancelar) antes de iniciar a captura, via `--delay`, o submenu «Captura com atraso» da bandeja/bola flutuante ou a página de configurações de captura; Flameshot, ksnip, Spectacle e a maioria oferecem apenas um início retardado simples, o ShareX uma captura programada, e PixPin, Snipaste e Xnip nenhuma.

**II. Anotação e inteligência** (você consegue trabalhar com eficiência após a captura)

- **Conjunto de ferramentas de anotação** — o DracoPho inclui 12+ ferramentas: pincel, marcador, linha, retângulo, elipse, seta, texto, números de etapa, mosaico, lupa de duas molduras, caneta laser, conta-gotas e régua; todas as 15 ferramentas têm anotação integrada com profundidades variadas.
- **Fixar na tela (Pin)** — o DracoPho oferece adesivos sem bordas sempre no topo, com escala, rotação, seleção de palavras via OCR e tradução no lugar via LLM; Snipaste, PixPin, ShareX, ksnip, CleanShot X, Shottr, Xnip e iShot também a incluem; Flameshot, Spectacle, Greenshot, PicPick, a Ferramenta de Recorte e o Snagit não a incluem.
- **Reconhecimento de texto OCR** — o DracoPho inclui RapidOCR (modelos PP-OCR offline) com fallback para Tesseract, funcionando pronto para uso; a Ferramenta de Recorte do Windows (Ações de Texto locais), ShareX, Snagit, CleanShot X, Shottr, Xnip e iShot a incluem; o Snipaste a restringe a uma assinatura paga, o ksnip via plug-in e o Spectacle via instalação externa do Tesseract; Flameshot, Greenshot e PicPick não têm nenhuma.
- **Tradução** — o DracoPho inclui tradução de capturas por LLM com interface compatível com OpenAI (com suporte offline / auto-hospedado). Apenas o iShot inclui tradução de capturas, o PixPin a restringe à assinatura e nenhuma outra ferramenta oferece. Esse continua sendo um dos diferenciais mais fortes.
- **Reconhecimento de QR / código de barras** — o DracoPho lê QR, códigos unidimensionais e PDF417 com a tecla `Q` durante a captura; ShareX e PixPin oferecem suporte; o Snipaste (assinatura), além de CleanShot X / Shottr / iShot, leem apenas QR, por meio de seus mecanismos de OCR.
- **Conta-gotas (seletor de cores)** — o DracoPho tem um seletor de cores de tela integrado com paleta de histórico; a maioria das ferramentas inclui um, exceto Spectacle e Snagit; a Ferramenta de Recorte do Windows apenas em PCs com IA Copilot+.
- **Mascaramento com IA em um clique** — o mascaramento automático adicionado em 2026 à captura do WeChat ainda não está integrado (❌); o mosaico/desfoque do DracoPho é manual. O Snagit (AI Smart Redact) e a redação de texto da Ferramenta de Recorte são implementações comparáveis; o mascaramento automatizado é um possível próximo passo.

**III. Saída e automação** (você consegue compartilhar/arquivar o resultado)

- **Gravação de tela** — o DracoPho grava MP4 / GIF / WebP animado, com gravação silenciosa e sem supervisão de região/monitor (sem janelas, sem diálogos e sem roubo de foco); ShareX, PixPin, Spectacle (Plasma 6), PicPick, a Ferramenta de Recorte, Snagit, CleanShot X e iShot a incluem; Snipaste, Flameshot, ksnip, Greenshot, Shottr e Xnip não a incluem.
- **Saída em GIF / WebP animado** — o DracoPho grava nativamente **WebP animado** (arquivos menores, suporte a alfa); nenhum concorrente produz WebP animado nativamente. PixPin e iShot gravam GIF, o ShareX grava GIF (WebP apenas para salvamento estático), Snagit e CleanShot X exportam GIF; os demais não têm saída animada.
- **Upload de imagens / nuvem** — o DracoPho inclui upload via ImgURL / sm.ms / imgbb / litterbox / comando personalizado (`Ctrl+U`); o ShareX tem o maior número de destinos; Greenshot, PicPick, Snagit e CleanShot X têm upload em nuvem integrado; o Flameshot apenas Imgur, o ksnip Imgur/FTP/scripts e o Shottr S3 após ativação; Snipaste, PixPin, a Ferramenta de Recorte, Xnip e iShot não têm nenhum.
- **CLI headless / scripts** — o DracoPho oferece um pipeline headless completo: `--capture-to` para capturas de região/monitor, `--list-windows` + `--window` para capturas de janela/componente, `--record-region` / `--record-display` para gravação sem supervisão e `--record-wait-json` para emitir o status estruturado — o tempo todo sem janelas, sem diálogos e sem roubo de foco, com **seleção por PID/nome do processo e captura de janelas ocultas/minimizadas**. O MCP server da edição empresarial reutiliza o mesmo pipeline. ShareX (somente Windows), Flameshot, ksnip e Spectacle têm CLIs sólidas; a CLI do Snipaste é um recurso de assinatura paga; o PixPin oferece apenas um mecanismo de ações por script JS.
- **Atalho pela bola flutuante** — o DracoPho tem uma bola arrastável que se encaixa nas bordas da tela (X11/Windows/macOS; o Wayland a mantém flutuando livremente por limites do protocolo) e esmaece quando ociosa; apenas o PixPin oferece algo parecido.
- **Histórico de capturas** — o DracoPho ainda não tem um painel de histórico dedicado; ShareX, Snipaste, Flameshot, Snagit e CleanShot X têm; ksnip / PicPick / Shottr / iShot têm alternativas leves (abas / galeria / acervo fixado); os demais não têm.

**IV. Plataforma e ecossistema**

- **Wayland nativo no Linux** — o DracoPho oferece suporte nativo a PipeWire portal, grim, layer-shell, KDE KWin ScreenShot2 e extensões do GNOME; entre as ferramentas open source, apenas o Spectacle (KDE) alcança esse nível de profundidade, enquanto Flameshot / ksnip são experimentais ou dependentes de portal.
- **Multiplataforma** — Snipaste, Flameshot, ksnip e Snagit cobrem os três principais sistemas operacionais de desktop (Windows + macOS + Linux); o DracoPho cobre Linux + Windows (macOS planejado); ShareX, PicPick, a Ferramenta de Recorte, Spectacle, CleanShot X, Shottr, Xnip e iShot são de plataforma única.
- **Mecanismo de plug-ins / extensões** — o DracoPho oferece um sistema de plug-ins Qt com um mercado de plug-ins no GitHub (providers extensíveis de OCR / tradução / leitura de código); ksnip e Greenshot têm APIs de plug-in; ShareX, Spectacle, Snagit, CleanShot X e Shottr os substituem por ações / integrações personalizadas.
- **Open source / gratuito** — a Edição Comunitária do DracoPho é licenciada sob MIT, totalmente gratuita, sem anúncios, sem conta e com conexão opcional; ShareX, Flameshot, ksnip, Spectacle e Greenshot (Windows) também são gratuitos/open source; Shottr e a Ferramenta de Recorte são gratuitos; PixPin / Snipaste / PicPick / Xnip / iShot são freemium de código fechado; Snagit / CleanShot X são softwares comerciais pagos.

> Nota: compilado com base nos sites e na documentação oficiais de cada produto (2026-08); o conjunto de capacidades muda a cada versão, portanto consulte sempre a documentação mais recente. FastStone Capture, Nimbus, Lightshot, Captura do Sogou e a captura de tela do WeChat / QQ não estão na matriz acima (o mascaramento com IA em um clique do WeChat de 2026 lidera o segmento).

---

## Interface de linha de comando (CLI)

### Exemplos de uso comuns

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

#### Captura sem interface (não interativa)

Scripts, automação de CI ou outros programas podem chamar o `mark-shot` para realizar capturas sem abrir a interface de anotação.
O quadro capturado é gravado em PNG, e uma linha compacta de resumo JSON é impressa na saída padrão:

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

Exemplo da saída JSON de `--capture-to` com um único monitor:

```json
{"path":"/tmp/shot.png","width":2560,"height":1440,"output":"DP-1","error":null}
```

Quando vários `--display` são especificados, a saída se torna um array com uma captura por tela:

```json
{"captures":[{"path":"/tmp/shots/mark-shot-DP-1-20260801-000000.png","width":2560,"height":1440,"output":"DP-1","error":null},
             {"path":"/tmp/shots/mark-shot-DP-2-20260801-000000.png","width":1920,"height":1080,"output":"DP-2","error":null}]}
```

Cada monitor selecionado é capturado usando sua própria geometria de origem; portanto, backends do tipo portal retornam
precisamente esse monitor, e não o desktop virtual inteiro.

A captura sem interface reutiliza todos os mesmos backends de captura da interface interativa (QScreen,
xdg-desktop-portal, PipeWire, grim, KWin/GNOME helpers, Windows Graphics Capture);
portanto, a qualidade da imagem e o comportamento do recorte de área são idênticos. Todos os parâmetros sem interface são
mutuamente exclusivos com o parâmetro de arquivo de imagem posicional.

### Descrição dos parâmetros da CLI

| Opção | Descrição |
| :--- | :--- |
| `[file]` | **Parâmetro posicional**: abre um arquivo de imagem local existente para entrar no modo de anotação, em vez de capturar a tela atual. |
| `-h`, `--help` | Exibe as informações de ajuda e sai. |
| `-v`, `--version` | Exibe a versão atual e sai. |
| `--all-outputs` | Captura todas as telas de saída do desktop virtual, em vez de apenas a tela ativa atual. |
| `--xdg-window` | Força o uso de uma janela normal em tela cheia XDG (xdg-shell) em vez da camada de sobreposição Wayland padrão (layer-shell). |
| `--fullscreen` | Pula a etapa de seleção e anota diretamente a captura de tela inteira. |
| `--default-tool <tool>` | Define a ferramenta de anotação padrão após a seleção normal; também é usada como padrão do modo tela cheia quando `--fullscreen-default-tool` não é definido. |
| `--fullscreen-default-tool <tool>` | Define a ferramenta padrão do modo de anotação em tela cheia. |
| `--default-color <color>` | Define a cor padrão de anotação. Suporta `#RRGGBB` e `#RRGGBBAA`. |
| `--tray` | Mantém o DracoPho em execução na bandeja do sistema e registra a tecla de atalho global de captura quando a plataforma suporta. |
| `--capture` | Força a realização de uma única captura quando a inicialização automática pela bandeja está habilitada na configuração. |
| `--pin-image <path>` | Abre diretamente uma imagem local como janela de fixação, pulando o fluxo de captura e seleção. |
| `--recording-status` | Emite o JSON do status atual da gravação por meio da instância em execução. |
| `--stop-recording` | Solicita que a instância em execução interrompa a gravação ativa atual. |
| `--record-region <x,y,w,h>` | Grava uma região da tela por meio da instância em execução; geometria como x,y,largura,altura. |
| `--record-display <id>` | Grava um monitor por id (consulte a saída de `--list-displays`: um nome de tela bruto como `DP-1`, a chave `output:DP-1` ou `all`). |
| `--record-output <path>` | Caminho do arquivo de saída da gravação (obrigatório com os sinalizadores de gravação). |
| `--record-duration <seconds>` | Duração da gravação em segundos; 0 grava até `--stop-recording`. |
| `--record-fps <n>` | Taxa de quadros da gravação (padrão 15). |
| `--record-format <mp4\|gif\|webp>` | Formato de gravação: mp4 (padrão), gif ou webp. |
| `--record-audio` | Inclui o áudio do sistema na gravação. |
| `--record-wait-json` | Aguarda a gravação terminar e então imprime o status final como JSON. |
| `--debug` | Habilita o log de depuração para esta execução. |
| `--no-debug` | Desabilita o log de depuração para esta execução, sobrescrevendo o arquivo de configuração e as variáveis de ambiente. |
| `--debug-log <path>` | Grava o log de depuração no caminho especificado; habilita o log de depuração a menos que `--no-debug` também seja definido. |
| `--capture-to <path>` | Captura sem interface: grava o PNG no arquivo ou diretório especificado, sem abrir a interface; imprime um resumo JSON na saída padrão. |
| `--region <x,y,w,h>` | Usado com `--capture-to`: captura apenas a região lógica de tela especificada. |
| `--display <name>` | Usado com `--capture-to`: captura a tela de saída especificada pelo nome do monitor. Pode ser repetido para capturar vários monitores de uma vez (um PNG por tela). |
| `--include-cursor` | Usado com `--capture-to`: desenha o ponteiro do mouse no quadro capturado. |
| `--output-name <name>` | Usado com `--capture-to`: nome de arquivo base usado quando o caminho de captura é um diretório (sem extensão). |
| `--list-displays` | Emite as informações de todos os monitores atuais em JSON e sai. |

### Atalhos de teclado

Vincule o `mark-shot` como a tecla de atalho de captura do sistema:

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

**Sway / i3** (modifique `~/.config/sway/config` ou `~/.config/i3/config`):
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bindsym Mod4+Shift+S exec mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bindsym Print exec mark-shot
```

**GNOME**: adicione em Configurações do sistema → Teclado → Atalhos de teclado → Atalhos personalizados.

**Modo bandeja**:
```powershell
mark-shot --tray
```

O modo bandeja registra por padrão os seguintes atalhos globais:
- `Ctrl+Alt+S`: inicia a captura de área.

O menu da bandeja também oferece ações como captura, captura em tela cheia, iniciar gravação, status da gravação, parar gravação, configurações e sair.


### Comandos de extensão

A barra de ferramentas de ações à direita fornece o botão **Extensions**; o programa lê os comandos personalizados do usuário em `~/.config/mark-shot/extensions.json`. O arquivo de configuração pode ser um array JSON ou um objeto JSON contendo um array `commands`.

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

`command` é executado por meio de `$SHELL -c` em sistemas tipo Unix e por `%COMSPEC% /C` no Windows, portanto aceita expressões de shell. Use `{slurp}` para passar a seleção atual ao comando como uma string de geometria `x,y widthxheight`. Use `{image}` ou `{imagePath}` para passar a seleção renderizada atual como um caminho PNG temporário, e `{imageUrl}` para passar uma URL `file://`. Esses espaços reservados são escapados automaticamente para citações de shell — não acrescente aspas adicionais na configuração. Se nenhum espaço reservado de imagem for usado, defina `saveImage` ou `needsImage` como `true`, e o programa anexará automaticamente o caminho PNG temporário ao final do comando. `workingDirectory` é equivalente a `cwd`. O valor padrão de `closeOnStart` é `true`; o DracoPho é ocultado e fechado antes de o comando ser iniciado.

### Arquivo de configuração do aplicativo

Consulte a [referência de configuração](../docs/configuration.zh-CN.md).

### Manual do usuário

Para as operações do dia a dia (seleção ao passar o mouse sobre janelas, ferramentas de anotação, ferramentas de inicialização, janelas de fixação, captura de rolagem, CLI headless
e a lista de verificação de autoteste de recursos), consulte o [manual do usuário](../docs/user-guide.zh-CN.md)
([English](../docs/user-guide.md)).

Leia este manual em outros idiomas：
[简体中文](../docs/user-guide.zh-CN.md) · [繁體中文](../docs/user-guide.zh-TW.md) ·
[日本語](../docs/user-guide.ja.md) · [한국어](../docs/user-guide.ko.md) ·
[Русский](../docs/user-guide.ru.md) · [Italiano](../docs/user-guide.it.md) ·
[العربية](../docs/user-guide.ar.md) · [Français](../docs/user-guide.fr.md) ·
[Deutsch](../docs/user-guide.de.md) · [Español](../docs/user-guide.es.md) ·
[Português](../docs/user-guide.pt.md)

## Compilação e instalação

### Guia de instalação

##### Arch Linux (AUR)
Os usuários de Arch Linux podem instalar diretamente por meio de um assistente AUR:
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

`mark-shot` é compilado a partir do código-fonte; `mark-shot-bin` baixa o pacote pacman pré-compilado do GitHub Releases e o instala.

##### NixOS
Os usuários de NixOS podem instalar adicionando um Flake input
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

##### Outras distribuições (pacotes pré-compilados)
Para outras distribuições (como Ubuntu, Debian, Fedora), baixe o pacote compilado na página Releases e execute o seguinte comando para instalar:
- **Debian / Ubuntu**:
  ```bash
  sudo apt install ./mark-shot_<version>_amd64.deb
  ```
- **Fedora**:
  ```bash
  sudo dnf install ./mark-shot-<version>-1.x86_64.rpm
  ```

> **Ubuntu 26.04 LTS**: o DracoPho foi verificado e tem suporte no Ubuntu 26.04 LTS (Resolute).
> No Ubuntu 26.04, a compilação a partir do código-fonte pode usar diretamente os pacotes Qt 6.10 fornecidos pela distribuição
> (sem a etapa do `aqtinstall`):
>
> ```bash
> sudo apt install build-essential cmake ninja-build pkg-config \
>   qt6-base-dev qt6-wayland libpipewire-0.3-dev libxcb-cursor0 \
>   xdg-desktop-portal pipewire xclip
> cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
> cmake --build build
> ```
>
> A captura headless (`--capture-to`), a captura de vários monitores (repetível `--display`) e o serviço MCP local
> funcionam tanto em sessões Wayland (GNOME) quanto X11 no Ubuntu 26.04.

### Dependências do sistema

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

> **Nota**: em ambientes que já trazem o Qt 5 no sistema, como o Ubuntu 22.04, instalar o Qt 6 em `~/Qt` não afeta o sistema. Na compilação, basta passar `-DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64`.

#### Suporte à entrada de chinês com fcitx5 (Qt 6 em ambiente X11)

O Qt 6 não inclui o plugin de método de entrada fcitx5. Para usar a entrada de chinês com fcitx5 em um ambiente X11, é necessário compilar esse plugin a partir do código-fonte:

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

O recurso de reconhecimento de texto do DracoPho depende do script Python integrado `mark-shot-ocr`. Esse script suporta **RapidOCR** (preferido, baseado nos modelos PaddleOCR PP-OCR) e **Tesseract** (fallback). No Linux, o script é instalado automaticamente; no Windows, é necessária uma configuração manual.

<details>
<summary><b>Linux</b></summary>

```bash
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime
```

Após a instalação, o `mark-shot-ocr` é detectado automaticamente, sem configuração adicional.

**Variáveis de ambiente** (opcional):

| Variável | Descrição | Valor padrão |
|------|------|--------|
| `MARK_SHOT_OCR_VERSION` | Versão do PaddleOCR (`PP-OCRv5`, `PP-OCRv4` etc.) | `PP-OCRv5` |
| `MARK_SHOT_OCR_MODEL_TYPE` | Tamanho do modelo: `mobile` ou `server` | `mobile` |
| `MARK_SHOT_OCR_MODEL_DIR` | Diretório de armazenamento de modelos personalizados | `~/.local/share/mark-shot/models` |
| `MARK_SHOT_OCR_NO_VENV` | Defina como `1` para desativar a troca automática de ambiente virtual | — |
| `MARK_SHOT_OCR_PYTHON` | Caminho do interpretador Python usado para o re-exec | `~/.local/share/mark-shot/ocr-venv/bin/python` |

</details>

<details>
<summary><b>Windows</b></summary>

O script auxiliar integrado não é instalado automaticamente no Windows; os passos a seguir precisam ser feitos manualmente:

**1. Instale o Python 3**

Baixe e instale o Python 3.10 ou superior em [python.org](https://www.python.org/downloads/). Durante a instalação, marque a opção **Add python.exe to PATH**.

**2. Copie o script auxiliar de OCR**

Copie o `scripts/mark-shot-ocr` do [repositório DracoPho](https://github.com/tystudio-26020701/mark-shot-community) para um diretório local, por exemplo `%LOCALAPPDATA%\mark-shot\mark-shot-ocr.py`.

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\mark-shot"
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/tystudio-26020701/mark-shot-community/main/scripts/mark-shot-ocr" `
  -OutFile "$env:LOCALAPPDATA\mark-shot\mark-shot-ocr.py"
```

**3. Crie um ambiente virtual e instale as dependências**

```powershell
python -m venv "$env:LOCALAPPDATA\mark-shot\ocr-venv"
& "$env:LOCALAPPDATA\mark-shot\ocr-venv\Scripts\pip.exe" install -U pip rapidocr onnxruntime
```

> `onnxruntime` fornece inferência via CPU. Se houver uma GPU compatível, você pode instalar `onnxruntime-directml` ou `onnxruntime-gpu` para acelerar o reconhecimento.

**4. Configure `ocr.command` em `config.json`**

Abra `%LOCALAPPDATA%\mark-shot\config.json` (crie se não existir) e defina `ocr.command`:

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

Substitua `%LOCALAPPDATA%` pelo caminho expandido real (como `C:\Users\seu-nome-de-usuário\AppData\Local`). O espaço reservado `{image}` é substituído em tempo de execução pelo caminho da captura temporária; se for omitido, o DracoPho o anexa automaticamente.

> **Dica**: definir a variável de ambiente `MARK_SHOT_OCR_NO_VENV=1` faz o script ignorar a detecção automática de ambiente virtual, pois o Python do ambiente virtual já é usado diretamente.

</details>

#### Backend de leitura de código (opcional)

```bash
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

O helper de leitura de código usa preferencialmente `zxing-cpp` e suporta formatos comuns como QR Code, Data Matrix, Aztec, PDF417, EAN, UPC, Code 39, Code 93 e Code 128. Se `pyzbar` ou OpenCV estiverem instalados, eles também serão usados como backend de fallback.

#### Backend de upload de imagens (opcional)

O recurso de upload usa por padrão o script Python integrado `mark-shot-upload`, sem dependências adicionais (usa apenas a biblioteca padrão do Python 3). O script é configurado por meio de variáveis de ambiente e suporta qualquer serviço de hospedagem compatível com o protocolo de upload multipart/form-data.

<details>
<summary>Variáveis de ambiente suportadas pelo helper integrado</summary>

| Variável de ambiente | Descrição | Valor padrão |
|---------|------|--------|
| `MARK_SHOT_UPLOAD_URL` | **Obrigatória**, endpoint da API de upload | — |
| `MARK_SHOT_UPLOAD_FIELD` | Nome do campo do arquivo | `image` |
| `MARK_SHOT_UPLOAD_API_KEY` | API Key / Token | — |
| `MARK_SHOT_UPLOAD_AUTH_HEADER` | Nome do cabeçalho de autenticação | `Authorization` |
| `MARK_SHOT_UPLOAD_AUTH_SCHEME` | Esquema de autenticação (como `Bearer`); deixe vazio para usar a API Key diretamente | `Bearer` |
| `MARK_SHOT_UPLOAD_URL_PATH` | Caminho com pontos da URL na resposta JSON (como `data.url`) | Detecção automática |
| `MARK_SHOT_UPLOAD_DELETE_URL_PATH` | Caminho da URL de exclusão | Detecção automática |
| `MARK_SHOT_UPLOAD_HEADER_xxx` | Cabeçalhos de solicitação personalizados (como `MARK_SHOT_UPLOAD_HEADER_X-Custom=foo`) | — |
| `MARK_SHOT_UPLOAD_FIELD_xxx` | Campos de formulário adicionais (como `MARK_SHOT_UPLOAD_FIELD_album=123`) | — |

</details>

<details>
<summary>Exemplo de configuração: ImgURL V3</summary>

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

O ImgURL V3 usa autenticação `Authorization: Bearer <token>` (`AUTH_SCHEME` é `Bearer` por padrão, sem necessidade de alteração).

</details>

<details>
<summary>Exemplo de configuração: sm.ms</summary>

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

O sm.ms usa o Token diretamente como valor de Authorization; portanto, `AUTH_SCHEME` deve ficar como string vazia.

</details>

<details>
<summary>Exemplo de configuração: imgbb</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://api.imgbb.com/1/upload?key=你的API_KEY",
    "MARK_SHOT_UPLOAD_FIELD": "image",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

O imgbb transmite a API Key por meio de um parâmetro de consulta na URL; não é necessário definir `API_KEY`.

</details>

<details>
<summary>Exemplo de configuração: litterbox (hospedagem temporária, sem API Key)</summary>

```json
"upload": {
  "command": "curl -sf --max-time 30 -A 'Mozilla/5.0' -F reqtype=fileupload -F time=72h -F fileToUpload=@{image} https://litterbox.catbox.moe/resources/internals/api.php",
  "timeoutMs": 35000
}
```

A resposta do litterbox é uma URL em texto simples (não JSON); o DracoPho reconhece automaticamente a saída que começa com `http://`/`https://` como resultado do upload.

</details>

<details>
<summary>Comandos de upload personalizados</summary>

Se o helper integrado não atender às necessidades, qualquer script de upload personalizado pode ser integrado por meio de `upload.command`. O comando deve atender a:

1. **Código de saída**: 0 em caso de sucesso; diferente de zero é considerado falha
2. **Formato da saída** (uma das opções):
   - **JSON**: `{"url":"https://...","deleteUrl":"https://...","errors":[]}` (`url` é obrigatório; os demais são opcionais)
   - **URL em texto simples**: a primeira linha não vazia do stdout começa com `http://` ou `https://`
3. **Espaços reservados**: suporta `{image}`, `{imagePath}` e `{imageUrl}`; se o comando não contiver nenhum espaço reservado, o DracoPho anexa automaticamente o caminho temporário da imagem ao final do comando

```json
"upload": {
  "command": "/path/to/your-uploader.sh --file {image} --json",
  "timeoutMs": 30000,
  "env": {
    "UPLOADER_API_KEY": "xxx"
  }
}
```

As variáveis de ambiente em `upload.env` também são transmitidas ao comando personalizado, facilitando a reutilização da configuração.

</details>

#### Windows

Instale o Qt 6, o CMake e o Ninja compatíveis com o seu compilador atual, além de um compilador com suporte a C++17, como MSVC ou MinGW. A compilação no Windows não requer Qt DBus, PipeWire, X11/XCB, LayerShellQt, `grim`, `wl-copy` ou `xclip`.

```powershell
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
cmake --build build-windows
```

O escopo atual de suporte no Windows é a captura de tela comum e a anotação de imagens. A captura de rolagem, a detecção de janelas específica do compositor e os atalhos de área de trabalho do Linux não estão disponíveis no Windows. Os scripts auxiliares Python integrados (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`) não são instalados automaticamente; consulte as seções [Backend de OCR](#ocr-后端可选), [Backend de leitura de código](#扫码后端可选) e a seção de tradução acima para a configuração manual.

### Compilação e build

```bash
# 使用系统 Qt 6
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 如果 Qt 6 安装在用户目录，额外指定 CMAKE_PREFIX_PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64

# 执行编译
cmake --build build
```

Ou, alternativamente, use o nix

```bash
nix build
```

O LayerShellQt é detectado automaticamente. Quando encontrado, o suporte completo a layer-shell do Wayland é ativado; quando não encontrado, a compilação ainda é bem-sucedida e o aplicativo faz downgrade automaticamente para uma janela padrão em tela cheia em tempo de execução.

### Instalação e integração

```bash
cmake --install build --prefix "$HOME/.local"
```

Esse comando instala o executável, os scripts auxiliares (`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`, `mark-shot-upload`), os atalhos da área de trabalho e os ícones.

### Extensão de captura de rolagem para GNOME Wayland

A captura de rolagem no GNOME Wayland exige que a extensão **DracoPho Scroll Helper** esteja habilitada. Sem essa extensão, o DracoPho não consegue capturar silenciosamente e de forma contínua a área selecionada nem desenhar o painel de visualização de rolagem nativo do GNOME; portanto, o botão de captura de rolagem é desativado no GNOME Wayland.

Os arquivos da extensão estão localizados no caminho `packaging/gnome-extension/mark-shot-scroll-helper@snemc.org` do repositório do projeto.

<details>
<summary><b>Expandir/recolher o guia de instalação e ativação da extensão de captura de rolagem do GNOME Wayland</b></summary>

##### Método A: instalar pelo pacote da distribuição
Se você instalou o DracoPho por meio de um pacote da distribuição (como `.deb` ou `.rpm`), a extensão já vem instalada por padrão. Execute o seguinte comando para ativá-la para o usuário atual:
```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```
*Se a extensão não for encontrada, faça logout e login novamente no sistema e tente de novo.*

##### Método B: instalar pelo diretório de código-fonte do repositório
Se você compilou a partir do código-fonte ou manualmente em sua máquina, primeiro copie a extensão para o caminho de extensões do GNOME do usuário:
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

Verifique se a interface D-Bus do helper está disponível:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
```

O resultado esperado é `('4.2',)`. Depois de ativar a extensão, reinicie o `mark-shot`.

</details>

---

## Guia de atalhos e gestos de interação

### Atalhos de alternância de ferramentas

| Atalho | Ferramenta | Descrição da função |
| :---: | :--- | :--- |
| **V** | Mover / Navegar (Move / Pan) | No modo de imagem existente, usado para mover e arrastar a tela de desenho. |
| **S** | Selecionar (Select) | Seleciona e move, redimensiona ou exclui as anotações vetoriais desenhadas. |
| **P** | Pincel (Pen) | Desenho de curvas livres. |
| **L** | Linha (Line) | Desenha linhas vetoriais retas. |
| **H** | Marcador (Highlighter) | Sobreposição de realce semitransparente, ideal para marcar pontos importantes. |
| **R** | Retângulo (Rectangle) | Desenha contornos de retângulo. |
| **E** | Elipse (Ellipse) | Desenha contornos de elipse. |
| **A** | Seta (Arrow) | Desenha a clássica seta longa e afilada de seis vértices com ângulo agudo. |
| **T** | Texto (Text) | Insere e organiza texto rico, com suporte a tamanho de fonte de 1000px e vínculo por arrastar. |
| **N** | Número (Number) | Marcadores de número de etapa com incremento automático. |
| **M** | Mosaico (Mosaic) | Aplica desfoque estilo vidro fosco em áreas sensíveis. |
| **G** | Caneta laser (Laser) | Traços temporários para aulas ou apresentações, que se dissolvem suavemente. |

### Ferramentas auxiliares da tela inicial

| Atalho | Ferramenta | Descrição da função |
| :---: | :--- | :--- |
| **C** | Conta-gotas (Color Picker) | Amostra os pixels da captura antes de selecionar a área. Role a roda do mouse para ajustar o tamanho da lupa; um clique com o botão esquerdo abre o painel de cores, com cópia nos formatos HEX, RGB, HSL, HSV, Qt etc. Clique com o botão direito ou pressione Esc para voltar à seleção normal. |
| **R** | Régua (Ruler) | Mede coordenadas antes de selecionar a área de captura. Ao passar o mouse, exibe o pixel atual; arraste com o botão esquerdo para desenhar um retângulo de medição com escala em pixels, mostrando largura, altura, diagonal e área. Clique com o botão direito ou pressione Esc para voltar à seleção normal. |
| **Q** | Leitor de código (Code Scanner) | Entra no modo de leitura de QR Codes e códigos de barras. Após selecionar a área, o conteúdo do código é reconhecido e exibido em uma janela com possibilidade de cópia. Clique com o botão direito ou pressione Esc para voltar à seleção normal. |
| **D** | Captura de monitor (Display Capture) | Captura imediatamente todas as telas de saída, recorta por monitor e mostra miniaturas; passe o mouse sobre uma miniatura para copiar, editar ou salvar. |

### Atalhos de operações globais

| Atalho | Ação |
| :---: | :--- |
| **Esc** | Sai imediatamente e fecha a janela de anotação. |
| **Ctrl + C** | Confirma todas as edições de texto e copia a captura atual/área anotada para a área de transferência do sistema. |
| **Ctrl + S** ou **Enter / Return** | Confirma todas as edições de texto e salva a captura atual. |
| **Ctrl + P** | Fixa a seleção atual como uma janela flutuante de fixação. |
| **Ctrl + U** | Envia a captura atual para a hospedagem personalizada; após o envio, a URL é copiada automaticamente para a área de transferência. |
| **Ctrl + Z** | Desfaz a última operação de anotação. |
| **Ctrl + Y** ou **Ctrl + Shift + Z** | Refaz as operações de anotação que foram desfeitas. |
| **Backspace** ou **Delete** | Quando a ferramenta **Selecionar (Select)** está ativa e uma anotação está selecionada, exclui a anotação selecionada. |
| **F** | Alterna a cobertura da captura atual (alterna entre o modo de seleção e o modo tela cheia). |

### Técnicas avançadas de interação

- **Restrição de formas**: ao desenhar um **Retângulo (Rectangle)** ou uma **Elipse (Ellipse)**, mantenha pressionada a tecla `Ctrl` para forçar um quadrado ou um círculo perfeito.
- **Alternância rápida para a ferramenta de seleção**: durante a anotação, um clique com o botão direito em uma área vazia da tela alterna imediatamente para a ferramenta **Selecionar (Select)**.
- **Alternância rápida de cor com clique duplo no botão direito**: um clique duplo com o botão direito em uma área vazia da tela abre a paleta de cores em anel, permitindo alternar rapidamente a cor da ferramenta de anotação atual.
- **Ajuste contínuo pela roda do mouse**: com a ferramenta de anotação correspondente ativa, rolar a roda do mouse ajusta em tempo real a espessura da linha, o tamanho da fonte, o tamanho dos marcadores de número ou o tamanho da grade do mosaico.
- **Panorâmica e zoom da tela**: no modo da ferramenta **Selecionar (Select)** ou ao editar arquivos locais, rolar a roda do mouse aplica um zoom contínuo na tela, e arrastar com o botão do meio pressionado move a tela. Um clique duplo na tecla `Ctrl` redefine o zoom e a posição.

### Interações exclusivas da janela de fixação

| Gestos / atalhos | Efeito |
| :--- | :--- |
| **Botão esquerdo do mouse pressionado e arrastar** | Move e posiciona livremente a imagem fixada na área de trabalho. |
| **Roda do mouse para cima/baixo** | Amplia/reduz a janela de fixação de forma contínua e proporcional. |
| **Clique duplo com o botão esquerdo** | Fecha rapidamente a janela de fixação. |
| **Clique com o botão direito** | Abre o menu de funções (incluindo rotação, copiar texto da imagem, traduzir, salvar, copiar, fechar etc.). |
| **Tecla Esc** | Fecha a janela de fixação que está com o foco. |

---

## Notas de versão

Consulte as [notas de versão](../docs/releases.zh-CN.md).

## Feedback e contato

### Como enviar uma Issue
Se você encontrar problemas durante o uso ou tiver sugestões de novos recursos, recomendamos usar a ferramenta de linha de comando GitHub CLI (`gh`) para enviar issues. Fornecemos um script que coleta as informações do ambiente com um clique e gera a issue automaticamente; consulte o [guia de envio de issues](../.doc/submit-issue-via-gh.md) para obter detalhes.

---

## Licença

Este projeto é open source sob a **licença MIT**; consulte o arquivo [LICENSE](../LICENSE) para obter detalhes.

## Agradecimentos

O DracoPho se apoia nos ombros da comunidade de código aberto, e aqui expressamos nosso sincero agradecimento:

- **O projeto original a montante [jswysnemc/mark-shot](https://github.com/jswysnemc/mark-shot), seus autores e todos os colaboradores.** Esta edição da comunidade é desenvolvida com base no projeto original a montante; seu design excepcional e suas contribuições contínuas são a base de tudo isso, e agradecemos sinceramente pelo excelente trabalho.
- **[serendipitywgy](https://github.com/serendipitywgy)**: agradecemos pelas contribuições por meio de `serendipitywgy/mark-shot`, incluindo melhorias de compatibilidade entre desktops, a ação de cópia de OCR na barra de ferramentas e o recurso de pré-seleção inteligente de retângulos.
- **Todos os projetos de código aberto dos quais o DracoPho depende**, incluindo Qt 6, PipeWire, xdg-desktop-portal, layer-shell-qt, wl-clipboard, xclip, grim, RapidOCR, onnxruntime, Tesseract, ZXing-C++ e outros.

Esta edição da comunidade é mantida pela [Beijing Taiyin Zhaowu Technology Co., Ltd.](https://github.com/tystudio-26020701/mark-shot-community) e por colaboradores, e é open source sob a **licença MIT**.
