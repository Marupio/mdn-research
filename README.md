# mdn-research

A research project into **Multi-Dimensional Numbers**.  Includes three main components:
* **library:** a _thread-safe_, _HPC-ready_ numerical library.
* **gui:** a _QT-based_ graphical user interface for experimenting with **MDNs**.
* **"sandbox"** an application library with various test apps, part of the dev process.

---

## Documentation

- [Overview](gui/help/overview.md) — *What are Multi-Dimensional Numbers?*
- [Installation Guide](gui/help/install.md) - *How to install the application*
- [User’s Guide](gui/help/guide.md) — *How to use the application*

---

## Release notes

### MDN 1.1.2

**Highlights**

* **Guide fixes & documentation updates** — clarified command-line help and improved installation guides.
* **Help resources restructured** — compressed resources disabled for easier patching and inspection.
* **Project loading bug fixed** — global configuration now correctly restored when re-loading saved projects.
* **Improved Linux build script** — streamlined CMake/Ninja workflow and checksum instructions.

**Core / Stability**

* Fixed **project load bug** where global configuration wasn’t restored correctly after opening a saved file, e.g. loading a project with base = 16, would not change the base to 16.
  (Commit: *Sync, corrected config lost on load project*.)

**User Interface & Help**

* Corrected **Guide** command-line documentation:

  * Properly quoted arguments like `--log-file "path"` and `--input "path"`.
  * Adjusted log-level syntax to show actual usage (`--log-level Debug4`).
* Updated **Help resources (QRC)** to disable compression for easier text editing and faster loading.
* Expanded **Install Guide** to include the new v1.1.2 builds and clarified previous release entries.
* Minor fixes to `overview.md` examples and number formatting.

**Build & Packaging**

* Incremented version to **1.1.2**.
* Reworked `linuxBuild.sh`:

  * Cleaned up structure (create and enter `build` directory explicitly).
  * Added comments and `sha256sum` usage hints.
  * Removed obsolete debug build lines.

* Updated `CMakeLists.txt` version field for consistency with new release.
* Verified `.deb` packaging includes desktop integration and icons.

### Version 1.1.1

* Fixed display for higher base digits - now uses alphanumeric characters 0-9, a-v
* Fixed linux build to disable logging, increasing performance


### MDN 1.1.1

**Highlights**

* **Logging disabled by default** — cleaner runs unless you explicitly enable it.
* **Alphanumeric digits for higher bases** — uses `a, b, c…` instead of multi-digit numerals (e.g. `a` for 10).

**Packaging & Platform**

* **Windows (portable, zip)**
* **Linux (Ubuntu/Debian, amd-64)**

### MDN 1.1.0

**Core / Numerics**

* Higher-base digit display now uses **alphanumeric symbols**; improves readability and matches common conventions.
  *Commit: “Fixed higher base alphanumeric display”*

**UX / Behavior**

* **Logging is off by default**; you can still enable it via config/CLI.
* Keyboard tweaks: `[+]` / `[-]` now target the line edit as expected.
* Sign convention indicator added to the status bar; clickable with an **F5** shortcut.

**Operations**

* Added user-facing control: *division iterations*
    * User chooses exponent *n*, where: nIters = 10ⁿ
    * *n* can be any integer [0 .. 7]
* Added user-facing control: *division direction*
    * Can be: **X**, **Y**, or **X←→Y**, latter is alternating
    * **CAUTION** alternating algorithm, **X←→Y** appears to be buggy.

**Carry over buttons**

* Fixed carryover buttons
* Changed behaviour of [-] and [+] buttons - now they start cell editing, was shortcut to fraxis setting.

**Sign Convention**

* Added sign convention to status bar
* Clicking sign convention button on status bar cycles its setting
* Added [F5] shortcut key for sign convention cycling

**Packaging & Platform**

* Added Linux .deb packaging (tested okay)
* Fixed Windows packaging

### Version 1.0.1

* Fixed Windows release packaging script to include vcredistributable.


**Packaging & Platform**

* **Linux .deb** packaging finalized (icons, desktop integration, headers under `/usr/include/mdn`).
* Windows portable bundle refined; Qt trimming options cleaned up.
* Misc readme/docs updates and icon set (16–512 px).

### Known issues

* If `mdn_gui` fails to start on Linux, ensure Qt6 runtime packages are present (they’re pulled automatically via `.deb` dependencies).
* On Windows, first launch may trigger a Defender/SmartScreen prompt due to the app being **unsigned**.

---

## License

See [LICENSE](license.md).
