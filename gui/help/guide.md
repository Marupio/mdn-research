## User's Guide

### What is this app?

This app is a calculator for **Multi-Dimenional Numbers (MDN)**.

### Running the app

Find the executable:

* **Windows** the executable is in the unzipped folder, **mdn_gui.exe**
* **Linux** the executable is **mdn_gui**, by default it installs in **/usr/bin**

#### Command Line Options

Command line options include:

* *-h, --help* - Displays help on commandline options.
* *-v, --version* - Displays version information.
* *-l, --log-level <level>* - Set log verbosity. One of:
    * Debug4
    * Debug3
    * Debug2
    * Debug
    * Info
    * Warning
    * Error
* *-f, --log-file <path>* - Write logs to the given file path (creates/overwrites).
* *--no-log-file* - Do not write logs to a file (overrides any defaults).
* *--check-indents* - Monitor debug logging for missing indentation calls
* *-i, --input <path>* - Path to JSON settings file.
* *--info* - Display application information

### Layout

The speadsheet shows all the digits of a single MDN.  You can switch between MDNs using the **tabs** at the bottom.

### What are the Tabs?

Each tab is an MDN.  Tabs can be created, destroyed, moved, renamed, saved, and loaded.

* _Right-click_ on a tab to open the context menu:
    * **Rename**, **Duplicate**, **Copy**, **Paste**, **Save as...**, **Delete**
* _Clicking_ the **[+] Plus tab** creates a new MDN.
* On the right side of all the **Tabs** is the **MDN toolbar**, with buttons for:
    * **Save tab**, **Open tab**, **Close tab**

### How do I fill in an MDN?

The spreadsheet shows all the digits of the MDN.  Fill these in as you want using the cursor.  When you start typing a number, your cursor turns into an edit field. You can specify as many digits as you want.  If it's too big to fit in one digit, the app will spread it out as necessary, following the rules of MDN:
* **Integer parts** expand symmetrically along _x+_ and _y+_
* **Fractional parts** expand along only one axis of your choice, _x-_ or _y-_, but also exhibit a **fraxis cascade** diagonally away from the axis.

#### Cursor modes

There are three cursor modes.  Press **[F3]** to change mode, or click the indicator in the _status bar_.

* **OVER** - overwrite mode - all digits touched by your input get replaced.
* **ADD +** - addition mode - your inputs add to existing digits.
* **SUB -** - subtraction mode - your inputs subtract from existing digits.

#### Fraxis modes

When entering a _fractional amount_, the app will perform a **fraxis cascade**.  The fraction amount only appears on one axis, and the cascade expands diagonally away from the axis.  There are two **fraxis modes** to choose from:

* **X** fraxis mode - fractional amount appears in the _x_ direcion, and
* **Y** fraxis mode - fractional amount appears in the _y_ direction.

Press **[F4]** to change **fraxis mode**, or click on the indicator in the _status bar_.

#### Sign convention

Sign convention is for polymorphic numbers.  The sign is the desired state of the root digit, or pivot digit.

```text
 0 │-3 0 0      0 │-4 0 0
 0 │-2 6 0   =  0 │ 8 5 0
 ──┼───────     ──┼───────
```
**Polymorphic number**

In this example, the left side shows a *negative* polymorphic state, where the *-2* is the pivot digit.  The right side shows a *positive* polymorphic state, where the *8* is the pivot digit.  Both are different forms of the same number.

Press **[F5]** to change **sign convention**, or click on the indicator in the _status bar_.

### What are unary operations?

_Operations that act on a single MDN alone._  There are three unary operations:

* **Transpose** - swap _x_ and _y_.  Click on the **[T↔]** button to perform a transpose operation.
* **Carry-positive** - perform carry-over operations on all _negative polymorphic digits_, turning them positive.  Click on the **[c/+]** button to perform this.
* **Carry-negative** - perform carry-over operations on all _positive polymorphic digits_, turning them negative.  Click on the **[c/-]** button to perform this.
> **What the heck are polymorphic digits?**
>
> Digits with a mixture of positive and negative have two valid states of carry-over.  They are named for the sign of their root digit, or pivot digit.

### How do I perform normal math operations (or 'what are binary operations) ?

There are 4 main binary operations:

* **[ + ] Addition**
* **[ - ] Subtraction**
* **[ x ] Multiplication**
* **[ ÷ ] Division**

You can perform these operations in two ways:

* **From the menu** _Operations_→_[Operation name...]_
    * Specify the operands and destination in the **Binary Operation** Window
* **From the toolbar** _Operation buttons are near the bottom of the main window_.
    * Specify the operands interactively by clicking on tabs.

#### Binary Operation window

>The **Binary Operation** window is the boring way to do it.

The displayed **MDN** is _operand A_.  Choose _operand B_ from the **Choose B** section of the window.  The **Answer** section of the window lets you choose where to put the answer.  If you choose to overwrite an existing **MDN**, that operation cannot be undone.

#### Operation toolbar

> The exciting way to do it.

Select the **MDN** you want to start with.  That is _operand A_.  Click on the operation button you want, such as **[+]**.  The status message area will tell you what to do next:

> Mdn0 + * Operand B * >> Choose tab << Hover to peek

Next choose _operand B_ by clicking on the next tab.  It can be the same as _operand A_.  If you aren't sure, hover your pointer over the tab and the app will show you what that **MDN** contains.

> Mdn0 + Mdn2 → * Destination * >> Choose tab << Hover to peek

Finally, click where you want the answer to go.  If you click the **[+]** tab, the app will create a new Mdn named something like **Mdn0+Mdn2**.

### Division is different from the other operations

The division algorithm requires a little more user input:

* The user has to choose a **Remainder** in addition to the **Operand B** and **Answer** tabs.
* Beside the **[÷] Division** button is **10^[0]** division iterations spin box.  Choose between [0..7] for the number of iterations:
    * 0 = 1 iteration
    * 1 = 10 iterations
    * 2 = 100 iterations
    * and so on, until 7 = 10,000,000 iterations
* Division will run for this many iterations, and halt, allowing the user to inspect the working answer and remainder.
* The user can click the **[÷] Division** button to continue refining the answer.
* The user can click **[Cancel]** to accept the current state of the answer.
* The user can also select the **Direction** of the division:
    * **X direction** means the algorithm works with rows of numbers,
    * **Y direction** means the algorithm works with columns of numbers, and
    * **X/Y direction** means it will alternate each iteration.

This division algorithm is a sort of implementation of *long division*, but in two dimensions.

#### Iterative ####

The algorithm is _iterative_.  The app will try **10 iterations** first.  If it ever gets the exact answer (**Remainder** = 0), it will stop.  Otherwise it offers the user a chance to inspect the **Remainder** and **Answer**.  The user can click the **[÷] Division** button to run another 10 iterations.

#### Choose Remainder ####

Whereas **[+] Addition**, **[-] Subtraction**, and **[x] Multiplication** all require the user to select **Operand B** and **Destination** , **[÷] Division** also requires **Remainder**.

Choose an existing **Mdn** tab to overwrite or choose the **[+]** tab to create a new tab.  The app will create a new tab named something like **rem(Mdn0÷Mdn2)**.
