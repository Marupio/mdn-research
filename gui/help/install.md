## Installation Guide

_Version 1.0.0_ has been released.  I have finally produced a precompiled _install package_ for **Windows**.  You can also _build from source_.  This application should build on **all platforms**.  I have tested it on:

* _Windows 11 Home Edition_, and
* _Linux: Ubuntu 24.04 LTS (Noble Numbat)_,

but it _should_ be possible to install it on others.

## Installation Packages

**➜ [Download the latest build (ZIP)](https://github.com/Marupio/mdn-research/releases/latest)**
* Use the **.zip** file for **Windows**
* Use the **.deb** file for **Linux**

If you prefer a specific version, grab it from the Releases page. For example:

- **[v1.1.1 (Windows ZIP)](https://github.com/Marupio/mdn-research/releases/download/v1.1.1/MDN-v1.1.1-win64-Release.zip):** *The latest version (Windows build).*
    https://github.com/Marupio/mdn-research/releases/download/v1.1.1/MDN-v1.1.1-win64-Release.zip

- **[v1.1.1 (Linux .deb)](https://github.com/Marupio/mdn-research/releases/download/v1.1.1/mdn_1.1.1-1_amd64.deb):** *The latest version (Linux build).*
    https://github.com/Marupio/mdn-research/releases/download/v1.1.1/mdn_1.1.1-1_amd64.deb

- **[v1.1.0 (Windows ZIP)](https://github.com/Marupio/mdn-research/releases/download/v1.1.0/MDN-v1.1.0-win64-Release.zip):** *The latest version (Windows build).*
    https://github.com/Marupio/mdn-research/releases/download/v1.1.0/MDN-v1.1.0-win64-Release.zip

- **[v1.1.0 (Linux .deb)](https://github.com/Marupio/mdn-research/releases/download/v1.1.0/mdn_1.1.0-1_amd64.deb):** *The latest version (Linux build).*
    https://github.com/Marupio/mdn-research/releases/download/v1.1.0/mdn_1.1.0-1_amd64.deb

- **[v1.0.1 (Windows ZIP)](https://github.com/Marupio/mdn-research/releases/download/v1.0.1/MDN-v1.0.1-win64-Release.zip):** *No dependencies.*
    https://github.com/Marupio/mdn-research/releases/download/v1.0.1/MDN-v1.0.1-win64-Release.zip
- **[v1.0.0 (Windows ZIP)](https://github.com/Marupio/mdn-research/releases/download/v1.0.0/MDN-v1.0.0-win64-Release.zip):** _This version requires [VCC redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) to be installed on your system.
    https://github.com/Marupio/mdn-research/releases/download/v1.0.0/MDN-v1.0.0-win64-Release.zip

### Windows

**How to run: Windows**
1. Optional: verify the SHA-256 checksum listed on the Release page.
2. Unzip the file anywhere (e.g., `C:\Apps\MDN`).
3. Double-click `mdn_gui.exe`. (Windows may warn about an unknown publisher—expected for an unsigned build.)

### Linux

**How to run: Linux**
1. Download the latest .deb file
2. Execute commands to install it, e.g.:
```bash
    # install MDN app
    sudo dpkg -i ~/Downloads/mdn_1.1.0-1_amd64.deb

    # It probably will complain:
    #
    # dpkg: error processing package mdn (--install):
    #  dependency problems - leaving unconfigured
    # Processing triggers for hicolor-icon-theme (0.17-2) ...
    # Errors were encountered while processing:
    #  mdn

    # To fix this, install dependencies
    sudo apt-get install -f

    # It will have a lot of work to do:

    # The following NEW packages will be installed:
    #   libb2-1 libdouble-conversion3 libevdev2 libgomp1
    #   libgudev-1.0-0 libice6 libinput-bin libinput10
    #   libmd4c0 libmtdev1t64 libopengl0 libpcre2-16-0
    #   libproxy1v5 libqt6core6t64 libqt6dbus6t64
    #   libqt6gui6t64 libqt6network6t64 libqt6opengl6t64
    #   libqt6qml6 libqt6qmlmodels6 libqt6quick6
    #   libqt6waylandclient6 libqt6waylandcompositor6
    #   libqt6waylandeglclienthwintegration6
    #   libqt6waylandeglcompositorhwintegration6
    #   libqt6widgets6t64 libqt6wlshellintegration6 libsm6
    #   libts0t64 libwacom-common libwacom9 libxcb-icccm4
    #   libxcb-image0 libxcb-keysyms1 libxcb-render-util0
    #   libxcb-shape0 libxcb-util1 libxcb-xkb1 libxkbcommon-x11-0
    #   qt6-gtk-platformtheme qt6-qpa-plugins qt6-translations-l10n
    #   qt6-wayland
    # 0 upgraded, 43 newly installed, 0 to remove and 0 not upgraded.
    # 1 not fully installed or removed.
    # Need to get 0 B/16.7 MB of archives.
    # After this operation, 66.4 MB of additional disk space will be used.
    # Do you want to continue? [Y/n] y
```
3. Default installation goes to **/usr/bin**, and the executable is **mdn_gui**.

## Build From Source

This is FOSS.  It is possible to build from source on nearly any platform.

### Prerequisites

This app relies on:

* A **C++ compiler** that supports _C++17_, (such as **MSVC**, **clang**, **GCC**)
* Build tools **CMake** _version 3.16_ or later
* **Qt 5** or **Qt 6**, specifically _Widgets_, _Core_ and _Gui_
* _Optional:_ **Ninja** build tool
* _Optional:_ **Git** to clone this repository

### Download and build the code

Download and build the project:

```
git clone https://github.com/Marupio/mdn-research.git
cd mdn-research
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

You need to have the prerequisites, and they all need to work with each other:

* You need **git** to download the code.  Alternatively, download a zip from github and unzip in your working directory.
* You need **cmake** to create a compilable configuration.  **cmake** needs to know where your **Qt** installation is.
* You need **Qt** to compile the gui aspect of the code.
* Finally you need a **C++ Compiler** to compile the code into an executable application.

### Run the application

The application will be located:

* On Windows:
```
<build directory>/gui/mdn_gui.exe
```

* On Linux:
```
<build directory>/gui/mdn_gui
```
