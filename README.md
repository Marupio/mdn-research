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

### Version 1.1.0

#### Division algorithm

* Added user-facing control: *division iterations*
    * User chooses exponent: nIters = 10ⁿ
    * Can be any integer [0 .. 7]
* Added user-facing control: *division direction*
    * Can be: X, Y, or X←→Y, latter is alternating

#### Carry over buttons

* Fixed carryover buttons
* Changed behaviour of [-] and [+] buttons - now they start cell editing, was shortcut to fraxis setting.

#### Sign Convention

* Added sign convention to status bar
* Clicking sign convention button on status bar cycles its setting
* Added [F5] shortcut key for sign convention cycling

#### Build system

* Added Linux .deb packaging (tested okay)
* Fixed Windows packaging

### Version 1.0.1

* Fixed Windows release packaging script to include vcredistributable.


---

## License

See [LICENSE](license.md).
