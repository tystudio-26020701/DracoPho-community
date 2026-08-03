# Mark Shot 사용자 가이드

이 매뉴얼은 Mark Shot의 일상적인 사용법을 다룹니다. 특히 **창 / 구성 요소 호버 선택** 기능(마우스를 움직이면 커서 아래에 있는 창을 자동으로 추적·강조 표시하고, 클릭하면 선택하는 기능), 주석(annotation) 작업 흐름, 헤드리스(headless) 캡처, 그리고 설정에 초점을 맞춥니다.

> 이 저장소의 문서는 커뮤니티 포크에서 작성되어 업스트림 및 엔터프라이즈 저장소로 미러링됩니다. 엔터프라이즈 에디션에는 로컬 MCP 서버에 대한 별도의 섹션이 추가됩니다.

---

## 1. 빠른 시작

### 1.1 실행

영역 캡처 세션 시작:

```bash
mark-shot
```

데스크톱 단축키(§ 8 참조)를 누르거나 터미널에서 실행하세요. 활성화된 디스플레이에 정지된 전체 화면 오버레이가 열립니다. 마우스를 움직여 선택 사각형을 그리고, 놓으면 주석 편집기로 들어갑니다.

### 1.2 휴대용 빌드

휴대용 번들(`mark-shot-upstream`, `mark-shot-community`, `mark-shot-enterprise`)을 사용한다면, 번들에 포함된 Qt 라이브러리, 플러그인, 헬퍼 스크립트를 찾을 수 있도록 번들에 포함된 런처로 실행하세요:

```bash
portable/mark-shot-community/bin/run-mark-shot.sh
```

런처는 자신의 `bin/` 디렉터리를 `PATH` 앞에 추가합니다. 이는 창 감지 헬퍼 스크립트(`mark-shot-window-detection-*`)와 OCR / 업로드 헬퍼에 필요합니다.

---

## 2. 창 / 구성 요소 호버 선택

Mark Shot은 영역을 선택하기 전에 현재 데스크톱의 창들을 감지할 수 있습니다. 선택 오버레이가 열려 있는 동안 **마우스를 움직이면 커서 아래의 창이 청록색(teal) 테두리로 강조 표시됩니다.** **드래그 없이 단순 왼쪽 클릭을 하면 해당 창 전체가 캡처 영역으로 선택**됩니다. 이후 바로 주석을 달거나, 복사, 핀, 저장할 수 있습니다.

강조 표시되는 창들은 오버레이가 나타나기 전에 실행되는 컴포지터별 감지 스크립트에서 가져옵니다:

| 데스크톱 | 감지 소스 | 참고 사항 |
| :--- | :--- | :--- |
| GNOME Wayland | 번들 `mark-shot-scroll-helper@snemc.org` Shell 확장(Over D-Bus) | 확장이 활성화되어 있어야 합니다(§ 2.1 참조) |
| KDE Plasma Wayland | `qdbus6` / `qdbus` + journalctl을 통한 1회성 KWin 스크립팅 | KWin 세션이 필요합니다 |
| Hyprland | `hyprctl -j clients` | |
| niri | `niri msg -j windows` + 설정 파싱 | |
| X11 | 프로세스 내 XCB `_NET_CLIENT_LIST_STACKING` 열거 | 스크립트 불필요 |
| Windows | 프로세스 내 `EnumWindows` | 스크립트 불필요 |

**최상위 창(top-level windows)** 만 추적됩니다. 창 안의 개별 위젯("구성 요소")은 Wayland 컴포지터가 노출하지 않으므로, 호버 선택은 모든 플랫폼에서 창 전체를 대상으로 합니다.

### 2.1 GNOME Wayland: 헬퍼 확장 활성화

```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```

D-Bus 헬퍼가 응답하는지 확인하세요:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
# -> ('5',)
```

호출이 실패하면 로그아웃 후 다시 로그인하고(X11에서는 GNOME Shell 재시작) 다시 시도하세요. 확장이 없으면 GNOME 헬퍼 스크립트는 오류와 함께 종료되고 호버 선택은 꺼진 상태로 유지됩니다(일반적인 드래그 선택은 여전히 작동합니다).

### 2.2 사용 방법

1. 캡처를 트리거합니다(`mark-shot` 또는 데스크톱 단축키).
2. 어떤 마우스 버튼도 누르지 않은 채 커서를 창 위로 움직입니다. 선택될 창이 청록색 테두리로 윤곽이 그려집니다.
3. **한 번 클릭**(몇 픽셀 이상 움직이지 않고 누르고 놓기)하여 해당 창을 선택합니다. 창이 겹쳐 있으면 커서 위치에서 가장 위에 있는 창이 선택됩니다(z-order 인식).
4. 놓으면 해당 창이 정확히 프레임된 주석 편집기로 들어갑니다.
5. **수동** 영역을 만들려면 평소처럼 사각형을 드래그하면 됩니다. 드래그가 클릭 임계값을 넘는 즉시 호버 프레임은 무시됩니다.

호버 강조 표시는 색상 피커(`C`) 또는 자(Ruler)(`R`) 시작 도구가 활성화된 동안에는 비활성화되며, 코드 스캐너(`Q`), 디스플레이 캡처(`D`), GIF / 비디오 녹화 시작 모드에서는 계속 사용할 수 있습니다.

### 2.3 올바른 모니터에서 창 선택

창 감지는 캡처 대상별로 실행됩니다. 다중 모니터 구성에서 각 정지된 창은 자신의 지오메트리와 교차하는 창만 받으므로, 호버 프레임은 해당 디스플레이에서 보이는 것과 일치합니다.

### 2.4 활성화 / 비활성화

이 기능은 기본적으로 활성화되어 있습니다(`windowDetection.enabled = true`). **설정 → 고급 → 창 감지 활성화(Window Detection Enabled)** 에서 전환하거나 `~/.config/mark-shot/config.json`을 편집하세요:

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

- `command`: 감지 스크립트. GNOME / KDE / Hyprland / niri Wayland에서는 세션과 일치하는 번들 `mark-shot-window-detection-*` 스크립트가 자동으로 선택됩니다. X11과 Windows에서는 플랫폼이 프로세스 내에서 열거되므로 `command`를 비워 두어도 됩니다. **사용자가 제공한 사용자 지정 명령(예: 절대 경로)은 항상 존중됩니다.**
- `timeoutMs`: 스크립트에 대한 최대 대기 시간(100–30000ms, 기본값 1000).
- `env`: 스크립트에 전달되는 추가 환경 변수. 컴포지터별 조정(오프셋)은 스크립트 헤더에 문서화되어 있습니다.

### 2.5 문제 해결

| 증상 | 확인 방법 |
| :--- | :--- |
| GNOME Wayland에서 청록색 프레임 없음 | 확장이 활성화되었는지 확인 — 위 `gdbus` 호출이 버전을 반환해야 합니다 |
| X11 / Windows에서 청록색 프레임 없음 | 해당 없음 — 플랫폼 열거가 내장되어 있습니다. 캡처 세션이 시작 포인터 도구를 사용하지 않는지 확인하세요 |
| 호버 프레임이 잘못된(아래쪽) 창을 선택함 | 사용자 지정 감지 스크립트에서 z-order 데이터가 누락됨. `zOrder`가 없는 창은 최하위 레이어로 처리됩니다 |
| 캡처가 느리게 시작됨 | 감지 스크립트가 오버레이 전에 실행됩니다. 데스크톱이 느린 경우에만 `timeoutMs`를 올리거나, 건너뛰려면 `enabled:false`로 설정하세요 |
| 진단 내용 확인 | `mark-shot --debug --debug-log /tmp/mark-shot.log` 실행 — `window-detection` 줄을 확인하세요 |

---

## 3. 영역 선택 & 시작 도구

영역이 확정되기 전에 시작 오버레이 도구를 사용할 수 있습니다:

| 단축키 | 도구 | 동작 |
| :---: | :--- | :--- |
| `C` | 색상 피커(Color Picker) | 픽셀 샘플링; 휠로 돋보기 크기 조절; 왼쪽 클릭으로 색상 패널(HEX / RGB / HSL / HSV / Qt 형식) 열기; 오른쪽 클릭 또는 `Esc`로 종료 |
| `R` | 자(Ruler) | 호버 시 픽셀 좌표 표시; 왼쪽 드래그로 너비, 높이, 대각선, 면적이 포함된 사각형 측정; 오른쪽 클릭 또는 `Esc`로 종료 |
| `Q` | 코드 스캐너(Code Scanner) | QR / 바코드 주변 영역 드래그; 디코딩된 결과가 복사 가능한 창에 열림 |
| `D` | 디스플레이 캡처(Display Capture) | 모든 출력 캡처, 디스플레이별 크롭, 호버 가능한 썸네일 표시(복사 / 편집 / 저장) |
| `S` | 활성 GIF / 비디오 녹화 중지 | 오버레이에 표시된 녹화를 중지합니다 |

`Esc`는 세션을 취소합니다. 오른쪽 클릭(시작 도구 없음)도 취소합니다.

#### 3.1 다중 모니터 정지 동작

기본 캡처 범위인 **Freeze All Screens**를 사용하면 영역을 선택하는 동안 연결된 모든 디스플레이가 정지됩니다. 한 모니터에서 선택을 확정하면 다른 디스플레이는 비대화형 배경으로 정지된 프레임을 계속 표시합니다. 마우스, 키보드, 휠, 단축키 입력은 무시되고 오버레이는 도구 모음을 표시하지 않으므로 캡처 세션이 끝날 때까지 나머지 가상 데스크톱은 정지된 상태로 유지됩니다. 대신 **Cursor Screen** 범위(설정 → 캡처 → Freeze Scope)를 사용하면 커서 아래의 모니터만 정지되고 다른 화면은 완전히 사용할 수 있습니다.

---

## 4. 주석 도구

영역이 선택되고(또는 로컬 이미지가 열리고) 나면 주석 도구 모음과 함께 편집기가 열립니다. 도구는 숫자 키나 도구 모음으로 전환합니다:

| 단축키 | 도구 | 설명 |
| :---: | :--- | :--- |
| `V` | 이동 / 팬(Move / Pan) | 선택 영역 전체 이동, 로컬 이미지 캔버스 팬 |
| `S` | 선택(Select) | 기존 주석 선택, 이동, 크기 조절, 회전, 삭제 |
| `P` | 펜(Pen) | 부드러운 자유 곡선 획 |
| `L` | 선(Line) | 직선 |
| `H` | 형광펜(Highlighter) | 반투명 마커; 자유 곡선 또는 직선 스타일 |
| `R` | 사각형(Rectangle) | `획(Stroke)` / `강조(Highlight)` / `반전(Invert)` 스타일, 둥근 모서리 지원 상자 |
| `E` | 타원(Ellipse) | 타원 / 원 |
| `A` | 화살표(Arrow) | 클래식 화살표(깃털형, KDE, 양방향) |
| `T` | 텍스트(Text) | 서식 있는 텍스트; 휠 또는 슬라이더로 크기 조절; 대각선 핸들은 양쪽 모두 스케일, 옆면 핸들은 줄바꿈 조절; 글꼴 패널에서 정확한 pt 크기, 글꼴 패밀리, 굵게 / 기울임꼴 설정 |
| `N` | 번호(Number) | 순차 번호 마커(아라비아 숫자, 알파, 로마 숫자, 중국어 등) |
| `M` | 모자이크(Mosaic) | 민감한 내용을 가리기 위한 아크릴 흐림 효과 |
| `G` | 레이저(Laser) | 자동으로 사라지는 임시 획 |

그리기 팁:

- 사각형 / 타원을 그리는 동안 `Ctrl`을 누르고 있으면 정사각형 / 원으로 제한됩니다.
- 도구가 활성화된 상태에서 휠을 스크롤하면 획 두께, 텍스트 크기, 번호 배율, 모자이크 블록 크기를 조절할 수 있습니다(실시간 미리 보기).
- `선택(Select)` 상태에서 스크롤하면 캔버스가 확대/축소되고 가운데 버튼을 누른 채로 팬합니다. `Ctrl`을 두 번 탭하면 초기화됩니다.

### 4.1 기존 주석 편집

**선택(Select)(`S`)** 으로 전환합니다. 주석을 클릭하면 핸들이 표시됩니다:

- 안쪽을 드래그하면 이동;
- 모서리 / 가장자리 핸들을 드래그하면 크기 조절;
- 위쪽 가장자리 위의 원형 핸들을 드래그하면 회전;
- `Delete` / `Backspace`를 누르면 삭제;
- 텍스트를 더블 클릭하면 그 자리에서 편집.

속성 패널(오른쪽)에서 선택한 주석을 편집합니다: 색상, 두께, 스타일, 텍스트 글꼴 패밀리 / 크기 / 굵게 / 기울임꼴. `선택` 도구에서 선택 상자를 드래그하면 여러 주석을 선택할 수 있으며, 그룹을 함께 이동, 크기 조절, 회전, 삭제할 수 있습니다.

### 4.2 작업

| 단축키 | 동작 |
| :--- | :--- |
| `Ctrl+C` | 클립보드에 복사 |
| `Ctrl+S` / `Enter` | 저장(설정의 경로 템플릿 사용) |
| `Ctrl+P` | 떠 있는 스티커 창으로 핀 |
| `Ctrl+U` | 설정된 이미지 호스트에 업로드; URL이 복사됨 |
| `Ctrl+Z` / `Ctrl+Y` | 실행 취소 / 다시 실행 |
| `F` | 캡처 범위 전환(선택 영역 ↔ 전체 화면) |

### 4.3 내보내기 프레임

**설정 → 내보내기 → Mac 스타일 프레임**을 활성화하면 저장 / 복사 / 업로드된 이미지에 투명 여백, 둥근 모서리, 부드러운 그림자가 추가됩니다.

---

## 5. 핀 고정 창 스티커

| 제스처 / 단축키 | 동작 |
| :--- | :--- |
| 왼쪽 드래그 | 스티커 위치 이동 |
| 휠 | 비율 유지 크기 조절 |
| 왼쪽 더블 클릭 / `Esc` | 닫기 |
| 오른쪽 클릭 | 상황에 맞는 메뉴(회전, 확대/축소, 항상 위에 표시, 텍스트 복사, 번역, 저장, 복사, 닫기) |

핀 고정된 창 안의 OCR 텍스트는 선택 및 복사(`Ctrl+C` / 상황에 맞는 메뉴)할 수 있습니다. 번역(OpenAI 호환 엔드포인트)은 번역된 텍스트를 원래 레이아웃 위치에 다시 이미지 위에 렌더링합니다.

---

## 6. 스크롤 스크린샷

1. 영역을 선택합니다(매우 큰 영역은 떠 있는 드래그 핸들 사용).
2. 오버레이가 대상 창을 스크롤합니다. 캡처된 프레임은 긴 이미지로 이어 붙여집니다.
3. GNOME Wayland에는 Mark Shot Scroll Helper 확장(§ 2.1)이 필요합니다.

스크롤 캡처는 niri 및 유사한 wlroots/Wayland 컴포지터에서 프로덕션 준비가 되어 있습니다. KDE, X11 및 기타 스택에서는 테스트 기능입니다. 실패하면 일반 스크린샷이나 사용자 지정 확장 명령을 사용하세요.

---

## 7. 헤드리스 캡처 (CLI)

비대화형 캡처는 PNG를 쓰고 JSON을 출력합니다:

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

모든 헤드리스 옵션은 위치 인수(포지셔널) 이미지 파일과 상호 배타적입니다. 전체 인수 표는 README를 참조하세요.

### 7.1 헤드리스 창 / 구성 요소 캡처

Mark Shot은 **특정 창 — 또는 창 내부의 구성 요소(하위 영역) — 을 UI를 열지 않고 캡처**할 수 있습니다. 스크립트, 빌드 파이프라인 또는 에이전트에서 사용할 수 있습니다. 프로세스는 이미지가 기록되거나 반환되는 즉시 종료되며, 창을 만들지 않고, 대화 상자를 띄우지 않으며, 포커스를 빼앗지 않으므로 도구가 데스크톱을 캡처하는 동안에도 사용자는 계속 작업할 수 있습니다.

먼저 창 목록을 확인하여 무엇을 사용할 수 있는지 알아보세요:

```bash
mark-shot --list-windows
```

출력 예시(GNOME Wayland):

```json
{"count":2,"platform":"wayland","source":"compositor-script","windows":[
  {"index":0,"id":"0x3c00007","title":"Mark Shot - VSCodium","class":"codium","instance":"codium","x":1920,"y":0,"width":1680,"height":1050,"zOrder":1},
  {"index":1,"title":"Terminal","class":"org.gnome.Terminal","x":67,"y":32,"width":800,"height":600}
]}
```

각 항목은 선택기가 일치시키는 필드를 담고 있습니다: `index`, `id`(X11 창 ID / 백엔드 제공 ID), `title`, `class`, `instance`, 그리고 `x`/`y`/`width`/`height`와 선택적 `zOrder`입니다.

#### 7.1.1 창 선택(단일 또는 다중)

`--window`는 반복할 수 있어 **한 번의 호출로 원하는 수만큼의 창을 캡처**할 수 있습니다. 각 선택기는 자동으로 해석됩니다(`--window-by auto`):

| 선택기 값 | 일치 대상 |
| :--- | :--- |
| `0`, `1`, … | 목록 `index` |
| `0x3c00007` | 창 `id` |
| `VSCodium` | `class` 또는 `instance`, 그다음 `title`(정확히 일치 후 부분 문자열) |
| `Mark Shot - VSCodium` | `title` |

`--window-by id|title|class|index`로 일치 규칙 하나를 강제합니다. 여러 창과 일치하는 선택기는 **모두** 캡처합니다.

선택기에 `@x,y,width,height`를 붙이면 구성 요소(창 내부 하위 영역)를 캡처합니다. 오프셋은 창의 왼쪽 위 모서리 기준이며 창 경계로 클램프됩니다:

```bash
# the top 100px strip of window 0
mark-shot --window "0@0,0,1680,100" --capture-destination file --capture-to /tmp/shots/
```

#### 7.1.2 이미지 저장 위치 선택

`--capture-destination`은 출력 대상을 결정합니다. 임의 개수의 `--window` 선택기와 구성 요소 하위 영역과 결합할 수 있습니다:

| 대상 | 동작 |
| :--- | :--- |
| `inline`(기본값) | JSON 출력에 포함된 Base64 PNG. **파일이 기록되지 않고 클립보드도 건드리지 않습니다.** 픽셀만 필요한 에이전트에게 가장 안전한 선택입니다. |
| `file` | `--capture-to <directory>`에 PNG 파일 기록. 해당 옵션이 필요합니다. |
| `stage` | 임시 스테이징 디렉터리(`$TMPDIR/mark-shot-staging`)에 PNG 파일 기록. "나중에 보관" 워크플로에 좋습니다. |
| `clipboard` | 이미지를 시스템 클립보드에 복사. 여러 이미지일 경우 **마지막 것이 승리**합니다. 콘텐츠는 CLI 종료 후에도 유지됩니다(지속형 `wl-copy` / `xclip` 소유자가 생성됨). |

예시:

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

**클립보드 정책.** 대화형 편집기는 의도적으로 선택 내용을 시스템 클립보드에 올립니다(`복사` 동작 / `Ctrl+C`). 이는 스크린샷 도구의 주요 워크플로이기 때문입니다. 헤드리스 모드(CLI 및 엔터프라이즈 MCP 서버)는 반대 규칙을 따릅니다: **`clipboard`가 대상으로 명시적으로 선택되고 클립보드 쓰기가 설정 > 저장소 > 헤드리스 모드(Headless Mode)에서 활성화된 경우가 아니면 클립보드는 절대 수정되지 않습니다.** `inline`(기본값)과 `stage`는 사용자의 현재 클립보드 내용을 그대로 두므로, 예약된 캡처나 에이전트 주도 캡처가 사용자가 다른 곳에서 작업 중인 텍스트나 이미지를 덮어쓸 수 없습니다. 헤드리스 클립보드 쓰기가 비활성화되어 `clipboard` 요청이 거부되면, 캡처는 설정된 헤드리스 기본 대상으로 폴백하고, JSON 출력(`"warning"`)과 stderr가 이를 알려주며, 프로세스는 자동화가 감지할 수 있도록 0이 아닌 코드로 종료됩니다. 설정에서 헤드리스 클립보드 쓰기를 활성화하려면 확인 암호 문구를 입력해야 합니다.

출력은 캡처된 창마다 하나의 항목이 있는 JSON 객체 `{"captures":[...]}`입니다. 모든 항목은 선택기, 창 식별 정보, 최종 캡처 사각형을 반복하며, `path`(file/stage) 또는 `data`(inline) 또는 둘 다 없음(clipboard)이 함께 제공됩니다. 종료 코드는 모든 선택기가 일치하고 모든 캡처가 성공한 경우에만 `0`입니다. 일치 실패나 캡처 실패가 있으면 조용한 성공 대신 `"error"` 필드와 함께 종료 코드 `1`이 반환됩니다.

동일한 캡처 파이프라인은 프로그래밍 방식으로 주석이 달린 출력을 생성할 수 있습니다. 엔터프라이즈 에디션의 MCP 서버 장을 참조하거나, 저장된 PNG를 대화형 편집기와 결합하세요.

#### 7.1.3 창 간섭 없음 보장

모든 헤드리스 모드는 보이지 않고 비간섭적임이 보장됩니다:

- **창이 절대 생성되지 않습니다** — 주석 편집기, 캡처 오버레이, 트레이를 포함합니다. 캡처는 헤드리스 캡처 경로를 재사용합니다;
- **대화 상자가 절대 표시되지 않습니다** — 오류 대화 상자도 포함합니다: 오류는 stderr로 갑니다; 잘못된 명령줄(`--window` 없이 `--window-by` 사용, 알 수 없는 `--capture-destination`, 추가 위치 파일 등)도 `QMessageBox`를 띄우거나 대화형 UI로 넘어가지 않고 즉시 0이 아닌 코드와 stderr 메시지로 종료됩니다;
- 대화형 포털 프롬프트가 표시되지 않습니다(`allowInteractivePortal` 비활성화);
- 프로세스는 출력 기록 직후 종료됩니다;
- 헤드리스 작업 전후에 캡처된 창 목록은 동일합니다;
- 헤드리스 모드는 `clipboard`가 명시적으로 요청되고 **또한** 클립보드 쓰기가 설정 > 저장소 > 헤드리스 모드에서 활성화된 경우가 아니면 시스템 클립보드를 건드리지 않습니다.

감지된 창이 없으면(예: 비활성화된 컴포지터 헬퍼 또는 창 열거가 없는 X11 세션) 명령은 stderr에 명확한 오류를 출력하고 아무것도 캡처하지 않은 채 조용히 종료하는 대신 코드 `1`로 종료합니다.

---

## 8. 데스크톱 단축키 & 트레이

트레이 모드(`mark-shot --tray`)는 영역 캡처용 `Ctrl+Alt+S`를 등록하고 캡처 / 녹화 / 설정 / 종료 메뉴 항목을 제공합니다. 데스크톱 단축키:

- **GNOME**: 설정 → 키보드 → 단축키 → 사용자 지정 단축키 → `mark-shot`에 바인딩.
- **KDE**: `mark-shot`에 바인딩된 사용자 지정 단축키(정확한 KDE 캡처를 위한 KWin ScreenShot2 권한 포함, README 참조).
- **Hyprland**: `bind = SUPER SHIFT, S, exec, mark-shot` 및 `bind = , Print, exec, mark-shot`.
- **niri**: `binds { Mod+Shift+S { spawn "mark-shot"; } }`.
- **Sway / i3**: `bindsym Mod4+Shift+S exec mark-shot`.

---

## 9. 설정 & 백엔드

- 설정 파일: `~/.config/mark-shot/config.json`(Linux), 첫 실행 시 생성됩니다.
- 전체 참조: [설정](configuration.md).
- 백엔드: Wayland(PipeWire 포털 / grim / wlroots screencopy), X11(`QScreen::grabWindow`), Windows(네이티브 WGC). 녹화는 PipeWire 포털을 선호하며 자동으로 폴백합니다.
- 설정 창은 저장되지 않은 변경 사항을 결정적으로 추적합니다: 모든 컨트롤(드롭다운, 스위치, 스핀 상자, 텍스트 필드, 단축키 필드, 색상 피커)은 콤보 상자 팝업과 모달 색상 대화 상자에서 선택한 값도 포함하여 저장되지 않은 변경 표시기를 즉시 업데이트합니다. 변경을 되돌리면 표시기가 지워지므로 창은 닫을 때 실제로 보류 중인 편집이 있는 경우에만 확인을 요청합니다.

선택적 헬퍼:

```bash
# OCR (RapidOCR / Tesseract)
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime

# Code scan (zxing-cpp)
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

---

## 10. 기능 테스트 체크리스트

빌드를 종단 간 검증할 때 사용하세요:

1. **실행** — `run-mark-shot.sh`가 정지된 오버레이를 엽니다.
2. **창 호버** — 창 위로 마우스를 움직입니다: 청록색 프레임이 따라옵니다; 단일 클릭으로 창이 선택됩니다; 겹치는 창은 가장 위쪽이 선택됩니다.
3. **수동 영역** — 사각형 드래그; 놓기; 편집기가 열립니다.
4. **주석** — 각 도구로 그립니다(펜, 선, 사각형, 타원, 화살표, 형광펜, 텍스트, 번호, 모자이크, 돋보기, 레이저); 실행 취소/다시 실행; 선택으로 이동/크기 조절/회전/삭제; 텍스트 더블 클릭으로 편집.
5. **복사 / 저장 / 핀 / 업로드** — `Ctrl+C`, `Ctrl+S`, `Ctrl+P`, `Ctrl+U`.
6. **시작 도구** — `C` 색상 피커, `R` 자, `Q` 코드 스캔, `D` 디스플레이 캡처.
7. **헤드리스** — `--capture-to`, `--region`, `--display`, `--list-displays`.
8. **헤드리스 창 캡처** — `--list-windows`가 데스크톱을 나열합니다; `--window`를 반복하여 여러 창 캡처; 4가지 모드(inline, file, stage, clipboard) 모두에서 `--capture-destination` 테스트; 구성 요소 선택기(`--window "0@0,0,400,300"`) 검증; 전후 창 목록이 동일한지 확인(창 간섭 없음).
9. **트레이 + 단축키** — `mark-shot --tray`, `Ctrl+Alt+S` 누르기.
10. **휴대용 세부 사항** — 번들이 자체 Qt 라이브러리/플러그인/스크립트를 찾습니다.

---

## 11. 피드백

번들에 포함된 [이슈 제출 가이드](../.doc/submit-issue-via-gh.md)를 사용하여 `gh issue create`로 문제를 보고하세요. `mark-shot --debug --debug-log /tmp/mark-shot.log`로 캡처한 디버그 로그를 첨부하세요.
