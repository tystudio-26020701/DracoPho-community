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

다른 언어로 보기：
[简体中文](../README.zh-CN.md) · [繁體中文](./README.zh-TW.md) ·
[日本語](./README.ja.md) · [한국어](./README.ko.md) ·
[Русский](./README.ru.md) · [Italiano](./README.it.md) ·
[العربية](./README.ar.md) · [Français](./README.fr.md) ·
[Deutsch](./README.de.md) · [Español](./README.es.md) ·
[Português](./README.pt.md)

**태그**: `C++` / `Qt 6` / `屏幕截图` / `图像标注` / `桌面贴图` / `OCR 识别` / `滚动长截图` / `Wayland` / `Windows`


<details>
<summary>데모 비디오</summary>
<p align="center">
  <video src="https://github.com/user-attachments/assets/4f86fcee-fef9-409e-98ba-1491ecee06c7" width="100%" controls></video>
</p>
</details>

DracoPho는 Qt 6 기반의 고성능 스크린샷 주석 도구입니다. 프로젝트는 원래 `niri` 같은 Wayland 창 관리자를 위해 설계되었으며, 현재 Linux(X11, GNOME, wlroots/Wayland 데스크톱)와 Windows 환경에서 일반적인 스크린샷 및 주석 작업 흐름을 지원합니다.

화면을 즉시 캡처하고 적응형 전체 화면 주석 오버레이를 열어 영역 선택, 주석, 클립보드 복사, 저장, 데스크톱 핀 등의 기능을 제공합니다.

---

## 핵심 기능

### 주석 도구 상자
- **펜과 형광펜**: 부드러운 자유 곡선 그리기와 반투명 하이라이트 겹치기를 지원합니다.
- **기하 벡터 도구**: 정밀한 직선, 사각형, 타원 경로를 제공합니다. 사각형은 다음 세 가지 스타일로 전환할 수 있습니다.
  - `描边`: 기존의 윤곽선 또는 채우기 사각형으로, 모서리 둥글림을 선택할 수 있습니다.
  - `高亮`: `CompositionMode_Multiply`와 반투명 채우기로 구현한 형광펜 스타일의 오버레이 효과입니다.
  - `反色`: 사각형이 덮는 영역의 픽셀을 RGB 반전시키고, 외곽선을 시각적 힌트로 유지합니다.
- **최적화된 화살표**: 6개 꼭짓점의 클래식 화살표 경로를 사용하며, 부드러운 가장자리와 앤티앨리어싱 렌더링을 지원합니다.
- **이중 연동 텍스트**:
  - 마우스 휠이나 속성 슬라이더로 초대형 글자 크기를 무단계로 부드럽게 조절할 수 있습니다.
  - 물리적 너비 버퍼 설계를 도입하여 극도로 높은 확대 비율에서 렌더링 지터로 인한 의도치 않은 줄바꿈을 방지합니다.
  - **대각선 제어점**으로 글자 크기와 텍스트 상자의 비율 연동 확대/축소를 할 수 있으며, **좌우 측면 제어선**은 레이아웃 경계 너비만 조절합니다.
- **레이저 프레젠터 펜**: 프레젠테이션이나 교육용으로 적합하며, 필적은 시간이 지나면 부드럽게 녹아 사라집니다.
- **자동 증가 단계 번호**: 클릭하면 1씩 증가하는 숫자 단계 표식을 배치합니다.
- **모자이크**: 민감한 정보에 모자이크 영역 블러 처리를 지원합니다.
- **이중 프레임 독립 조절 돋보기**: 돋보기의 내부 뷰파인더 프레임과 외부 렌즈는 각각 크기 조절 핸들을 갖습니다. 사각형 렌즈는 프레임당 모서리/변 8개 핸들, 원형 렌즈는 프레임당 상하좌우 4개 핸들이 있습니다. 한 프레임을 조절하면 확대 배율에 따라 다른 프레임도 연동되며 배율은 항상 유지됩니다. 한 프레임을 평행 이동하면 다른 프레임은 제자리에 유지됩니다.
- **시작 단계 코드 스캔**: 영역 선택 전에 `Q`를 누르면 코드 스캔 모드로 전환되며, QR 코드나 바코드 영역을 드래그하면 복사 가능한 인식 결과 창이 열립니다.
- **빠른 디스플레이 캡처**: 영역 선택 전에 `D`를 누르면 모든 출력 화면을 즉시 캡처하고 디스플레이별로 잘라 썸네일을 만듭니다. 썸네일 위로 마우스를 올리면 해당 디스플레이의 스크린샷을 복사, 편집 또는 저장할 수 있습니다.
- **GIF, MP4 및 애니메이션 WebP 녹화**: 시작 단계 녹화 단축키나 트레이 메뉴를 통해 지정한 디스플레이나 사용자 지정 영역을 GIF, MP4 또는 애니메이션 WebP로 녹화할 수 있습니다. 녹화는 크래시 세이프합니다: MP4는 임시 MKV로 기록되었다가 종료 시 리먹싱되므로, 중단된 녹화도 복구할 수 있습니다. 진행 중인 녹화는 트레이와 고정 프레임에 상태가 표시되며, `S`, 오버레이 버튼, 트레이 메뉴 또는 `--stop-recording`으로 중지할 수 있고 시작 및 저장 시 데스크톱 알림이 전송됩니다. Wayland에서는 PipeWire portal 백엔드가 우선 사용되며, portal 캡처를 사용할 수 없으면 wlroots screencopy 또는 폴링 캡처로 대체됩니다.
- **이미지 호스팅 업로드**: 영역 선택 후 `Ctrl+U`를 누르거나 도구 모음의 업로드 버튼을 클릭하면 현재 스크린샷을 사용자 지정 이미지 호스팅(예: ImgURL, sm.ms, imgbb, litterbox 등)에 업로드하며, 업로드가 성공하면 URL이 자동으로 클립보드에 복사됩니다. `upload.env`로 이미지 호스팅 매개변수를 구성하거나 `upload.command`로 임의의 사용자 지정 업로드 스크립트를 연결할 수 있습니다.
- **Mac 스타일 내보내기 외곽선**: 저장, 복사, 업로드, 열기 방식 및 확장 명령 이미지에 투명 여백, 둥근 모서리, 부드러운 그림자를 추가합니다.

### 고정 핀 (Pin)
- 스크린샷이나 주석 영역을 독립적이고 테두리 없는 항상 위 플로팅 핀 창으로 화면에 고정할 수 있습니다.
- 핀 창에서 OCR로 인식된 텍스트를 직접 선택하고 `Ctrl + C` 또는 오른쪽 클릭 메뉴로 이미지 텍스트를 복사할 수 있습니다.
- OpenAI 호환 API를 통해 LLM으로 OCR 텍스트를 번역하고, 번역문을 원본 이미지 위치에 맞춰 핀에 오버레이로 렌더링할 수 있습니다.
- **편리한 조작**:
  - 마우스 왼쪽 버튼으로 드래그하면 핀 위치를 자유롭게 이동할 수 있습니다.
  - 마우스 휠을 스크롤하면 핀을 비율에 맞게 확대/축소할 수 있습니다.
  - 마우스 왼쪽 버튼을 두 번 클릭하거나 `Esc` 키를 누르면 핀을 닫을 수 있습니다.
  - 오른쪽 클릭으로 메뉴를 열면 여러 각도 회전, 이미지 텍스트 복사, 번역, 다른 이름으로 저장, 복사 또는 닫기를 지원합니다.

### 스크롤 캡처
- PipeWire screencast, 대화형 스크롤 오버레이, 이미지 스티처를 통해 긴 페이지나 긴 영역을 캡처합니다.
- 이 기능은 주로 `niri` 및 유사한 동작을 하는 Wayland 환경을 대상으로 합니다. 이러한 환경에서는 출력 지오메트리, 캡처 타이밍, 창 위치가 더 안정적으로 유지됩니다.
- **대형 선택 영역 플로팅 핸들**: 선택한 캡처 영역이 너무 커서 화면 남은 공간에 스크롤 미리보기 패널을 표시할 수 없으면 미리보기 패널이 자동으로 숨겨지고, 선택 영역 가장자리에 **플로팅 드래그 핸들**(방향 화살표가 있는 플로팅 버튼)이 표시됩니다.
  - **드래그로 선택 영역 조정**: 플로팅 핸들을 누른 채 드래그하여 캡처 영역을 스크롤 축 방향으로 이동시키면 초기 화면 범위를 벗어난 콘텐츠도 캡처할 수 있습니다.
  - **클릭으로 축 전환**: 캡처를 시작하기 전에 플로팅 핸들을 클릭하면 스크롤 방향(세로/가로)을 바로 전환할 수 있습니다.
- **호환성 참고**: KDE, GNOME, X11 및 기타 비 `niri` 환경의 스크롤 캡처는 아직 테스트 기능으로 완성도가 낮습니다. 이러한 데스크톱 스택은 portal 백엔드 정책, 셸 또는 창 관리자 동작, 창 지오메트리 피드백, 프레임 타이밍, 스크롤 이벤트 처리 방식이 다릅니다.
- 스크롤 캡처를 사용할 수 없다면 일반 스크린샷 흐름을 사용하거나, DracoPho 확장 명령으로 외부 긴 캡처 도구를 연결하세요.
- 스크롤 캡처 문제를 제출하려면 먼저 `mark-shot --debug --debug-log /path/to/mark-shot.log`를 실행하여 문제를 재현한 뒤 로그를 GitHub issue에 첨부하세요. `config.json`에서 `debug.enabled`와 `debug.logPath`로도 활성화할 수 있습니다. `DEBUG=1`과 `MARK_SHOT_DEBUG_LOG=/path/to/log`도 계속 사용할 수 있습니다.

### 교차 디스플레이 서버 지원
- **Wayland**: PipeWire portal screencast로 녹화와 실험적 스크롤 캡처를 지원하며 공유 메모리와 DMA-BUF 두 가지 프레임 경로를 처리합니다. `grim`으로 wlroots 스크린샷을, `layer-shell-qt`로 네이티브 오버레이를 만들고, `wl-copy`로 클립보드를 유지합니다.
- **X11**: `QScreen::grabWindow`로 스크린샷을 찍고, 전체 화면 항상 위 창을 오버레이로 사용하며, `xclip`으로 클립보드를 유지합니다.
- **Windows**: Qt 네이티브 스크린샷 및 클립보드 API로 기본 스크린샷, 주석, 복사, 저장, 핀 흐름을 지원합니다. PipeWire, xdg-desktop-portal, `grim`, XCB 창 감지, LayerShellQt, GNOME Shell helper 등 Linux 전용 백엔드는 컴파일 시 비활성화됩니다.
- Linux 디스플레이 서버 백엔드는 런타임에 `$XDG_SESSION_TYPE`으로 자동 감지됩니다. Windows는 Qt 네이티브 플랫폼 백엔드를 사용합니다.
- **Multi-monitor freeze scope**: 기본적으로 영역 선택 시 연결된 모든 디스플레이가 고정됩니다(X11/Windows에서 DPR이 일치하면 단일 가상 데스크톱 창으로 표시). 한 모니터에서 선택을 확정하면 다른 디스플레이는 세션이 종료될 때까지 고정된 채 상호 작용할 수 없습니다. **Cursor Screen** 범위는 커서가 있는 모니터만 고정합니다.

### 데스크톱 통합
- **시작 동작 구성 가능**: DracoPho 시작 시 캡처 오버레이가 더 이상 기본으로 열리지 않습니다. 설정 → 일반 → **시작 동작**에서 **트레이 아이콘**, **플로팅 볼**, **설정 창**, **직접 캡처** 네 가지 모드를 자유롭게 조합할 수 있습니다. 새 설치 기본값은 트레이 아이콘 + 플로팅 볼이며, 직접 캡처는 선택 항목입니다.
- **플로팅 볼**: 항상 위에 표시되는 작은 원형 위젯(기본값 화면 오른쪽 아래)으로 캡처, 전체 화면 캡처, 녹화, 설정에 빠르게 접근할 수 있습니다. 클릭 한 번으로 메뉴, 더블 클릭으로 바로 캡처, 드래그로 이동(위치가 기억됩니다)하며 캡처 중에는 자동으로 숨겨집니다.
  화면 가장자리 근처로 끌어다 놓으면 자동으로 달라붙어 **가장자리 쪽 절반이 화면 밖으로 숨겨집니다**(마우스를 올리면 다시 나타나고, 멀어지면 다시 숨겨집니다. 모든 플랫폼에서 적용). 유휴 상태가 몇 초 지속되면 반투명하게 자동 페이드아웃되고, 마우스를 올리면 바로 원래 불투명도로 돌아옵니다.
- **데스크톱 바로가기**:
  - `mark-shot.desktop`: 시스템 전체 스크린샷 도구로 구성되어 시스템 단축키로 바로 호출할 수 있습니다.
  - `mark-shot-edit.desktop`: 독립적인 이미지 편집기로 등록되어 파일 관리자(예: Dolphin, Nautilus)의 오른쪽 클릭 "연결 프로그램" 메뉴에 통합할 수 있습니다.
- 고해상도 `dracoPho.svg` 및 `mark-shot-edit.svg` 시스템 벡터 아이콘이 함께 제공됩니다.

### KDE KWin ScreenShot2 권한

KDE Wayland에서 DracoPho은 KWin의 `org.kde.KWin.ScreenShot2` 인터페이스를 사용하여 정밀한 영역 스크린샷을 찍을 수 있습니다. KWin은 이 인터페이스를 제한된 D-Bus 인터페이스로 취급하므로, 애플리케이션의 데스크톱 파일은 권한 필드를 선언해야 합니다.

<details>
<summary>KDE KWin ScreenShot2 권한 및 데스크톱 파일 구성 설명 (클릭하여 펼치기)</summary>

권한 필드 선언:
```ini
X-KDE-DBUS-Restricted-Interfaces=org.kde.KWin.ScreenShot2
```

배포판 패키지와 `cmake --install`은 필요한 데스크톱 파일을 자동으로 설치합니다. 프로젝트를 설치하지 않고 로컬 빌드 산출물을 직접 실행하는 경우 `~/.local/share/applications/mark-shot.desktop`을 만들거나 업데이트하세요:

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

KDE의 명령 단축키 서비스를 통해 DracoPho을 바인딩하려면 `~/.local/share/applications/net.local.mark-shot.desktop`도 만들어야 합니다:

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

데스크톱 파일을 수정한 후에는 로그아웃 후 다시 로그인하여 KDE가 데스크톱 파일 캐시를 다시 읽도록 권장합니다. 현재 KDE 세션에서 여전히 `NoAuthorized`가 반환되면 KWin을 다시 시작하거나 시스템을 한 번 재부팅하세요.
</details>


---

## 제품 기능 비교

DracoPho 커뮤니티 에디션은 **오픈소스(MIT)이며 크로스 플랫폼(Linux X11/Wayland 네이티브 + Windows)이고 완전히 오프라인으로 동작하는** 스크린샷·주석·고정·OCR·번역·녹화 올인원 도구입니다. 아래 표는 각 제품의 공식 문서와 공식 웹사이트를 기준으로 정리한 것입니다(**2026년 8월** 기준). 모든 주요 데스크톱 플랫폼의 가장 인기 있는 스크린샷 도구(오픈소스 및 상용)를 다룹니다. 기능의 유무를 있는 그대로 표시합니다: **✅ 내장 지원**; **⭕ 부분 지원(유료 등급, 플랫폼 또는 외부 도구/서비스에 의해 제한됨)**; **❌ 미지원**. 모든 기능은 "설치 직후 바로 사용 가능" 기준으로 집계되며, 유료 등급, 클라우드 서비스 또는 실험적 브랜치와 무관합니다. ⭕의 정확한 제한 사항은 아래 상세 설명에서 다룹니다.

### 핵심 기능 개요

#### I. 캡처 (원하는 것을 얻을 수 있는가)

| 기능 | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 영역 스크린샷 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 전체 화면/멀티 모니터 스크린샷 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 창 스크린샷 | ⭕ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 가려진/최소화된 창 내용 | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| 스크롤/긴 스크린샷 | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ⭕ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 예약/지연 스크린샷 | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⭕ | ❌ | ✅ |

#### II. 주석 및 지능 기능 (캡처 후 효율적으로 작업할 수 있는가)

| 기능 | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 주석 도구 세트 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 화면 고정 Pin | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ |
| OCR 텍스트 인식 | ✅ | ✅ | ✅ | ⭕ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 번역 | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| QR/바코드 인식 | ✅ | ✅ | ✅ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ⭕ |
| 색상 선택기 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ✅ | ✅ | ✅ | ✅ |

#### III. 출력 및 자동화 (결과를 공유/보관할 수 있는가)

| 기능 | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 화면 녹화 | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ |
| 애니메이션 GIF/WebP 출력 | ✅ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ❌ | ✅ |
| 이미지 호스팅 업로드/클라우드 | ✅ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ |
| 헤드리스 CLI/스크립트 | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| 플로팅 볼 빠른 실행 | ✅ | ❌ | ⭕ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| 캡처 기록 | ❌ | ✅ | ❌ | ✅ | ✅ | ⭕ | ❌ | ❌ | ⭕ | ❌ | ✅ | ✅ | ⭕ | ❌ | ⭕ |

#### IV. 플랫폼 및 생태계

| 기능 | DracoPho CE | ShareX | PixPin | Snipaste | Flameshot | ksnip | Spectacle | Greenshot | PicPick | Snipping Tool | Snagit | CleanShot X | Shottr | Xnip | iShot |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Linux Wayland 네이티브 | ✅ | ❌ | ❌ | ❌ | ⭕ | ⭕ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| 크로스 플랫폼(데스크톱 OS 2개 이상) | ⭕ | ❌ | ⭕ | ✅ | ✅ | ✅ | ❌ | ⭕ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ |
| 플러그인/확장 메커니즘 | ✅ | ⭕ | ❌ | ❌ | ⭕ | ✅ | ⭕ | ✅ | ❌ | ❌ | ⭕ | ⭕ | ⭕ | ❌ | ❌ |
| 오픈소스/무료 | ✅ | ✅ | ⭕ | ⭕ | ✅ | ✅ | ✅ | ✅ | ⭕ | ✅ | ❌ | ❌ | ✅ | ⭕ | ⭕ |

### 기능 상세 설명

**I. 캡처**(원하는 것을 얻을 수 있는가)

- **영역/전체 화면/멀티 모니터 스크린샷** — 나열된 15개 도구 모두에 내장되어 있습니다. DracoPho은 추가로 음수 좌표 디스플레이와 디스플레이별 HiDPI 배율을 올바르게 처리하며, 멀티 모니터 캡처를 이어 붙일 때 고스트 이미지가 없습니다.
- **창 스크린샷** — DracoPho의 대화형 모드는 전체 화면 선택 오버레이를 사용합니다(전용 "활성 창" 원클릭 모드가 없으므로 ⭕). 반면 헤드리스 모드(`--window`)는 id, 제목, 클래스, **PID 또는 프로세스 이름**으로 창을 정확히 선택합니다. Flameshot에는 창 모드가 전혀 없고, 나머지 모든 도구에는 창 모드가 있습니다.
- **가려진/최소화된 창 내용** — X11에서 DracoPho은 XComposite를 통해 창 자체의 합성 버퍼를 읽어 창이 완전히 가려지거나 최소화된 상태에서도 실제 내용을 캡처합니다(창을 띄우거나 포커스를 빼앗지 않음). 유사한 도구 중 scrot의 `--stack`만이 이를 수행할 수 있습니다. Wayland 프로토콜은 모든 도구가 최소화된 창 내용을 읽는 것을 막습니다.
- **스크롤/긴 스크린샷** — DracoPho은 niri와 GNOME Wayland(공식 확장)에서 네이티브로 동작하며, KDE/X11은 실험 기능입니다. ShareX, PixPin, PicPick, Snagit, CleanShot X, Shottr, Xnip, iShot은 내장 지원하며, Greenshot은 레거시 IE 시나리오에서만 지원합니다. Snipaste, Flameshot, ksnip, Spectacle, Windows Snipping Tool은 지원하지 않습니다.
- **예약/지연 스크린샷** — DracoPho는 설정한 초만큼 기다린 뒤 캡처를 시작합니다(전체 화면 카운트다운 오버레이, Esc로 취소). `--delay`, 트레이/플로팅볼의 "지연 스크린샷" 하위 메뉴, 캡처 설정 페이지에서 사용할 수 있습니다. Flameshot·ksnip·Spectacle 등은 단순 지연 시작만, ShareX는 타이머 캡처를 지원하며, PixPin·Snipaste·Xnip은 지원하지 않습니다.

**II. 주석 및 지능 기능**(캡처 후 효율적으로 작업할 수 있는가)

- **주석 도구 세트** — DracoPho에는 펜, 형광펜, 직선, 사각형, 타원, 화살표, 텍스트, 단계 번호, 모자이크, 이중 프레임 돋보기, 레이저 포인터, 색상 선택기, 자 등 12개 이상의 도구가 내장되어 있습니다. 15개 도구 모두 다양한 수준의 주석 기능을 내장하고 있습니다.
- **화면 고정 Pin** — DracoPho은 확대/축소, 회전, OCR 단어 선택, 고정 이미지 내 LLM 번역을 지원하는 테두리 없는 항상 위 고정 스티커를 제공합니다. Snipaste, PixPin, ShareX, ksnip, CleanShot X, Shottr, Xnip, iShot도 지원하며, Flameshot, Spectacle, Greenshot, PicPick, Snipping Tool, Snagit은 지원하지 않습니다.
- **OCR 텍스트 인식** — DracoPho은 RapidOCR(오프라인 PP-OCR 모델)과 Tesseract 폴백을 내장하여 설치 직후 바로 사용할 수 있습니다. Windows Snipping Tool(로컬 텍스트 작업), ShareX, Snagit, CleanShot X, Shottr, Xnip, iShot도 내장하고 있으며, Snipaste는 유료 등급으로, ksnip은 플러그인으로, Spectacle은 외부 Tesseract 설치로 제한됩니다. Flameshot, Greenshot, PicPick은 없습니다.
- **번역** — DracoPho에는 OpenAI 호환 LLM 스크린샷 번역이 내장되어 있습니다(오프라인/자체 호스팅 지원). 스크린샷 번역을 내장한 도구는 iShot뿐이며, PixPin은 멤버십으로 제한하고 그 외에는 어떤 도구도 제공하지 않습니다. 이는 여전히 가장 강력한 차별화 요소 중 하나입니다.
- **QR/바코드 인식** — DracoPho은 캡처 중 `Q` 키로 QR, 1차원 코드, PDF417을 스캔합니다. ShareX와 PixPin이 지원하며, Snipaste(유료)와 CleanShot X/Shottr/iShot은 OCR 엔진을 통해 QR만 읽습니다.
- **색상 선택기** — DracoPho에는 기록 팔레트가 있는 화면 색상 선택기가 내장되어 있습니다. Spectacle과 Snagit을 제외한 대부분의 도구가 제공하며, Windows Snipping Tool은 Copilot+ AI PC에서만 제공합니다.
- **원클릭 AI 마스킹** — WeChat의 2026년 자동 마스킹은 아직 내장되지 않았습니다(❌). DracoPho의 모자이크/블러는 수동입니다. Snagit(AI Smart Redact)과 Snipping Tool의 텍스트 마스킹은 유사한 구현이며, 자동 마스킹은 가능한 후속 작업입니다.

**III. 출력 및 자동화**(결과를 공유/보관할 수 있는가)

- **화면 녹화** — DracoPho은 창, 대화 상자, 포커스 선점 없이 조용히 무인으로 영역/디스플레이를 MP4/GIF/애니메이션 WebP로 녹화합니다. ShareX, PixPin, Spectacle(Plasma 6), PicPick, Snipping Tool, Snagit, CleanShot X, iShot도 지원하며, Snipaste, Flameshot, ksnip, Greenshot, Shottr, Xnip은 지원하지 않습니다.
- **애니메이션 GIF/WebP 출력** — DracoPho은 **애니메이션 WebP**(파일 크기 감소, 알파 지원)를 네이티브로 녹화합니다. 어떤 경쟁 제품도 애니메이션 WebP를 네이티브로 생성하지 못합니다. PixPin과 iShot은 GIF를 녹화하고, ShareX는 GIF를 녹화하며(WebP는 정적 저장만), Snagit과 CleanShot X는 GIF로 내보냅니다. 나머지는 애니메이션 출력이 없습니다.
- **이미지 호스팅 업로드/클라우드** — DracoPho은 ImgURL/sm.ms/imgbb/litterbox/사용자 지정 명령 업로드(`Ctrl+U`)를 내장하고 있습니다. ShareX는 업로드 대상이 가장 많고, Greenshot, PicPick, Snagit, CleanShot X는 클라우드 업로드를 내장하고 있습니다. Flameshot은 Imgur만, ksnip은 Imgur/FTP/스크립트, Shottr은 활성화 후 S3를 지원합니다. Snipaste, PixPin, Snipping Tool, Xnip, iShot은 없습니다.
- **헤드리스 CLI/스크립트** — DracoPho은 완전한 헤드리스 파이프라인을 제공합니다: `--capture-to` 영역/디스플레이 스크린샷, `--list-windows` + `--window` 창/구성 요소 캡처, `--record-region`/`--record-display` 무인 녹화, `--record-wait-json` 구조화된 상태 출력. 전 과정에서 창, 대화 상자, 포커스 선점이 없으며, **PID/프로세스 이름 지정과 가려지거나 최소화된 창 캡처**를 지원합니다. 엔터프라이즈 MCP 서버는 동일한 파이프라인을 재사용합니다. ShareX(Windows 전용), Flameshot, ksnip, Spectacle은 견고한 CLI를 제공하며, Snipaste의 CLI는 유료 등급 기능이고, PixPin은 JS 동작 엔진만 제공합니다.
- **플로팅 볼 빠른 실행** — DracoPho에는 화면 가장자리에 달라붙는 드래그 가능한 플로팅 볼(X11/Windows/macOS, Wayland는 프로토콜 제한으로 자유 부유 상태 유지)이 있으며, 유휴 상태에서는 페이드아웃됩니다. 유사한 기능은 PixPin만 제공합니다.
- **캡처 기록** — DracoPho에는 아직 전용 기록 패널이 없습니다. ShareX, Snipaste, Flameshot, Snagit, CleanShot X는 있으며, ksnip/PicPick/Shottr/iShot은 가벼운 대안(탭/갤러리/고정 보관함)을 제공하고 나머지는 없습니다.

**IV. 플랫폼 및 생태계**

- **Linux Wayland 네이티브** — DracoPho은 PipeWire portal, grim, layer-shell, KDE KWin ScreenShot2, GNOME 확장을 네이티브로 지원합니다. 오픈소스 도구 중 Spectacle(KDE)만이 그 수준에 도달했으며, Flameshot/ksnip은 실험적이거나 portal에 의존합니다.
- **크로스 플랫폼** — Snipaste, Flameshot, ksnip, Snagit은 세 가지 주요 데스크톱 OS(Windows + macOS + Linux)를 모두 지원합니다. DracoPho은 Linux + Windows를 지원하며(macOS 계획 중), ShareX, PicPick, Snipping Tool, Spectacle, CleanShot X, Shottr, Xnip, iShot은 단일 플랫폼입니다.
- **플러그인/확장 메커니즘** — DracoPho은 GitHub 플러그인 마켓플레이스가 포함된 Qt 플러그인 시스템을 제공합니다(OCR/번역/코드 스캔 provider 확장 가능). ksnip과 Greenshot은 플러그인 API를 가지며, ShareX, Spectacle, Snagit, CleanShot X, Shottr은 사용자 지정 동작/통합으로 대체합니다.
- **오픈소스/무료** — DracoPho CE는 MIT 라이선스이며 완전히 무료이고, 광고 없이, 계정 없이, 네트워크 연결 없이 사용할 수 있습니다. ShareX, Flameshot, ksnip, Spectacle, Greenshot(Windows)도 오픈소스 무료이며, Shottr과 Snipping Tool은 무료입니다. PixPin/Snipaste/PicPick/Xnip/iShot은 폐쇄형 소스 프리미엄(freemium)이고, Snagit/CleanShot X는 유료 상용 소프트웨어입니다.

> 참고: 각 제품의 공식 웹사이트와 공식 문서(2026-08)를 기준으로 정리되었습니다. 기능은 각 릴리스에서 변경될 수 있으므로 최신 문서를 참조하세요. FastStone Capture, Nimbus, Lightshot, Sogou 캡처, WeChat/QQ 스크린샷은 위 표에 포함되지 않았습니다(WeChat의 2026년 원클릭 AI 마스킹이 해당 분야를 선도합니다).

---

## 명령줄 인터페이스 (CLI)

### 일반적인 사용 예시

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

#### 헤드리스(비대화형) 스크린샷

스크립트, CI 자동화 또는 기타 프로그램은 주석 UI를 열지 않고도 `mark-shot`을 호출하여 스크린샷을 찍을 수 있습니다. 캡처한 프레임은 PNG로 저장되고 표준 출력에 한 줄의 간결한 JSON 요약이 출력됩니다:

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

단일 디스플레이 `--capture-to`의 JSON 출력 예시:

```json
{"path":"/tmp/shot.png","width":2560,"height":1440,"output":"DP-1","error":null}
```

여러 개의 `--display`를 지정하면 출력은 화면당 하나의 캡처 배열이 됩니다:

```json
{"captures":[{"path":"/tmp/shots/mark-shot-DP-1-20260801-000000.png","width":2560,"height":1440,"output":"DP-1","error":null},
             {"path":"/tmp/shots/mark-shot-DP-2-20260801-000000.png","width":1920,"height":1080,"output":"DP-2","error":null}]}
```

선택된 각 디스플레이는 자체 소스 지오메트리로 캡처되므로 portal 계열 백엔드는 전체 가상 데스크톱이 아니라 해당 디스플레이를 정확히 반환합니다.

헤드리스 스크린샷은 대화형 UI와 동일한 모든 캡처 백엔드(QScreen,
xdg-desktop-portal, PipeWire, grim, KWin/GNOME 헬퍼, Windows Graphics Capture)를
재사용하므로 이미지 품질과 영역 크롭 동작이 완전히 동일합니다. 모든 헤드리스 옵션은 위치 이미지 파일 인수와 상호 배타적입니다.

### CLI 매개변수 설명

| 매개변수 | 기능 설명 |
| :--- | :--- |
| `[file]` | **위치 인수**: 현재 화면을 캡처하는 대신 기존 로컬 이미지 파일을 열어 주석 모드로 진입합니다. |
| `-h`, `--help` | 도움말을 표시하고 종료합니다. |
| `-v`, `--version` | 현재 버전 정보를 표시하고 종료합니다. |
| `--all-outputs` | 현재 활성 화면만 캡처하는 대신 가상 디스플레이 데스크톱의 모든 출력 화면을 캡처합니다. |
| `--xdg-window` | 기본 Wayland 오버레이(layer-shell) 대신 표준 XDG 전체 화면 일반 창(xdg-shell)을 강제로 사용합니다. |
| `--fullscreen` | 영역 선택 단계를 건너뛰고 캡처한 전체 화면 스크린샷에 바로 주석을 답니다. |
| `--default-tool <tool>` | 일반 영역 선택 완료 후 기본 주석 도구를 지정합니다. `--fullscreen-default-tool`이 설정되지 않은 경우 전체 화면 모드의 기본 도구로도 사용됩니다. |
| `--fullscreen-default-tool <tool>` | 전체 화면 주석 모드의 기본 도구를 지정합니다. |
| `--default-color <color>` | 기본 주석 색상을 지정합니다. `#RRGGBB` 및 `#RRGGBBAA`를 지원합니다. |
| `--tray` | DracoPho을 시스템 트레이에서 계속 실행하며, 플랫폼이 지원하면 전역 스크린샷 단축키를 등록합니다. |
| `--capture` | 설정에서 트레이 자동 시작이 활성화된 경우 단일 스크린샷을 강제로 트리거합니다. |
| `--pin-image <path>` | 스크린샷 및 영역 선택 흐름을 건너뛰고 로컬 이미지를 핀 창으로 바로 엽니다. |
| `--recording-status` | 실행 중인 인스턴스를 통해 현재 녹화 상태 JSON을 출력합니다. |
| `--stop-recording` | 실행 중인 인스턴스에 현재 진행 중인 녹화를 중지하도록 요청합니다. |
| `--record-region <x,y,w,h>` | 실행 중인 인스턴스를 통해 화면 영역을 녹화합니다. 지오메트리는 x,y,너비,높이 형식입니다. |
| `--record-display <id>` | ID로 디스플레이를 녹화합니다(`--list-displays` 출력 참조: `DP-1` 같은 원시 화면 이름, `output:DP-1` 키 또는 `all`). |
| `--record-output <path>` | 녹화의 출력 파일 경로(녹화 플래그 사용 시 필수). |
| `--record-duration <seconds>` | 녹화 시간(초). 0이면 `--stop-recording`까지 녹화합니다. |
| `--record-fps <n>` | 녹화 프레임 레이트(기본값 15). |
| `--record-format <mp4\|gif\|webp>` | 녹화 형식: mp4(기본값), gif 또는 webp. |
| `--record-audio` | 녹화에 시스템 오디오를 포함합니다. |
| `--record-wait-json` | 녹화가 끝날 때까지 대기한 후 최종 상태를 JSON으로 출력합니다. |
| `--debug` | 이번 실행에 대한 디버그 로그를 활성화합니다. |
| `--no-debug` | 이번 실행에 대한 디버그 로그를 비활성화하고 구성 파일과 환경 변수를 덮어씁니다. |
| `--debug-log <path>` | 디버그 로그를 지정된 경로에 기록합니다. `--no-debug`도 함께 설정하지 않는 한 디버그 로그가 활성화됩니다. |
| `--capture-to <path>` | 헤드리스 스크린샷: UI를 열지 않고 PNG를 지정된 파일 또는 디렉터리에 기록하며 표준 출력에 JSON 요약을 출력합니다. |
| `--region <x,y,w,h>` | `--capture-to`와 함께 사용: 지정된 논리 화면 영역만 캡처합니다. |
| `--display <name>` | `--capture-to`와 함께 사용: 디스플레이 이름으로 지정된 출력 화면을 캡처합니다. 여러 디스플레이를 한 번에 캡처하려면 반복 지정할 수 있습니다(화면당 PNG 하나). |
| `--include-cursor` | `--capture-to`와 함께 사용: 마우스 커서를 캡처 프레임에 그립니다. |
| `--output-name <name>` | `--capture-to`와 함께 사용: 캡처 경로가 디렉터리인 경우 사용되는 기본 파일 이름(확장자 제외). |
| `--list-displays` | 현재 모든 디스플레이 정보를 JSON으로 출력하고 종료합니다. |

### 단축키 바인딩

`mark-shot`을 시스템 스크린샷 단축키로 바인딩합니다:

**niri**(`~/.config/niri/config.kdl` 수정):
```kdl
binds {
    Mod+Shift+S { spawn "mark-shot"; }
}
```

**Hyprland**(`~/.config/hypr/hyprland.conf` 수정):
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bind = SUPER SHIFT, S, exec, mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bind = , Print, exec, mark-shot
```

**Sway / i3**(`~/.config/sway/config` 또는 `~/.config/i3/config` 수정):
```ini
# 绑定 Super+Shift+S 启动 mark-shot 选区截图
bindsym Mod4+Shift+S exec mark-shot
# 绑定 Print 按键启动 mark-shot 选区截图
bindsym Print exec mark-shot
```

**GNOME**: 시스템 설정 → 키보드 → 키보드 단축키 → 사용자 지정 단축키에서 추가합니다.

**트레이 모드**:
```powershell
mark-shot --tray
```

트레이 모드는 기본적으로 다음 전역 단축키를 등록합니다:
- `Ctrl+Alt+S`: 영역 스크린샷 시작.

트레이 메뉴에는 스크린샷, 전체 화면 스크린샷, 녹화 시작, 녹화 상태, 녹화 중지, 설정, 종료 등의 작업도 제공됩니다.


### 확장 명령

오른쪽 작업 도구 모음에는 **Extensions** 버튼이 있으며, 프로그램은 `~/.config/mark-shot/extensions.json`에서 사용자 지정 명령을 읽습니다. 구성 파일은 JSON 배열이거나 `commands` 배열을 포함하는 JSON 객체일 수 있습니다.

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

`command`는 유닉스 계열 시스템에서는 `$SHELL -c`로, Windows에서는 `%COMSPEC% /C`로 실행되므로 셸 표현식을 지원합니다. `{slurp}`를 사용하면 현재 선택 영역을 `x,y widthxheight` 지오메트리 문자열로 명령에 전달할 수 있습니다. `{image}` 또는 `{imagePath}`를 사용하면 현재 렌더링된 선택 영역을 임시 PNG 경로로, `{imageUrl}`을 사용하면 `file://` URL로 전달할 수 있습니다. 이 자리표시자는 셸 인용 이스케이프가 자동으로 처리되므로 구성에 추가 인용 부호를 넣지 마세요. 이미지 자리표시자를 사용하지 않는다면 `saveImage` 또는 `needsImage`를 `true`로 설정하면 프로그램이 임시 PNG 경로를 명령 끝에 자동으로 추가합니다. `workingDirectory`는 `cwd`와 동일합니다. `closeOnStart`의 기본값은 `true`이며, 명령 시작 전에 DracoPho을 숨기고 닫습니다.

### 애플리케이션 구성 파일

[구성 참조](../docs/configuration.zh-CN.md)를 참조하세요.

### 사용자 매뉴얼

일상적인 작업(창 위에 마우스를 올려 영역 선택, 주석 도구, 시작 도구, 핀 창, 긴 캡처, 헤드리스 CLI
및 기능 자가 테스트 체크리스트)은 [사용자 매뉴얼](../docs/user-guide.zh-CN.md)
([English](../docs/user-guide.md))을 참조하세요.

다른 언어 버전:
[简体中文](../docs/user-guide.zh-CN.md) · [繁體中文](../docs/user-guide.zh-TW.md) ·
[日本語](../docs/user-guide.ja.md) · [한국어](../docs/user-guide.ko.md) ·
[Русский](../docs/user-guide.ru.md) · [Italiano](../docs/user-guide.it.md) ·
[العربية](../docs/user-guide.ar.md) · [Français](../docs/user-guide.fr.md) ·
[Deutsch](../docs/user-guide.de.md) · [Español](../docs/user-guide.es.md) ·
[Português](../docs/user-guide.pt.md)

## 빌드 및 설치

### 설치 안내

##### Arch Linux (AUR)
Arch Linux 사용자는 AUR 헬퍼로 바로 설치할 수 있습니다:
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

`mark-shot`은 소스에서 컴파일되며, `mark-shot-bin`은 GitHub Releases에서 미리 컴파일된 pacman 패키지를 다운로드하여 설치합니다.

##### NixOS
NixOS 사용자는 Flake input을 추가하여 설치할 수 있습니다
```nix
# flake.nix
mark-shot = {
  url = "github:tystudio-26020701/DracoPho-community";
  inputs.nixpkgs.follows = "nixpkgs";
};

# home-manager
home.packages = with pkgs; [
  # 其他用户应用
  inputs.mark-shot.packages.${pkgs.stdenv.hostPlatform.system}.default
]
```

##### 기타 배포판 (사전 컴파일 설치 패키지)
기타 배포판(Ubuntu, Debian, Fedora 등)의 경우 Releases 페이지에서 컴파일된 설치 패키지를 다운로드하여 다음 명령으로 설치하세요:
- **Debian / Ubuntu**:
  ```bash
  sudo apt install ./mark-shot_<version>_amd64.deb
  ```
- **Fedora**:
  ```bash
  sudo dnf install ./mark-shot-<version>-1.x86_64.rpm
  ```

> **Ubuntu 26.04 LTS**: DracoPho은 Ubuntu 26.04 LTS(Resolute)에서 검증 및 지원됩니다.
> Ubuntu 26.04에서 소스에서 빌드하면 배포판에 포함된 Qt 6.10 패키지를 바로 사용할 수 있습니다
> (`aqtinstall` 단계 불필요):
>
> ```bash
> sudo apt install build-essential cmake ninja-build pkg-config \
>   qt6-base-dev qt6-wayland libpipewire-0.3-dev libxcb-cursor0 \
>   xdg-desktop-portal pipewire xclip
> cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
> cmake --build build
> ```
>
> 헤드리스 스크린샷(`--capture-to`), 다중 디스플레이 스크린샷(반복 가능한 `--display`) 및 로컬
> MCP 서비스는 모두 Ubuntu 26.04의 Wayland(GNOME)와 X11 세션에서 실행할 수 있습니다.

### 시스템 종속성

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

> **참고**: Ubuntu 22.04 등 시스템에 Qt 5가 내장된 환경에서는 Qt 6을 `~/Qt`에 설치해도 시스템에 영향을 주지 않습니다. 컴파일 시 `-DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64`를 전달하면 됩니다.

#### fcitx5 중국어 입력 지원 (X11 환경의 Qt 6)

Qt 6에는 fcitx5 입력기 플러그인이 포함되어 있지 않습니다. X11 환경에서 fcitx5 중국어 입력을 사용하려면 플러그인을 소스에서 컴파일해야 합니다:

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

#### OCR 백엔드 (선택 사항)

DracoPho의 텍스트 인식 기능은 내장된 `mark-shot-ocr` Python 스크립트에 의존합니다. 이 스크립트는 **RapidOCR**(권장, PaddleOCR PP-OCR 모델 기반)과 **Tesseract**(대체)를 지원합니다. Linux에서는 이 스크립트가 자동으로 설치되며, Windows에서는 수동으로 구성해야 합니다.

<details>
<summary><b>Linux</b></summary>

```bash
python3 -m venv ~/.local/share/mark-shot/ocr-venv
~/.local/share/mark-shot/ocr-venv/bin/pip install -U pip rapidocr onnxruntime
```

설치가 완료되면 `mark-shot-ocr`이 자동으로 발견되므로 추가 구성이 필요 없습니다.

**환경 변수**(선택 사항):

| 변수 | 설명 | 기본값 |
|------|------|--------|
| `MARK_SHOT_OCR_VERSION` | PaddleOCR 버전(`PP-OCRv5`, `PP-OCRv4` 등) | `PP-OCRv5` |
| `MARK_SHOT_OCR_MODEL_TYPE` | 모델 크기: `mobile` 또는 `server` | `mobile` |
| `MARK_SHOT_OCR_MODEL_DIR` | 사용자 지정 모델 저장 디렉터리 | `~/.local/share/mark-shot/models` |
| `MARK_SHOT_OCR_NO_VENV` | `1`로 설정하면 가상 환경 자동 전환을 비활성화합니다 | — |
| `MARK_SHOT_OCR_PYTHON` | re-exec에 사용할 Python 인터프리터 경로 지정 | `~/.local/share/mark-shot/ocr-venv/bin/python` |

</details>

<details>
<summary><b>Windows</b></summary>

내장 헬퍼 스크립트는 Windows에 자동으로 설치되지 않으므로 다음 단계를 수동으로 수행해야 합니다:

**1. Python 3 설치**

[python.org](https://www.python.org/downloads/)에서 Python 3.10 이상을 다운로드하여 설치합니다. 설치 시 **Add python.exe to PATH**를 체크하세요.

**2. OCR 헬퍼 스크립트 복사**

[DracoPho 저장소](https://github.com/tystudio-26020701/DracoPho-community)의 `scripts/mark-shot-ocr`를 `%LOCALAPPDATA%\mark-shot\mark-shot-ocr.py` 같은 로컬 디렉터리로 복사합니다.

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\mark-shot"
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/tystudio-26020701/DracoPho-community/main/scripts/mark-shot-ocr" `
  -OutFile "$env:LOCALAPPDATA\mark-shot\mark-shot-ocr.py"
```

**3. 가상 환경 생성 및 종속성 설치**

```powershell
python -m venv "$env:LOCALAPPDATA\mark-shot\ocr-venv"
& "$env:LOCALAPPDATA\mark-shot\ocr-venv\Scripts\pip.exe" install -U pip rapidocr onnxruntime
```

> `onnxruntime`은 CPU 추론을 제공합니다. 호환되는 GPU가 있다면 `onnxruntime-directml` 또는 `onnxruntime-gpu`를 설치하여 인식을 가속할 수 있습니다.

**4. `config.json`에서 `ocr.command` 구성**

`%LOCALAPPDATA%\mark-shot\config.json`을 열고(없으면 새로 생성) `ocr.command`를 설정합니다:

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

`%LOCALAPPDATA%`를 실제로 확장된 경로(예: `C:\Users\你的用户名\AppData\Local`)로 바꿉니다. `{image}` 자리표시자는 실행 시 임시 스크린샷 경로로 대체되며, 생략하면 DracoPho이 자동으로 추가합니다.

> **팁**: 이미 가상 환경의 Python을 직접 사용하고 있으므로, 환경 변수 `MARK_SHOT_OCR_NO_VENV=1`을 설정하면 스크립트 내장 가상 환경 자동 감지를 건너뛸 수 있습니다.

</details>

#### 코드 스캔 백엔드 (선택 사항)

```bash
python3 -m venv ~/.local/share/mark-shot/code-scan-venv
~/.local/share/mark-shot/code-scan-venv/bin/pip install -U pip zxing-cpp pillow
```

코드 스캔 헬퍼는 `zxing-cpp`를 우선 사용하며 QR Code, Data Matrix, Aztec, PDF417, EAN, UPC, Code 39, Code 93, Code 128 등 일반적인 형식을 지원합니다. `pyzbar` 또는 OpenCV가 설치되어 있으면 대체 백엔드로 사용됩니다.

#### 이미지 호스팅 업로드 백엔드 (선택 사항)

이미지 호스팅 업로드 기능은 기본적으로 내장된 `mark-shot-upload` Python 스크립트를 사용하며 추가 종속성 설치가 필요 없습니다(Python 3 표준 라이브러리만 사용). 이 스크립트는 환경 변수로 이미지 호스팅 매개변수를 구성하며, multipart/form-data 업로드 프로토콜을 지원하는 모든 이미지 호스팅 서비스를 지원합니다.

<details>
<summary>내장 헬퍼가 지원하는 환경 변수</summary>

| 환경 변수 | 설명 | 기본값 |
|---------|------|--------|
| `MARK_SHOT_UPLOAD_URL` | **필수**, 이미지 호스팅 업로드 API 엔드포인트 | — |
| `MARK_SHOT_UPLOAD_FIELD` | 파일 필드 이름 | `image` |
| `MARK_SHOT_UPLOAD_API_KEY` | API Key / Token | — |
| `MARK_SHOT_UPLOAD_AUTH_HEADER` | 인증 헤더 이름 | `Authorization` |
| `MARK_SHOT_UPLOAD_AUTH_SCHEME` | 인증 방식(예: `Bearer`), 비워 두면 API Key를 직접 사용 | `Bearer` |
| `MARK_SHOT_UPLOAD_URL_PATH` | JSON 응답에서 URL의 점 표기 경로(예: `data.url`) | 자동 감지 |
| `MARK_SHOT_UPLOAD_DELETE_URL_PATH` | 삭제 URL 경로 | 자동 감지 |
| `MARK_SHOT_UPLOAD_HEADER_xxx` | 사용자 지정 요청 헤더(예: `MARK_SHOT_UPLOAD_HEADER_X-Custom=foo`) | — |
| `MARK_SHOT_UPLOAD_FIELD_xxx` | 추가 양식 필드(예: `MARK_SHOT_UPLOAD_FIELD_album=123`) | — |

</details>

<details>
<summary>구성 예시: ImgURL V3</summary>

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

ImgURL V3는 `Authorization: Bearer <token>` 인증을 사용합니다(`AUTH_SCHEME` 기본값이 `Bearer`이므로 수정할 필요 없습니다).

</details>

<details>
<summary>구성 예시: sm.ms</summary>

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

sm.ms는 Token을 Authorization 값으로 직접 사용하므로 `AUTH_SCHEME`를 빈 문자열로 설정합니다.

</details>

<details>
<summary>구성 예시: imgbb</summary>

```json
"upload": {
  "env": {
    "MARK_SHOT_UPLOAD_URL": "https://api.imgbb.com/1/upload?key=你的API_KEY",
    "MARK_SHOT_UPLOAD_FIELD": "image",
    "MARK_SHOT_UPLOAD_URL_PATH": "data.url"
  }
}
```

imgbb는 URL 쿼리 매개변수로 API Key를 전달하므로 `API_KEY`를 설정할 필요가 없습니다.

</details>

<details>
<summary>구성 예시: litterbox (API Key 불필요한 임시 이미지 호스팅)</summary>

```json
"upload": {
  "command": "curl -sf --max-time 30 -A 'Mozilla/5.0' -F reqtype=fileupload -F time=72h -F fileToUpload=@{image} https://litterbox.catbox.moe/resources/internals/api.php",
  "timeoutMs": 35000
}
```

litterbox의 응답은 순수 텍스트 URL(JSON 아님)이며, DracoPho은 `http://`/`https://`로 시작하는 출력을 업로드 결과로 자동 인식합니다.

</details>

<details>
<summary>사용자 지정 업로드 명령</summary>

내장 헬퍼로 충분하지 않으면 `upload.command`를 통해 임의의 사용자 지정 업로드 스크립트를 연결할 수 있습니다. 명령은 다음 요구 사항을 충족해야 합니다:

1. **종료 코드**: 성공 시 종료 코드 0, 0이 아니면 실패로 간주
2. **출력 형식**(둘 중 하나):
   - **JSON**: `{"url":"https://...","deleteUrl":"https://...","errors":[]}`(`url` 필수, 나머지는 선택)
   - **순수 텍스트 URL**: stdout의 첫 번째 비어 있지 않은 줄이 `http://` 또는 `https://`로 시작
3. **자리표시자**: `{image}`, `{imagePath}`, `{imageUrl}` 지원. 명령에 자리표시자가 포함되지 않은 경우 DracoPho이 명령 끝에 임시 이미지 경로를 자동으로 추가

```json
"upload": {
  "command": "/path/to/your-uploader.sh --file {image} --json",
  "timeoutMs": 30000,
  "env": {
    "UPLOADER_API_KEY": "xxx"
  }
}
```

`upload.env`의 환경 변수는 사용자 지정 명령에도 함께 전달되므로 구성을 재사용하기 쉽습니다.

</details>

#### Windows

현재 컴파일러와 일치하는 Qt 6, CMake, Ninja, 그리고 C++17을 지원하는 컴파일러(예: MSVC 또는 MinGW)를 설치하세요. Windows 빌드에는 Qt DBus, PipeWire, X11/XCB, LayerShellQt, `grim`, `wl-copy` 또는 `xclip`이 필요 없습니다.

```powershell
cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
cmake --build build-windows
```

현재 Windows 지원 범위는 일반 스크린샷과 이미지 주석입니다. 스크롤 캡처, 컴포지터 전용 창 감지, Linux 데스크톱 바로가기는 Windows에서 사용할 수 없습니다. 내장 Python 헬퍼 스크립트(`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`)는 자동으로 설치되지 않으므로 위의 [OCR 백엔드](#ocr-后端可选), [코드 스캔 백엔드](#扫码后端可选) 및 번역 섹션을 참조하여 수동으로 구성하세요.

### 빌드 및 컴파일

```bash
# 使用系统 Qt 6
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 如果 Qt 6 安装在用户目录，额外指定 CMAKE_PREFIX_PATH
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.7.3/gcc_64

# 执行编译
cmake --build build
```

또는 nix 사용

```bash
nix build
```

LayerShellQt는 자동으로 감지됩니다. 발견되면 완전한 Wayland layer-shell 지원이 활성화되고, 발견되지 않으면 컴파일은 정상적으로 성공하며 런타임에 자동으로 표준 전체 화면 창으로 대체됩니다.

### 설치 및 통합

```bash
cmake --install build --prefix "$HOME/.local"
```

이 명령은 실행 파일, 헬퍼 스크립트(`mark-shot-ocr`, `mark-shot-code-scan`, `mark-shot-translate`, `mark-shot-upload`), 데스크톱 바로가기 및 아이콘을 설치합니다.

### GNOME Wayland 스크롤 캡처 확장

GNOME Wayland에서 스크롤 캡처를 사용하려면 **DracoPho Scroll Helper** 확장을 활성화해야 합니다. 이 확장이 없으면 DracoPho이 선택한 영역을 조용히 연속 캡처할 수 없고 GNOME 네이티브 스크롤 미리보기 패널도 그릴 수 없으므로, GNOME Wayland에서는 스크롤 캡처 버튼이 비활성화됩니다.

확장 파일은 프로젝트 저장소의 `../packaging/gnome-extension/mark-shot-scroll-helper@snemc.org` 경로에 있습니다.

<details>
<summary><b>GNOME Wayland 스크롤 캡처 확장 설치 및 활성화 가이드 펼치기/접기</b></summary>

##### 방법 A: 배포판 패키지로 설치
DracoPho을 배포판 패키지(예: `.deb` 또는 `.rpm`)로 설치했다면 확장이 이미 시스템에 기본 설치되어 있습니다. 다음 명령으로 현재 사용자에 대해 확장을 활성화할 수 있습니다:
```bash
gnome-extensions enable mark-shot-scroll-helper@snemc.org
```
*확장을 찾을 수 없다는 메시지가 표시되면 로그아웃 후 다시 로그인하고 다시 시도하세요.*

##### 방법 B: 저장소 소스 디렉터리에서 설치
소스 또는 로컬에서 수동으로 빌드했다면 먼저 확장을 사용자의 GNOME 확장 경로로 복사해야 합니다:
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

helper D-Bus 인터페이스 사용 가능 여부 확인:

```bash
gdbus call --session \
  --dest org.gnome.Shell \
  --object-path /org/gnome/Shell/Extensions/MarkShotScrollHelper \
  --method org.gnome.Shell.Extensions.MarkShotScrollHelper.Version
```

예상 결과는 `('4.2',)`입니다. 확장을 활성화한 후 `mark-shot`을 다시 시작하세요.

</details>

---

## 인터랙션 단축키 및 제스처 가이드

### 도구 전환 단축키

| 단축키 | 전환 대상 도구 | 기능 설명 |
| :---: | :--- | :--- |
| **V** | 이동 / 탐색 (Move / Pan) | 기존 이미지 모드에서 이미지 캔버스를 평행 이동하고 드래그하는 데 사용합니다. |
| **S** | 선택 (Select) | 그린 벡터 주석을 선택하여 이동, 크기 조절 또는 삭제합니다. |
| **P** | 펜 (Pen) | 자유 곡선 그리기. |
| **L** | 직선 (Line) | 곧은 벡터 선 그리기. |
| **H** | 형광펜 (Highlighter) | 반투명 하이라이트 오버레이로 강조 표시에 적합합니다. |
| **R** | 사각형 (Rectangle) | 사각형 테두리 그리기. |
| **E** | 타원 (Ellipse) | 타원형 테두리 그리기. |
| **A** | 화살표 (Arrow) | 6개 꼭짓점의 클래식하고 가늘고 긴 예리한 화살표 그리기. |
| **T** | 텍스트 (Text) | 서식 있는 텍스트 입력 및 배치, 1000px 글자 크기와 드래그 연동 지원. |
| **N** | 번호 (Number) | 자동 증가 단계 번호 표식. |
| **M** | 모자이크 (Mosaic) | 민감한 영역 모자이크 처리. |
| **G** | 레이저 펜 (Laser) | 교육이나 프레젠테이션용 임시 필적으로, 부드럽게 자동 사라집니다. |

### 시작 화면 보조 도구

| 단축키 | 도구 | 기능 설명 |
| :---: | :--- | :--- |
| **C** | 색상 선택기 (Color Picker) | 스크린샷 영역을 선택하기 전에 스크린샷 픽셀을 샘플링합니다. 마우스 휠을 스크롤하면 돋보기 크기를 조절하고, 왼쪽 클릭으로 색상 패널을 열어 HEX, RGB, HSL, HSV, Qt 등 형식으로 복사할 수 있습니다. 오른쪽 클릭 또는 Esc로 일반 영역 선택으로 돌아갑니다. |
| **R** | 자 (Ruler) | 스크린샷 영역을 선택하기 전에 좌표를 측정합니다. 마우스를 올리면 현재 픽셀이 표시되고, 왼쪽 버튼 드래그로 픽셀 눈금이 있는 측정 사각형을 그려 너비, 높이, 대각선, 면적을 표시합니다. 오른쪽 클릭 또는 Esc로 일반 영역 선택으로 돌아갑니다. |
| **Q** | 코드 스캔 (Code Scanner) | QR 코드 및 바코드 스캔 모드로 진입합니다. 영역을 드래그하면 포함된 코드 내용을 인식하여 복사 가능한 창에 결과를 표시합니다. 오른쪽 클릭 또는 Esc로 일반 영역 선택으로 돌아갑니다. |
| **D** | 디스플레이 캡처 (Display Capture) | 모든 출력 화면을 즉시 캡처하여 디스플레이별로 잘라 썸네일을 표시합니다. 썸네일 위로 마우스를 올리면 복사, 편집 또는 저장할 수 있습니다. |

### 전역 작업 단축키

| 단축키 | 실행 동작 |
| :---: | :--- |
| **Esc** | 즉시 종료하고 주석 창을 닫습니다. |
| **Ctrl + C** | 모든 텍스트 편집을 확정하고 현재 스크린샷/주석 영역을 시스템 클립보드에 복사합니다. |
| **Ctrl + S** 또는 **Enter / Return** | 모든 텍스트 편집을 확정하고 현재 스크린샷을 저장합니다. |
| **Ctrl + P** | 현재 영역을 플로팅 핀 창으로 고정합니다. |
| **Ctrl + U** | 현재 스크린샷을 사용자 지정 이미지 호스팅에 업로드하고, 성공하면 URL이 자동으로 클립보드에 복사됩니다. |
| **Ctrl + Z** | 이전 주석 작업을 실행 취소합니다. |
| **Ctrl + Y** 또는 **Ctrl + Shift + Z** | 실행 취소된 주석 작업을 다시 실행합니다. |
| **Backspace** 또는 **Delete** | **선택 (Select)** 도구가 활성화되어 있고 주석이 선택된 상태에서 선택한 주석을 삭제합니다. |
| **F** | 현재 캡처 범위 전환(영역 모드와 전체 화면 모드 전환). |

### 고급 인터랙션 팁

- **도형 그리기 제약**: **사각형(Rectangle)** 또는 **타원(Ellipse)**을 그릴 때 `Ctrl` 키를 누르고 있으면 정사각형 또는 정원으로 강제 제약됩니다.
- **선택 도구로 빠른 전환**: 주석 작업 중 캔버스 빈 곳을 마우스 오른쪽 버튼으로 클릭하면 즉시 **선택(Select)** 도구로 전환됩니다.
- **오른쪽 버튼 두 번 클릭으로 색상 빠른 전환**: 캔버스 빈 곳에서 마우스 오른쪽 버튼을 두 번 클릭하면 링 색상 팔레트가 열려 현재 주석 도구의 색상을 빠르게 전환할 수 있습니다.
- **휠 무단계 조절**: 해당 주석 도구가 활성화된 상태에서 마우스 휠을 스크롤하면 현재 도구의 선 두께, 글자 크기, 번호 표식 크기 또는 모자이크 격자 크기를 실시간으로 조절할 수 있습니다.
- **캔버스 평행 이동 및 확대/축소**: **선택(Select)** 도구 모드이거나 로컬 파일을 편집할 때 마우스 휠을 스크롤하면 캔버스를 매끄럽게 확대/축소할 수 있고, 마우스 가운데 버튼을 누른 채 드래그하면 캔버스를 이동할 수 있습니다. `Ctrl` 키를 두 번 누르면 확대/축소와 이동이 초기화됩니다.

### 핀 창 전용 인터랙션

| 상호작용 제스처 / 단축키 | 동작 효과 |
| :--- | :--- |
| **마우스 왼쪽 버튼 누른 채 드래그** | 데스크톱 핀 위치를 자유롭게 이동하고 배치합니다. |
| **마우스 휠 위/아래** | 핀 창을 비율에 맞게 무단계로 확대/축소합니다. |
| **마우스 왼쪽 버튼 두 번 클릭** | 해당 핀 창을 빠르게 닫습니다. |
| **마우스 오른쪽 버튼 클릭** | 기능 메뉴 표시(회전, 이미지 텍스트 복사, 번역, 저장, 복사, 닫기 등). |
| **Esc 키** | 현재 포커스가 있는 핀 창을 닫습니다. |

---

## 릴리스 노트

[릴리스 노트](../docs/releases.zh-CN.md)를 참조하세요.

## 피드백 및 커뮤니티

### Issue 제출
실행 중 문제가 발생하거나 새 기능 제안이 있으면 GitHub CLI(`gh`) 명령줄 도구로 Issue를 제출하는 것을 권장합니다. 환경 정보를 한 번에 수집하고 자동 생성하는 스크립트를 제공하므로 자세한 내용은 [Issue 제출 가이드](../.doc/submit-issue-via-gh.md)를 참조하세요.

---

## 라이선스 정보

이 프로젝트는 **MIT 라이선스**로 오픈소스로 제공됩니다. 자세한 내용은 [LICENSE](../LICENSE) 파일을 참조하세요.

## 감사의 글

DracoPho은 오픈소스 커뮤니티의 어깨 위에 서 있습니다. 진심 어린 감사를 드립니다:

- **원래 상위 프로젝트인 [jswysnemc/mark-shot](https://github.com/jswysnemc/mark-shot)과 그 저자 및 모든 기여자.** 이 커뮤니티 버전은 원래 상위 프로젝트를 기반으로 개발되었으며, 뛰어난 디자인과 지속적인 기여가 모든 것의 기초입니다. 그들의 훌륭한 작업에 진심으로 감사드립니다.
- **[serendipitywgy](https://github.com/serendipitywgy)**: `serendipitywgy/mark-shot`을 통해 크로스 데스크톱 호환성 개선, OCR 복사 도구 모음 작업, 스마트 사각형 영역 사전 선택 기능을 기여해 주셔서 감사합니다.
- **DracoPho이 의존하는 모든 오픈소스 프로젝트**: Qt 6, PipeWire, xdg-desktop-portal, layer-shell-qt, wl-clipboard, xclip, grim, RapidOCR, onnxruntime, Tesseract, ZXing-C++ 등.

이 커뮤니티 버전은 [北京太殷造物科技有限公司 (베이징 타이인 자오우 과학기술유한공사)](https://github.com/tystudio-26020701/DracoPho-community) 및 기여자들이 유지관리하며, **MIT 라이선스**로 오픈소스로 제공됩니다.
