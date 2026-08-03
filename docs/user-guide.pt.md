# Guia do usuário do Mark Shot

Este manual cobre a operação diária do Mark Shot, com foco no recurso de
**seleção por hover de janela / componente** (mover o mouse automaticamente
rastreia e destaca a janela sob o cursor; um clique a seleciona), no fluxo de
anotação, na captura headless e na configuração.

> Os documentos neste repositório são redigidos no fork da comunidade e
> espelhados nos repositórios upstream e enterprise. A edição enterprise
> adiciona uma seção extra para seu servidor MCP local.

---

## 1. Início rápido

### 1.1 Inicialização

Inicie uma sessão de captura de região:

```bash
mark-shot
```

Pressione um atalho do desktop (veja § 8) ou execute a partir de um terminal.
Uma sobreposição congelada em tela cheia abre no display focado. Mova o mouse
para desenhar um retângulo de seleção e, em seguida, solte para entrar no editor
de anotação.

### 1.2 Compilações portáteis

Se você usar um pacote portátil (`mark-shot-upstream`, `mark-shot-community`,
`mark-shot-enterprise`), inicie-o com o launcher incluído para que as
bibliotecas Qt, plug-ins e scripts auxiliares incluídos sejam encontrados:

```bash
portable/mark-shot-community/bin/run-mark-shot.sh
```

O launcher antepõe seu diretório `bin/` ao `PATH`, o que é necessário para os
scripts auxiliares de detecção de janelas (`mark-shot-window-detection-*`) e
para os auxiliares de OCR / upload.

---

## 2. Seleção por Hover de Janela / Componente

O Mark Shot pode detectar as janelas do desktop atual antes de você escolher
uma região. Enquanto a sobreposição de seleção está aberta, **mover o mouse
destaca a janela sob o cursor** com um contorno ciano. **Um clique esquerdo
simples (sem arrastar) seleciona a janela inteira** como região de captura;
você pode então anotar, copiar, fixar ou salvá-la diretamente.

As janelas destacadas vêm de um script de detecção por compositor que é
executado antes de a sobreposição aparecer:

| Desktop | Fonte de detecção | Observações |
| :--- | :--- | :--- |
| GNOME Wayland | extensão Shell `mark-shot-scroll-helper@snemc.org` incluída, via D-Bus | requer que a extensão esteja habilitada (veja § 2.1) |
| KDE Plasma Wayland | script KWin de uma única execução via `qdbus6` / `qdbus` + journalctl | requer uma sessão KWin |
| Hyprland | `hyprctl -j clients` | |
| niri | `niri msg -j windows` + análise de configuração | |
| X11 | enumeração XCB em processo de `_NET_CLIENT_LIST_STACKING` | nenhum script necessário |
| Windows | `EnumWindows` em processo | nenhum script necessário |

Apenas **janelas de nível superior** são rastreadas. Widgets individuais dentro
de uma janela ("componentes") não são expostos pelos compositores Wayland,
portanto, a seleção por hover tem como alvo janelas inteiras em todas as
plataformas.

### 2.1 GNOME Wayland: habilite a extensão auxiliar

```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```

Verifique se o auxiliar D-Bus responde:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
# -> ('5',)
```

Se a chamada falhar, faça logout e login novamente (ou reinicie o GNOME Shell
no X11) e tente novamente. Sem a extensão, o script auxiliar do GNOME sai com
um erro e a seleção por hover permanece desativada (a seleção normal por
arrastar continua funcionando).

### 2.2 Como usar

1. Dispare uma captura (`mark-shot` ou o atalho do desktop).
2. Sem pressionar nenhum botão do mouse, mova o cursor sobre uma janela. Um
   contorno ciano delineia a janela que seria selecionada.
3. **Clique uma vez** (pressione e solte sem mover mais de alguns pixels) para
   selecionar essa janela. Se as janelas se sobrepõem, a janela mais superior
   sob o cursor vence (ciente da ordem z).
4. Soltar entra no editor de anotação com a janela exatamente enquadrada.
5. Para fazer uma região **manual**, basta arrastar um retângulo como de
   costume — o quadro de hover é ignorado assim que o arrasto excede o limite
   de clique.

O destaque de hover é desativado enquanto a ferramenta de inicialização
Seletor de Cores (`C`) ou Régua (`R`) está ativa e permanece disponível para
o Scanner de Código (`Q`), captura de display (`D`) e modos de inicialização
de gravação de GIF / vídeo.

### 2.3 Selecionando janelas no monitor correto

A detecção de janelas é executada por alvo de captura. Em uma configuração com
vários monitores, cada janela congelada recebe apenas as janelas que
interseccionam sua própria geometria, então o quadro de hover corresponde ao
que você vê naquele display.

### 2.4 Habilitando / desabilitando

O recurso é habilitado por padrão (`windowDetection.enabled = true`). Alterne-o
em **Configurações → Avançado → Detecção de Janela Habilitada**, ou edite
`~/.config/mark-shot/config.json`:

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

- `command`: o script de detecção. No Wayland do GNOME / KDE / Hyprland / niri,
  o script `mark-shot-window-detection-*` incluído que corresponde à sua sessão
  é escolhido automaticamente; no X11 e no Windows, a plataforma é enumerada em
  processo e `command` pode ficar vazio. **Um comando personalizado fornecido
  pelo usuário (por exemplo, um caminho absoluto) é sempre respeitado.**
- `timeoutMs`: espera máxima pelo script (100–30000 ms, padrão 1000).
- `env`: variáveis de ambiente extras passadas ao script. Ajustes
  específicos por compositor (offsets) são documentados nos cabeçalhos dos
  scripts.

### 2.5 Solução de problemas

| Sintoma | Verificação |
| :--- | :--- |
| Sem contorno ciano no GNOME Wayland | extensão habilitada? a chamada `gdbus` acima deve retornar uma versão |
| Sem contorno ciano no X11 / Windows | nenhuma — a enumeração da plataforma é integrada; certifique-se de que a sessão de captura não esteja usando uma ferramenta de ponteiro de inicialização |
| O quadro de hover escolhe a janela errada (por baixo) | dados de ordem z ausentes de um script de detecção personalizado; janelas sem `zOrder` são classificadas como a camada inferior |
| A captura começa lentamente | o script de detecção é executado antes da sobreposição; aumente `timeoutMs` apenas se o desktop for lento, ou defina `enabled:false` para ignorá-lo |
| Ver diagnósticos | execute `mark-shot --debug --debug-log /tmp/mark-shot.log`; procure linhas `window-detection` |

---

## 3. Seleção de Região e Ferramentas de Inicialização

Antes de a região ser confirmada, você pode usar as ferramentas da sobreposição
de inicialização:

| Atalho | Ferramenta | Comportamento |
| :---: | :--- | :--- |
| `C` | Seletor de Cores | Amostra um pixel; a roda redimensiona a lupa; o clique esquerdo abre um painel de cores (formatos HEX / RGB / HSL / HSV / Qt); o clique direito ou `Esc` sai |
| `R` | Régua | O hover lê as coordenadas dos pixels; o arrasto esquerdo mede um retângulo com largura, altura, diagonal e área; o clique direito ou `Esc` sai |
| `Q` | Scanner de Código | Arraste uma região ao redor de um QR / código de barras; o resultado decodificado abre em uma janela copiável |
| `D` | Captura de Display | Captura todas as saídas, recorta por display, mostra miniaturas com hover (copiar / editar / salvar) |
| `S` | Para gravação ativa de GIF / vídeo | interrompe a gravação mostrada na sobreposição |

`Esc` cancela a sessão; o clique direito (sem ferramenta de inicialização)
também cancela.

#### 3.1 Comportamento de congelamento em múltiplos monitores

Com o escopo de captura padrão **Freeze All Screens**, cada tela conectada é
congelada enquanto uma região é selecionada. Depois que você confirma uma seleção
em um monitor, os outros displays continuam mostrando seu quadro congelado como
um pano de fundo não interativo: entradas do mouse, teclado, roda e atalhos são
engolidas e as sobreposições não mostram barras de ferramentas, então o resto da
área de trabalho virtual permanece congelado até o fim da sessão de captura. Se,
em vez disso, você usar o escopo **Cursor Screen** (Settings → Capture → Freeze
Scope), apenas o monitor sob o cursor é congelado e as outras telas permanecem
totalmente utilizáveis.

---

## 4. Ferramentas de Anotação

Após uma região ser selecionada (ou uma imagem local ser aberta), o editor abre
com a barra de ferramentas de anotação. As ferramentas são alternadas com as
teclas numéricas ou a barra de ferramentas:

| Atalho | Ferramenta | Descrição |
| :---: | :--- | :--- |
| `V` | Mover / Panorama | move toda a seleção, aplica panorama ao canvas de uma imagem local |
| `S` | Selecionar | seleciona, move, redimensiona, rotaciona, exclui anotações existentes |
| `P` | Caneta | traços suaves à mão livre |
| `L` | Linha | linhas retas |
| `H` | Marcador | marcador semitransparente; estilo de mão livre ou linha reta |
| `R` | Retângulo | caixa com estilos `Stroke` / `Highlight` / `Invert`, cantos arredondados |
| `E` | Elipse | elipse / círculo |
| `A` | Seta | setas clássicas (com penas, KDE, bidirecional) |
| `T` | Texto | texto rico; a roda ou os controles deslizantes redimensionam; as alças diagonais escalam ambos, as alças laterais ajustam a quebra de linha; tamanho exato em pt, família de fonte, negrito / itálico no painel de fontes |
| `N` | Número | marcadores numerados sequenciais (arábico, alfa, romano, chinês, …) |
| `M` | Mosaico | desfoque fosco acrílico para ocultar conteúdo sensível |
| `G` | Laser | traços temporários que se dissolvem automaticamente |

Dicas de desenho:

- Segure `Ctrl` ao desenhar um retângulo / elipse para restringir a um quadrado
  / círculo.
- Role a roda enquanto uma ferramenta está ativa para ajustar a largura do
  traço, o tamanho do texto, a escala dos números ou o tamanho do bloco de
  mosaico (pré-visualização ao vivo).
- Em `Select`, role para aplicar zoom ao canvas e segure o botão do meio para
  aplicar panorama; toque duas vezes em `Ctrl` para redefinir.

### 4.1 Editando uma anotação existente

Alterne para **Selecionar** (`S`). Clique em uma anotação para mostrar suas
alças:

- arraste para dentro para mover;
- arraste as alças de canto / borda para redimensionar;
- arraste a alça redonda acima da borda superior para rotacionar;
- pressione `Delete` / `Backspace` para remover;
- dê dois cliques no texto para editá-lo no local.

O painel de propriedades (lado direito) edita a anotação selecionada: cor,
largura, estilo, família / tamanho / negrito / itálico da fonte do texto.
Várias anotações podem ser selecionadas arrastando uma caixa de seleção sob a
ferramenta `Select`; o grupo pode então ser movido, redimensionado, rotacionado
e excluído em conjunto.

### 4.2 Ações

| Atalho | Ação |
| :--- | :--- |
| `Ctrl+C` | copiar para a área de transferência |
| `Ctrl+S` / `Enter` | salvar (modelo de caminho das configurações) |
| `Ctrl+P` | fixar como uma janela de adesivo flutuante |
| `Ctrl+U` | enviar para o host de imagens configurado; a URL é copiada |
| `Ctrl+Z` / `Ctrl+Y` | desfazer / refazer |
| `F` | alternar o escopo da captura (seleção ↔ tela cheia) |

### 4.3 Moldura de exportação

Habilite **Configurações → Exportar → Moldura no estilo Mac** para adicionar
preenchimento transparente, cantos arredondados e uma sombra suave às imagens
salvas / copiadas / enviadas.

---

## 5. Janelas de Adesivos Fixadas

| Gesto / Atalho | Comportamento |
| :--- | :--- |
| arrasto esquerdo | reposicionar o adesivo |
| roda | escalar proporcionalmente |
| duplo clique esquerdo / `Esc` | fechar |
| clique direito | menu de contexto (rotacionar, zoom, sempre no topo, copiar texto, traduzir, salvar, copiar, fechar) |

O texto OCR dentro de uma janela fixada é selecionável e copiável (`Ctrl+C` /
menu de contexto). A tradução (endpoint compatível com OpenAI) renderiza o
texto traduzido de volta na imagem nas posições originais do layout.

---

## 6. Captura de Tela com Rolagem

1. Selecione uma região (ou use a alça flutuante de arrasto para regiões muito
   grandes).
2. A sobreposição rola a janela de destino; os quadros capturados são
   costurados em uma imagem longa.
3. O GNOME Wayland requer a extensão Mark Shot Scroll Helper (§ 2.1).

A captura com rolagem está pronta para produção no niri e em compositores
wlroots/Wayland semelhantes; no KDE, X11 e outras pilhas, é um recurso de
teste. Se falhar, use capturas de tela normais ou um comando personalizado de
extensão.

---

## 7. Captura Headless (CLI)

A captura não interativa grava um PNG e imprime JSON:

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

Todas as opções headless são mutuamente exclusivas com um arquivo de imagem
posicional. Veja o README para a tabela completa de argumentos.

### 7.1 Captura headless de janela / componente

O Mark Shot pode capturar **janelas específicas — ou um componente (sub-região)
dentro de uma janela — sem abrir nenhuma interface**, a partir de um script, de
um pipeline de build ou de um agente. O processo sai assim que as imagens são
gravadas ou retornadas, e nunca cria uma janela, nunca abre um diálogo e nunca
rouba o foco, para que o usuário possa continuar trabalhando enquanto uma
ferramenta captura o desktop.

Primeiro liste as janelas para ver o que está disponível:

```bash
mark-shot --list-windows
```

Exemplo de saída (GNOME Wayland):

```json
{"count":2,"platform":"wayland","source":"compositor-script","windows":[
  {"index":0,"id":"0x3c00007","title":"Mark Shot - VSCodium","class":"codium","instance":"codium","x":1920,"y":0,"width":1680,"height":1050,"zOrder":1},
  {"index":1,"title":"Terminal","class":"org.gnome.Terminal","x":67,"y":32,"width":800,"height":600}
]}
```

Cada entrada carrega os campos contra os quais os seletores correspondem:
`index`, `id` (id de janela X11 / id fornecido pelo backend), `title`, `class`
e `instance`, além de `x`/`y`/`width`/`height` e um `zOrder` opcional.

#### 7.1.1 Selecionando janelas (únicas ou múltiplas)

`--window` pode ser repetido para capturar **qualquer número de janelas em uma
única chamada**. Cada seletor é interpretado automaticamente
(`--window-by auto`):

| Valor do seletor           | Corresponde a                                   |
| :---                      | :---                                                |
| `0`, `1`, …               | `index` da lista                                   |
| `0x3c00007`               | `id` da janela                                      |
| `VSCodium`                | `class` ou `instance`, depois `title` (exato, depois substring) |
| `Mark Shot - VSCodium`    | `title`                                             |

Force uma regra de correspondência com `--window-by id|title|class|index`. Um
seletor que corresponde a várias janelas captura **todas elas**.

Capture um componente (uma sub-região dentro de uma janela) anexando
`@x,y,width,height` ao seletor — o deslocamento é relativo ao canto superior
esquerdo da janela e é limitado aos limites da janela:

```bash
# the top 100px strip of window 0
mark-shot --window "0@0,0,1680,100" --capture-destination file --capture-to /tmp/shots/
```

#### 7.1.2 Escolhendo para onde as imagens vão

`--capture-destination` decide a saída; pode ser combinado com qualquer número
de seletores `--window` e uma sub-região de componente:

| Destino | Comportamento |
| :--- | :--- |
| `inline` (padrão) | PNGs em Base64 incorporados na saída JSON. **Nenhum arquivo é gravado e a área de transferência nunca é tocada.** A escolha mais segura para agentes que desejam apenas os pixels. |
| `file` | arquivos PNG gravados em `--capture-to <directory>`; requer essa opção. |
| `stage` | arquivos PNG gravados em um diretório de staging temporário (`$TMPDIR/mark-shot-staging`). Bom para um fluxo de trabalho de "guardar para depois". |
| `clipboard` | imagens copiadas para a área de transferência do sistema; com várias imagens, a **última vence**. O conteúdo sobrevive à saída da CLI (um proprietário persistente de `wl-copy` / `xclip` é gerado). |

Exemplos:

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

**Política da área de transferência.** O editor interativo coloca
deliberadamente sua seleção na área de transferência do sistema (a ação
`Copy` / `Ctrl+C`), porque esse é o fluxo de trabalho principal de uma
ferramenta de captura de tela. Os modos headless (a CLI e o servidor MCP
enterprise) seguem a regra oposta: **a área de transferência nunca é
modificada a menos que `clipboard` seja explicitamente escolhido como destino
E as gravações na área de transferência estejam habilitadas em Configurações >
Armazenamento > Modo Headless** — `inline` (padrão) e `stage` deixam o
conteúdo atual da área de transferência do usuário intocado, para que uma
captura agendada ou conduzida por agente não possa sobrescrever texto ou
imagens com os quais o usuário está trabalhando em outro lugar. Quando uma
solicitação `clipboard` é rejeitada porque as gravações headless na área de
transferência estão desabilitadas, a captura volta para o destino padrão
headless configurado, a saída JSON (`"warning"`) e o stderr avisam você, e o
processo sai com um código diferente de zero para que a automação possa
detectar. Habilitar gravações headless na área de transferência nas
configurações requer digitar uma senha de confirmação.

A saída é um objeto JSON `{"captures":[...]}` com uma entrada por janela
capturada; cada entrada repete o seletor, a identidade da janela e o retângulo
final de captura, além de um `path` (file/stage) ou `data` (inline) ou nenhum
(clipboard). O código de saída é `0` apenas quando cada seletor correspondeu e
cada captura foi bem-sucedida; uma correspondência ausente ou uma captura
falha produz o código de saída `1` com um campo `"error"` em vez de um sucesso
silencioso.

O mesmo pipeline de captura pode produzir saída anotada programaticamente —
veja o capítulo do servidor MCP da edição enterprise, ou combine o PNG salvo
com o editor interativo.

#### 7.1.3 Garantia de não interferência em janelas

Todo modo headless é garantido como invisível e não perturbador:

- **nenhuma janela é nunca criada** — incluindo o editor de anotação, a
  sobreposição de captura e a bandeja; a captura reutiliza o caminho de captura
  headless;
- **nenhum diálogo é nunca mostrado** — incluindo diálogos de erro: erros vão
  para o stderr; até mesmo linhas de comando malformadas (por exemplo,
  `--window-by` sem `--window`, um `--capture-destination` desconhecido ou
  arquivos posicionais extras) saem imediatamente com um código diferente de
  zero e uma mensagem no stderr em vez de abrir um `QMessageBox` ou cair na
  interface interativa;
- nenhum prompt de portal interativo aparece (`allowInteractivePortal` está
  desabilitado);
- o processo sai imediatamente após gravar a saída;
- a lista de janelas capturada antes e depois de uma operação headless é
  idêntica;
- os modos headless nunca tocam a área de transferência do sistema, a menos que
  `clipboard` tenha sido explicitamente solicitado **e** as gravações na área
  de transferência estejam habilitadas em Configurações > Armazenamento > Modo
  Headless.

Se nenhuma janela for detectada (por exemplo, um auxiliar de compositor que
está desabilitado ou uma sessão X11 sem enumeração de janelas), o comando
imprime um erro claro no stderr e sai com o código `1` em vez de capturar nada
silenciosamente.

---

## 8. Atalhos do Desktop e Bandeja

O modo de bandeja (`mark-shot --tray`) registra `Ctrl+Alt+S` para captura de
região e fornece entradas de menu de captura / gravação / configurações /
sair. Atalhos do desktop:

- **GNOME**: Configurações → Teclado → Atalhos → Atalhos Personalizados →
  vincule a `mark-shot`.
- **KDE**: atalho personalizado vinculado a `mark-shot` (mais a permissão KWin
  ScreenShot2 para captura exata no KDE, veja o README).
- **Hyprland**: `bind = SUPER SHIFT, S, exec, mark-shot` e `bind = , Print, exec, mark-shot`.
- **niri**: `binds { Mod+Shift+S { spawn "mark-shot"; } }`.
- **Sway / i3**: `bindsym Mod4+Shift+S exec mark-shot`.

---

## 9. Configuração e Backends

- Arquivo de configuração: `~/.config/mark-shot/config.json` (Linux), criado na
  primeira execução.
- Referência completa: [Configuration](configuration.md).
- Backends: Wayland (portal PipeWire / grim / screencopy wlroots), X11
  (`QScreen::grabWindow`), Windows (WGC nativo). A gravação prefere o portal
  PipeWire e faz fallback automaticamente.
- A janela de Configurações rastreia alterações não salvas de forma
  determinística: cada controle (menu suspenso, interruptor, caixa de rotação,
  campo de texto, campo de atalho, seletor de cores) atualiza imediatamente o
  indicador de alterações não salvas, incluindo valores escolhidos em pop-ups de
  caixa de combinação e no diálogo modal de cores. Reverter uma alteração limpa
  o indicador, então a janela só pergunta sobre alterações pendentes reais ao
  fechar.

Auxiliares opcionais:

```bash
# OCR (RapidOCR / Tesseract)
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime

# Code scan (zxing-cpp)
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

---

## 10. Lista de Verificação de Teste de Recursos

Use isto para verificar um build de ponta a ponta:

1. **Inicialização** — `run-mark-shot.sh` abre a sobreposição congelada.
2. **Hover de janela** — mova o mouse sobre uma janela: o contorno ciano
   segue; um único clique seleciona a janela; janelas sobrepostas escolhem a
   mais superior.
3. **Região manual** — arraste um retângulo; solte; o editor abre.
4. **Anotar** — desenhe com cada ferramenta (Caneta, Linha, Retângulo, Elipse,
   Seta, Marcador, Texto, Número, Mosaico, Lupa, Laser); desfazer/refazer;
   Selecionar para mover/redimensionar/rotacionar/excluir; dê dois cliques em
   um texto para editar.
5. **Copiar / Salvar / Fixar / Enviar** — `Ctrl+C`, `Ctrl+S`, `Ctrl+P`,
   `Ctrl+U`.
6. **Ferramentas de inicialização** — `C` seletor de cores, `R` régua, `Q`
   scan de código, `D` captura de display.
7. **Headless** — `--capture-to`, `--region`, `--display`, `--list-displays`.
8. **Captura de janela headless** — `--list-windows` lista o desktop; repita
   `--window` para capturar várias janelas; teste `--capture-destination` em
   todos os quatro modos (inline, file, stage, clipboard); verifique um seletor
   de componente (`--window "0@0,0,400,300"`); confirme que a lista de janelas
   antes e depois é inalterada (não interferência em janelas).
9. **Bandeja + atalho** — `mark-shot --tray`, pressione `Ctrl+Alt+S`.
10. **Especificidades do portátil** — o pacote encontra suas próprias
    libs/plugins/scripts Qt.

---

## 11. Feedback

Relate problemas com `gh issue create` usando o
[guia de envio de issues](../.doc/submit-issue-via-gh.md) incluído. Anexe um
log de depuração capturado com `mark-shot --debug --debug-log /tmp/mark-shot.log`.
