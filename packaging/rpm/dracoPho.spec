Name:           dracoPho
Version:        0.1.41
Release:        1%{?dist}
Summary:        Qt 6 screenshot selection and annotation tool

License:        MIT
URL:            https://github.com/tystudio-26020701/DracoPho-community
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  cmake-rpm-macros
BuildRequires:  gcc-c++
BuildRequires:  ninja-build
BuildRequires:  pkgconfig
BuildRequires:  cmake(Qt6Concurrent)
BuildRequires:  cmake(Qt6Core)
BuildRequires:  cmake(Qt6DBus)
BuildRequires:  cmake(Qt6Gui)
BuildRequires:  cmake(Qt6Widgets)
BuildRequires:  cmake(LayerShellQt)
BuildRequires:  pkgconfig(libpipewire-0.3)
BuildRequires:  pkgconfig(x11-xcb)
BuildRequires:  pkgconfig(xcb)
BuildRequires:  pkgconfig(xcomposite)

Requires:       python3
Requires:       qt6-qtwayland
Requires:       grim
Requires:       wl-clipboard
Recommends:     xclip
Recommends:     xdg-desktop-portal
Recommends:     pipewire
Suggests:       tesseract
Suggests:       tesseract-langpack-chi_sim
Suggests:       gnome-shell

%description
DracoPho captures screenshots, annotates image regions, pins floating image
stickers, and provides OCR and translation helpers for pinned image text.

%prep
%autosetup

%build
%cmake -G Ninja \
    -DMARK_SHOT_WITH_LAYER_SHELL=ON \
    -DMARK_SHOT_REQUIRE_LAYER_SHELL=ON
%cmake_build

%install
%cmake_install
sed -i '1s|^#!/usr/bin/env python3$|#!/usr/bin/python3|' \
    %{buildroot}%{_bindir}/dracoPho-ocr \
    %{buildroot}%{_bindir}/dracoPho-code-scan \
    %{buildroot}%{_bindir}/dracoPho-translate \
    %{buildroot}%{_bindir}/dracoPho-upload \
    %{buildroot}%{_bindir}/dracoPho-window-detection-niri \
    %{buildroot}%{_bindir}/dracoPho-window-detection-hyprland \
    %{buildroot}%{_bindir}/dracoPho-window-detection-gnome \
    %{buildroot}%{_bindir}/dracoPho-window-detection-kde

%files
%license LICENSE
%doc %{_docdir}/dracoPho/
%{_bindir}/dracoPho
%{_bindir}/dracoPho-ocr
%{_bindir}/dracoPho-code-scan
%{_bindir}/dracoPho-translate
%{_bindir}/dracoPho-upload
%{_bindir}/dracoPho-window-detection-niri
%{_bindir}/dracoPho-window-detection-hyprland
%{_bindir}/dracoPho-window-detection-gnome
%{_bindir}/dracoPho-window-detection-kde
%{_libdir}/dracoPho/
%{_datadir}/dracoPho/python/
%{_datadir}/applications/dracoPho.desktop
%{_datadir}/applications/dracoPho-edit.desktop
%{_datadir}/applications/net.local.dracoPho.desktop
%{_datadir}/icons/hicolor/scalable/apps/dracoPho.svg
%{_datadir}/icons/hicolor/scalable/apps/dracoPho.svg
%{_datadir}/icons/hicolor/scalable/apps/dracoPho-edit.svg
%{_datadir}/gnome-shell/extensions/dracoPho-scroll-helper@snemc.org/

%changelog
* Thu Jul 16 2026 jswysnemc <snemc@qq.com> - 0.1.41-1
- Update to version 0.1.41

* Thu Jul 16 2026 jswysnemc <snemc@qq.com> - 0.1.40-1
- Update to version 0.1.40

* Fri Jul 10 2026 jswysnemc <snemc@qq.com> - 0.1.39-1
- Update to version 0.1.39

* Sun Jun 07 2026 jswysnemc <snemc@qq.com> - 0.1.22-1
- Update to version 0.1.22

* Sat Jun 06 2026 jswysnemc <snemc@qq.com> - 0.1.21-1
- Update to version 0.1.21

* Fri Jun 05 2026 jswysnemc <snemc@qq.com> - 0.1.20-1
- Update to version 0.1.20

* Fri Jun 05 2026 jswysnemc <snemc@qq.com> - 0.1.19-1
- Update to version 0.1.19

* Thu Jun 04 2026 jswysnemc <snemc@qq.com> - 0.1.18-1
- Update to version 0.1.18

* Thu Jun 04 2026 jswysnemc <snemc@qq.com> - 0.1.17-1
- Update to version 0.1.17

* Thu Jun 04 2026 jswysnemc <snemc@qq.com> - 0.1.16-1
- Initial Fedora RPM packaging
