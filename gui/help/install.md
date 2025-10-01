## Installation Guide

I haven't wrapped this up as pre-packaged binaries yet.  I haven't created a windows installer.  At this point, your only option is to _build from source_.

This application should build on **all platforms**.  I have tested it on:

* _Windows 11 Home Edition_, and
* _Linux: Ubuntu 24.04 LTS (Noble Numbat)_,

but it _should_ be possible to install it on others.

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
