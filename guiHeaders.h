#include "../library/Coord.h"
#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"


class DigitGridWidget : public QWidget {
    Q_OBJECT

public:
    DigitGridWidget(QWidget* parent = nullptr);
    void setMdn(std::shared_ptr<mdn::Mdn2d> mdn);
    void moveCursor(int dx, int dy);
    mdn::Coord getCursor() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    std::shared_ptr<mdn::Mdn2d> m_mdn;
    mdn::Coord m_cursor = mdn::Coord(0, 0);
    int m_cellSize = 20;
};
----- GuiDriver.h -----
#pragma once

#include <memory>

#include "MainWindow.h"
#include "Project.h"

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"

namespace mdn {

class GuiDriver {

    MainWindow m_mainWindow;
    std::shared_ptr<Project>* m_projectPtr;

};


} // end namespace mdn----- MainWindow.h -----
#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

#include "DigitGridWidget.h"

#include "../library/GlobalConfig.h"


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

signals:
    void newProjectRequested();
    void newMdn2dRequested();
    void openProjectRequested();
    void openMdn2dRequested();
    void saveProjectRequested();
    void saveMdn2dRequested();
    void closeProjectRequested();

private:
    void createMenus();
    void setupLayout();

    QSplitter* m_splitter = nullptr;

    // MDN Digit Browser (upper pane)
    QTabWidget* m_tabWidget = nullptr;

    // Command History (lower pane)
    QTextEdit* m_commandHistory = nullptr;
    QLineEdit* m_commandInput = nullptr;
    QPushButton* m_submitButton = nullptr;
    QPushButton* m_copyButton = nullptr;

    QMap<int, mdn::Mdn2d*> m_mdnMap;
    int m_nextMdnId = 0;

};
----- MdnMainWindow.h -----
#ifndef MDNMAINWINDOW_H
#define MDNMAINWINDOW_H

#include <QMainWindow>

#include "../library/GlobalConfig.h"

namespace Ui {
class MdnMainWindow;
}

class MDN_API MdnMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MdnMainWindow(QWidget *parent = nullptr);
    ~MdnMainWindow();

private:
    Ui::MdnMainWindow *ui;
};

#endif // MDNMAINWINDOW_H
----- MdnQtInterface.h -----
#pragma once

/*
    MdnQtInterface
    Provides conversion functionality between elemental type classes in Mdn and conceptually similar
    classes in QT
*/

#include <QString>
#include <QPoint>
#include <QRect>

#include "../library/Coord.h"
#include "../library/GlobalConfig.h"
#include "../library/Rect.h"


namespace mdn {

class MDN_API MdnQtInterface {
public:

    // *** Strings

        // Mdn std::string to QString
        static QString toQString(const std::string& s) {
            return QString::fromStdString(s);
        }

        // QString to Mdn std::string
        static std::string fromQString(const QString& qs) {
            return qs.toStdString();
        }


    // *** Coords

        // Mdn Coord to QPoint
        static QPoint toQPoint(const Coord& c) {
            return QPoint(c.x(), c.y());
        }

        // QPoint to Mdn Coord
        static Coord fromQPoint(const QPoint& p) {
            return Coord(p.x(), p.y());
        }


    // *** Rect

        // Mdn Rect to QRect
        static QRect toQRect(const Rect& r) {
            if (!r.isValid())
                return QRect(); // returns an invalid QRect

            Coord min = r.min();
            int width = r.width();   // includes +1 already
            int height = r.height(); // includes +1 already

            return QRect(min.x(), min.y(), width, height);
        }

        // QRect to Mdn Rect
        static Rect toRect(const QRect& q) {
            if (!q.isValid())
                return Rect::invalid();

            Coord min(q.left(), q.top());
            Coord max(q.right(), q.bottom()); // inclusive in MDN

            return Rect(min, max);
        }
};

} // end namespace mdn
----- NumberDisplayWidget.h -----
#pragma once

#include <qnamespace.h>
#include <QWidget>
// #include <QtGui/qcolor.h>
// #include <QColor>

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"

class NumberDisplayWidget : public QWidget {
    Q_OBJECT

public:
    NumberDisplayWidget(QWidget* parent = nullptr);

    void setModel(const mdn::Mdn2d* mdn);
    void setViewCenter(int x, int y);
    void moveCursor(int dx, int dy);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    const mdn::Mdn2d* m_model = nullptr;

    Qt::GlobalColor m_defaultColors_gridLines = Qt::gray;

    // X coordinate, in digit position, of top-left cell in view
    int m_viewOriginX = -16;

    // Y coordinate, in digit position, of top-left cell in view
    int m_viewOriginY = -16;

    // X digit coordinate of the cursor location
    int m_cursorX = 0;

    // Y digit coordinate of the cursor location
    int m_cursorY = 0;

    // Number of columns in view
    int m_cols = 32;

    // Number of rows in view
    int m_rows = 32;

    // Size of each cell in pixels
    int m_cellSize = 20;


};
----- OldMainWindow.h -----
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>

#include <QLabel>

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"

class MDN_API MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addNewMDN();
    void addValueToCurrent();
    void renameTab();
    void duplicateTab();
    void deleteTab();
    void onTabBarDoubleClicked(int index);
    void onTabContextMenuRequested(const QPoint &pos);

private:
    QTabWidget* m_tabWidget;
    QMap<int, mdn::Mdn2d*> m_mdnMap;
    int m_nextMdnId = 0;

    void createMDNTab(int id);
};

#endif // MAINWINDOW_H
----- Project.h -----
#pragma once

#include <vector>
#include <unordered_map>

// QT includes
#include <QMessageBox>

#include "MainWindow.h"

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"
#include "../library/Mdn2dFramework.h"

namespace mdn {

class Project: public Mdn2dFramework {

protected:

    // *** Protected member data

    // Appended to project name 'untitled' when no name provided
    static int m_untitledNumber;

    // The parent main window app in Qt
    MainWindow* m_parent;

    // Name of this project
    std::string m_name;

    // Config for all numbers in this project
    Mdn2dConfig m_config;

    // References to the constituent Mdn2d data, key is its tab position in the gui
    std::unordered_map<int, Mdn2d> m_data;

    // m_addressingNameToIndex[name] = index
    std::unordered_map<std::string, int> m_addressingNameToIndex;

    // m_addressingIndexToName[index] = name
    std::unordered_map<int, std::string> m_addressingIndexToName;

    // Selection, Rect bounds of digits, and list of MDNs coser;
    Selection m_selection;


    // *** Protected member functions

    // Shift Mdn tabs, starting at 'start', ending at 'end', shifting a distance of 'shift' tabs
    //  Exceptions
    //      * InvalidArgument - shift cannot be negative
    void shiftMdnTabsRight(int start, int end=-1, int shift=1);
    void shiftMdnTabsLeft(int start, int end=-1, int shift=1);
    // void shiftMdnTabsRight(int start, int shift);
    // void shiftMdnTabsLeft(int end, int shift);


public:

    // *** Constructors

    // Construct a null project' given its name and the number of empty Mdns to start with
    Project(MainWindow* parent=nullptr, std::string name="", int nStartMdn=3);


    // *** Mdn2dFramework API

        // Returns the framework's derived class type name as a string
        std::string className() const override {
            return "Project";
        }

        // Returns the framework's 'name', used in error messaging
        std::string name() const override {
            return m_name;
        }

        // Set the project name
        void setName(const std::string& nameIn) override {
            m_name = nameIn;
        }

        // Returns true if an Mdn2d exists with the given name, false otherwise
        bool mdnNameExists(const std::string& nameIn) const override {
            return m_addressingNameToIndex.find(nameIn) != m_addressingNameToIndex.cend();
        }

        // Gives framework final say over name changes - given a desired Mdn name change from
        //  origName to newName, returns the allowed name
        //  Messaging
        //      * Information: if newName already exists (i.e. failed to rename)
        //  Exceptions
        //      * InvalidArgument: if origName does not exist (i.e. no number to rename)
        std::string requestMdnNameChange(
            const std::string& origName,
            const std::string& newName
        ) override;


    // *** Project API

        // Accessors for m_config
        const Mdn2dConfig& config() const {
            return m_config;
        }

        // Setter for m_config requires resetting of the Mdn2d's
        void setConfig(Mdn2dConfig newConfig);


        // *** MDN Accessors

        // Checks if the Mdn2d exists in the main m_data array, returns:
        //  true  - number exists
        //  false - number does not exist
        //  false - addressing data bad
        // warnIfMissing, when true, issues a QMessageBox warning if the Mdn2d is missing
        bool Contains(std::string name, bool warnIfMissing = false) const;
        bool Contains(int i, bool warnIfMissing = false) const;

        // Return the index (tab position) for the Mdn of the given name, -1 = not found
        int IndexOfMdn(std::string name) const;

        // Return the name for the Mdn at the given tab index, empty string for bad index
        std::string NameOfMdn(int i) const;

        // Return pointer to the i'th Mdn tab, nullptr on failure
        //  e.g.:
        //      Mdn2d* src = GetMdn(fromIndex);
        //      AssertQ(src, "Failed to acquire Mdn2d from index " << fromIndex);
        const Mdn2d* GetMdn(int i) const;
        Mdn2d* GetMdn(int i);

        // Return pointer to the i'th Mdn tab, nullptr on failure
        const Mdn2d* GetMdn(std::string name) const;
        Mdn2d* GetMdn(std::string name);

        // Return pointer to Mdn2d at first tab, nullptr on failure
        const Mdn2d* FirstMdn() const;
        Mdn2d* FirstMdn();

        // Return pointer to Mdn2d at last tab, nullptr on failure
        const Mdn2d* LastMdn() const;
        Mdn2d* LastMdn();

        // Inserts a new number at the 'end', i.e. the last index
        void AppendMdn(Mdn2d& mdn) {
            InsertMdn(mdn, -1);
        }

        // Insert a new number at the given index, index == -1 means 'at the end'
        //  Messaging
        //      * Warning: if index is too big
        //          Recover: places number at the end
        //      * Warning: if number's name conflicts
        //          Recover: rename the new mdn, by convention
        void InsertMdn(Mdn2d& mdn, int index);

        // Duplicate the Mdn2d at the given index or given name, returns the name of the new mdn.
        //  An empty string return indicates the operation failed
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns empty string, takes no other action
        std::string DuplicateMdn(int index);
        std::string DuplicateMdn(const std::string& name);

        // Move Mdn2d at 'fromIndex' or given name to  'toIndex'; if toIndex == -1, moves it to the
        //  end.  Returns true or false based on success of operation.
        //  Messaging
        //      * Warning: if fromIndex out of range or name does not exist
        //          Recover: returns false
        //      * Warning: if toIndex is out of range
        //          Recover: places number at the end
        bool MoveMdn(int fromIndex, int toIndex);
        bool MoveMdn(const std::string& name, int toIndex);

        // Erase the Mdn2d at the given index or given name, shifting all higher entries one lower
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns false
        bool Erase(int index);
        bool Erase(const std::string& name);


    // Selection actions

        // Access current selection
        const Selection& selection() const { return m_selection; }

        // Set new selection
        void setSelection(Selection s) {
            m_selection = std::move(s);
            // TODO
            // emit signal, update UI
        }

        // Perform a 'copy' operation on the selection
        void CopySelection() const;

        // Perform a 'cut' operation on the selection - a combination of Copy and Delete
        void CutSelection();

        // Selection acts as anchor to paste operation
        void PasteOnSelection();

        // Perform 'delete' operation on the selection
        void DeleteSelection();

};


} // end namespace mdn----- Selection.h -----
#pragma once

#include <vector>

#include "../library/Rect.h"
#include "../library/Mdn2d.h"
#include "../library/MdnObserver.h"

namespace mdn {

class Selection : public MdnObserver {

public:

    // Coordinate bounds of selection, inclusive: [x0..x1] × [y0..y1]
    Rect rect;

    bool isEmpty() const {
        return get() == nullptr || !rect.isValid();
    }

    // Attach to an MDN
    void attach(Mdn2d* m) {
        if (m_ref == m) return;
        if (m_ref) m_ref->unregisterObserver(this);
        m_ref = m;
        if (m_ref) m_ref->registerObserver(this);
    }

    // Detatch from an MDN
    void detach() {
        if (m_ref) m_ref->unregisterObserver(this);
        m_ref = nullptr;
        rect.clear();
    }

    // Destructor
    ~Selection() { if (m_ref) m_ref->unregisterObserver(this); }


    // *** MdnObserver interface

    // The observed object is being destroyed
    void farewell() override {
        MdnObserver::farewell();
        rect.clear();
    }

    void reallocating(Mdn2d* newRef) override {
        MdnObserver::reallocating(newRef);
    }

};

} // end namespace mdn
----- DigitGridWidget.h -----
#include "../library/Coord.h"
#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"


class DigitGridWidget : public QWidget {
    Q_OBJECT

public:
    DigitGridWidget(QWidget* parent = nullptr);
    void setMdn(std::shared_ptr<mdn::Mdn2d> mdn);
    void moveCursor(int dx, int dy);
    mdn::Coord getCursor() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    std::shared_ptr<mdn::Mdn2d> m_mdn;
    mdn::Coord m_cursor = mdn::Coord(0, 0);
    int m_cellSize = 20;
};
----- GuiDriver.h -----
#pragma once

#include <memory>

#include "MainWindow.h"
#include "Project.h"

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"

namespace mdn {

class GuiDriver {

    MainWindow m_mainWindow;
    std::shared_ptr<Project>* m_projectPtr;

};


} // end namespace mdn----- MainWindow.h -----
#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

#include "DigitGridWidget.h"

#include "../library/GlobalConfig.h"


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

signals:
    void newProjectRequested();
    void newMdn2dRequested();
    void openProjectRequested();
    void openMdn2dRequested();
    void saveProjectRequested();
    void saveMdn2dRequested();
    void closeProjectRequested();

private:
    void createMenus();
    void setupLayout();

    QSplitter* m_splitter = nullptr;

    // MDN Digit Browser (upper pane)
    QTabWidget* m_tabWidget = nullptr;

    // Command History (lower pane)
    QTextEdit* m_commandHistory = nullptr;
    QLineEdit* m_commandInput = nullptr;
    QPushButton* m_submitButton = nullptr;
    QPushButton* m_copyButton = nullptr;

    QMap<int, mdn::Mdn2d*> m_mdnMap;
    int m_nextMdnId = 0;

};
----- MdnMainWindow.h -----
#ifndef MDNMAINWINDOW_H
#define MDNMAINWINDOW_H

#include <QMainWindow>

#include "../library/GlobalConfig.h"

namespace Ui {
class MdnMainWindow;
}

class MDN_API MdnMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MdnMainWindow(QWidget *parent = nullptr);
    ~MdnMainWindow();

private:
    Ui::MdnMainWindow *ui;
};

#endif // MDNMAINWINDOW_H
----- MdnQtInterface.h -----
#pragma once

/*
    MdnQtInterface
    Provides conversion functionality between elemental type classes in Mdn and conceptually similar
    classes in QT
*/

#include <QString>
#include <QPoint>
#include <QRect>

#include "../library/Coord.h"
#include "../library/Rect.h"


namespace mdn {

class MdnQtInterface {
public:

    // *** Strings

        // Mdn std::string to QString
        static QString toQString(const std::string& s) {
            return QString::fromStdString(s);
        }

        // QString to Mdn std::string
        static std::string fromQString(const QString& qs) {
            return qs.toStdString();
        }


    // *** Coords

        // Mdn Coord to QPoint
        static QPoint toQPoint(const Coord& c) {
            return QPoint(c.x(), c.y());
        }

        // QPoint to Mdn Coord
        static Coord fromQPoint(const QPoint& p) {
            return Coord(p.x(), p.y());
        }


    // *** Rect

        // Mdn Rect to QRect
        static QRect toQRect(const Rect& r) {
            if (!r.isValid())
                return QRect(); // returns an invalid QRect

            Coord min = r.min();
            int width = r.width();   // includes +1 already
            int height = r.height(); // includes +1 already

            return QRect(min.x(), min.y(), width, height);
        }

        // QRect to Mdn Rect
        static Rect toRect(const QRect& q) {
            if (!q.isValid())
                return Rect::GetInvalid();

            Coord min(q.left(), q.top());
            Coord max(q.right(), q.bottom()); // inclusive in MDN

            return Rect(min, max);
        }
};

} // end namespace mdn
----- NumberDisplayWidget.h -----
#pragma once

#include <qnamespace.h>
#include <QWidget>
// #include <QtGui/qcolor.h>
// #include <QColor>

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"

class NumberDisplayWidget : public QWidget {
    Q_OBJECT

public:
    NumberDisplayWidget(QWidget* parent = nullptr);

    void setModel(const mdn::Mdn2d* mdn);
    void setViewCenter(int x, int y);
    void moveCursor(int dx, int dy);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    const mdn::Mdn2d* m_model = nullptr;

    Qt::GlobalColor m_defaultColors_gridLines = Qt::gray;

    // X coordinate, in digit position, of top-left cell in view
    int m_viewOriginX = -16;

    // Y coordinate, in digit position, of top-left cell in view
    int m_viewOriginY = -16;

    // X digit coordinate of the cursor location
    int m_cursorX = 0;

    // Y digit coordinate of the cursor location
    int m_cursorY = 0;

    // Number of columns in view
    int m_cols = 32;

    // Number of rows in view
    int m_rows = 32;

    // Size of each cell in pixels
    int m_cellSize = 20;


};
----- OldMainWindow.h -----
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>

#include <QLabel>

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"

class MDN_API MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addNewMDN();
    void addValueToCurrent();
    void renameTab();
    void duplicateTab();
    void deleteTab();
    void onTabBarDoubleClicked(int index);
    void onTabContextMenuRequested(const QPoint &pos);

private:
    QTabWidget* m_tabWidget;
    QMap<int, mdn::Mdn2d*> m_mdnMap;
    int m_nextMdnId = 0;

    void createMDNTab(int id);
};

#endif // MAINWINDOW_H
----- Project.h -----
#pragma once

#include <vector>
#include <unordered_map>

// QT includes
#include <QMessageBox>

#include "MainWindow.h"

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"
#include "../library/Mdn2dFramework.h"

namespace mdn {

class Project: public Mdn2dFramework {

protected:

    // *** Protected member data

    // Appended to project name 'untitled' when no name provided
    static int m_untitledNumber;

    // The parent main window app in Qt
    MainWindow* m_parent;

    // Name of this project
    std::string m_name;

    // Config for all numbers in this project
    Mdn2dConfig m_config;

    // References to the constituent Mdn2d data, key is its tab position in the gui
    std::unordered_map<int, Mdn2d> m_data;

    // m_addressingNameToIndex[name] = index
    std::unordered_map<std::string, int> m_addressingNameToIndex;

    // m_addressingIndexToName[index] = name
    std::unordered_map<int, std::string> m_addressingIndexToName;

    // Selection, Rect bounds of digits, and list of MDNs coser;
    Selection m_selection;


    // *** Protected member functions

    // Shift Mdn tabs, starting at 'start', ending at 'end', shifting a distance of 'shift' tabs
    //  Exceptions
    //      * InvalidArgument - shift cannot be negative
    void shiftMdnTabsRight(int start, int end=-1, int shift=1);
    void shiftMdnTabsLeft(int start, int end=-1, int shift=1);
    // void shiftMdnTabsRight(int start, int shift);
    // void shiftMdnTabsLeft(int end, int shift);


public:

    // *** Constructors

    // Construct a null project' given its name and the number of empty Mdns to start with
    Project(MainWindow* parent=nullptr, std::string name="", int nStartMdn=3);


    // *** Mdn2dFramework API

        // Returns the framework's derived class type name as a string
        std::string className() const override {
            return "Project";
        }

        // Returns the framework's 'name', used in error messaging
        std::string name() const override {
            return m_name;
        }

        // Set the project name
        void setName(const std::string& nameIn) override {
            m_name = nameIn;
        }

        // Returns true if an Mdn2d exists with the given name, false otherwise
        bool mdnNameExists(const std::string& nameIn) const override {
            return m_addressingNameToIndex.find(nameIn) != m_addressingNameToIndex.cend();
        }

        // Gives framework final say over name changes - given a desired Mdn name change from
        //  origName to newName, returns the allowed name
        //  Messaging
        //      * Information: if newName already exists (i.e. failed to rename)
        //  Exceptions
        //      * InvalidArgument: if origName does not exist (i.e. no number to rename)
        std::string requestMdnNameChange(
            const std::string& origName,
            const std::string& newName
        ) override;


    // *** Project API

        // Accessors for m_config
        const Mdn2dConfig& config() const {
            return m_config;
        }

        // Setter for m_config requires resetting of the Mdn2d's
        void setConfig(Mdn2dConfig newConfig);


        // *** MDN Accessors

        // Checks if the Mdn2d exists in the main m_data array, returns:
        //  true  - number exists
        //  false - number does not exist
        //  false - addressing data bad
        // warnOnFailure, when true, issues a QMessageBox warning if the Mdn2d is missing
        bool contains(std::string name, bool warnOnFailure = false) const;
        bool contains(int i, bool warnOnFailure = false) const;

        // Return the index (tab position) for the Mdn of the given name, -1 = not found
        int indexOfMdn(std::string name) const;

        // Return the name for the Mdn at the given tab index, empty string for bad index
        std::string nameOfMdn(int i) const;

        // Return pointer to the i'th Mdn tab, nullptr on failure
        //  e.g.:
        //      Mdn2d* src = getMdn(fromIndex);
        //      AssertQ(src, "Failed to acquire Mdn2d from index " << fromIndex);
        const Mdn2d* getMdn(int i, bool warnOnFailure=false) const;
        Mdn2d* getMdn(int i, bool warnOnFailure=false);

        // Return pointer to the Mdn tab with the given name, nullptr on failure
        const Mdn2d* getMdn(std::string name, bool warnOnFailure=false) const;
        Mdn2d* getMdn(std::string name, bool warnOnFailure=false);

        // Return pointer to Mdn2d at first tab, nullptr on failure
        const Mdn2d* firstMdn(bool warnOnFailure=false) const;
        Mdn2d* firstMdn(bool warnOnFailure=false);

        // Return pointer to Mdn2d at last tab, nullptr on failure
        const Mdn2d* lastMdn(bool warnOnFailure=false) const;
        Mdn2d* lastMdn(bool warnOnFailure=false);

        // Inserts a new number at the 'end', i.e. the last index
        void appendMdn(Mdn2d& mdn) {
            insertMdn(mdn, -1);
        }

        // Insert a new number at the given index, index == -1 means 'at the end'
        //  Messaging
        //      * Warning: if index is too big
        //          Recover: places number at the end
        //      * Warning: if number's name conflicts
        //          Recover: rename the new mdn, by convention
        void insertMdn(Mdn2d& mdn, int index);

        // Duplicate the Mdn2d at the given index or given name, returns the name of the new mdn.
        //  An empty string return indicates the operation failed
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns empty string, takes no other action
        std::string duplicateMdn(int index);
        std::string duplicateMdn(const std::string& name);

        // Move Mdn2d at 'fromIndex' or given name to  'toIndex'; if toIndex == -1, moves it to the
        //  end.  Returns true or false based on success of operation.
        //  Messaging
        //      * Warning: if fromIndex out of range or name does not exist
        //          Recover: returns false
        //      * Warning: if toIndex is out of range
        //          Recover: places number at the end
        bool moveMdn(int fromIndex, int toIndex);
        bool moveMdn(const std::string& name, int toIndex);

        // eraseMdn the Mdn2d at the given index or given name, shifting all higher entries one lower
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns false
        bool eraseMdn(int index);
        bool eraseMdn(const std::string& name);


    // Selection actions

        // Access current selection
        const Selection& selection() const { return m_selection; }

        // Set new selection
        void setSelection(Selection s) {
            m_selection = std::move(s);
            // TODO
            // emit signal, update UI
        }

        // Perform a 'copy' operation on the selection
        void copySelection() const;

        // Perform a 'cut' operation on the selection - a combination of Copy and Delete
        void cutSelection();

        // Perform a paste operation, using clipboard data and the target:, where the target depends
        //  on the supplied index:
        //      if index < 0 (or missing), use m_selection for target
        //      if index >= 0, target is Mdn tab at given index
        //  Always overwrite destination, rect is anchored to the bottom-left (xmin, ymin())
        //
        //  Source scope (data on the clipboard)
        //  A) "mdn"  - defines an entire Mdn for pasting
        //  B) "rect" - defines a specific area on a specific Mdn
        //
        //  Destination scope (data currently selected, m_selection)
        //  1. selection.hasMdnOnly    - target is the entire Mdn, (index >= 0)
        //  2. selection.hasRectOnly   - invalid - need a Mdn for actual operation
        //  3. selection.hasMdnAndRect - target is the specific area on the selected Mdn
        //
        //  A-1 - Mdn ->  Mdn       - replace entire target Mdn with source Mdn
        //  A-2 - Mdn ->  Rect      - Not valid (app error)
        //  A-3 - Mdn ->  Mdn+Rect  - Not valid (user error - tell user invalid data to paste here)
        //  B-1 - Rect -> Mdn       - replace same rect (absolute) on target with source
        //  B-2 - Rect -> Rect      - Not valid (app error)
        //  B-3 - Rect -> Mdn+Rect  - replace same rect (relative) on target with source, size check
        //      required: if target is 1x1, paste okay, use that as bottom left anchor, otherwise
        //      the size must match exactly
        void pasteOnSelection(int index=-1);

        // Perform 'delete' operation on the selection
        void deleteSelection();

};


} // end namespace mdn----- Selection.h -----
#pragma once

#include <vector>

#include "../library/Rect.h"
#include "../library/Mdn2d.h"
#include "../library/MdnObserver.h"

namespace mdn {

class Selection : public MdnObserver {

public:

    // Coordinate bounds of selection, inclusive: [x0..x1] × [y0..y1]
    Rect rect;


    // * Selection queries

        // True if the selection includes a valid Mdn
        bool hasMdn() const {
            return get() != nullptr;
        }

        // True if the selected rectangular area is valid
        bool hasRect() const {
            return rect.isValid();
        }

        // True if an Mdn is selected, but there's no rectangular area
        bool hasMdnOnly() const {
            return hasMdn() && !hasRect();
        }

        // True if a rectangular area is selected, but not on a valid Mdn
        bool hasRectOnly() const {
            return !hasMdn() && hasRect();
        }

        // True if selection contains a valid rect and a valid Mdn
        bool hasMdnAndRect() const {
            return hasRect() && hasMdn();
        }

        // True if no Mdn is selected, and no rectangular area is valid
        bool isEmpty() const {
            return !hasMdn() && !hasRect();
        }


    // * Selection operations

        // Attach to an MDN
        void attach(Mdn2d* m) {
            if (m_ref == m) return;
            if (m_ref) m_ref->unregisterObserver(this);
            m_ref = m;
            if (m_ref) m_ref->registerObserver(this);
        }

        // Detatch from an MDN
        void detach() {
            if (m_ref) m_ref->unregisterObserver(this);
            m_ref = nullptr;
            rect.clear();
        }

        // Destructor
        ~Selection() { if (m_ref) m_ref->unregisterObserver(this); }


    // *** MdnObserver interface

    // The observed object is being destroyed
    void farewell() override {
        MdnObserver::farewell();
        rect.clear();
    }

    void reallocating(Mdn2d* newRef) override {
        MdnObserver::reallocating(newRef);
    }

};

} // end namespace mdn
----- DigitGridWidget.h -----
#include "../library/Coord.h"
#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"


class DigitGridWidget : public QWidget {
    Q_OBJECT

public:
    DigitGridWidget(QWidget* parent = nullptr);
    void setMdn(std::shared_ptr<mdn::Mdn2d> mdn);
    void moveCursor(int dx, int dy);
    mdn::Coord getCursor() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    std::shared_ptr<mdn::Mdn2d> m_mdn;
    mdn::Coord m_cursor = mdn::Coord(0, 0);
    int m_cellSize = 20;
};
----- GuiDriver.h -----
#pragma once

#include <memory>

#include "MainWindow.h"
#include "Project.h"

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"

namespace mdn {

class GuiDriver {

    MainWindow m_mainWindow;
    std::shared_ptr<Project>* m_projectPtr;

};


} // end namespace mdn----- MainWindow.h -----
#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

#include "DigitGridWidget.h"

#include "../library/GlobalConfig.h"


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

signals:
    void newProjectRequested();
    void newMdn2dRequested();
    void openProjectRequested();
    void openMdn2dRequested();
    void saveProjectRequested();
    void saveMdn2dRequested();
    void closeProjectRequested();

private:
    void createMenus();
    void setupLayout();

    QSplitter* m_splitter = nullptr;

    // MDN Digit Browser (upper pane)
    QTabWidget* m_tabWidget = nullptr;

    // Command History (lower pane)
    QTextEdit* m_commandHistory = nullptr;
    QLineEdit* m_commandInput = nullptr;
    QPushButton* m_submitButton = nullptr;
    QPushButton* m_copyButton = nullptr;

    QMap<int, mdn::Mdn2d*> m_mdnMap;
    int m_nextMdnId = 0;

};
----- MdnMainWindow.h -----
#ifndef MDNMAINWINDOW_H
#define MDNMAINWINDOW_H

#include <QMainWindow>

#include "../library/GlobalConfig.h"

namespace Ui {
class MdnMainWindow;
}

class MDN_API MdnMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MdnMainWindow(QWidget *parent = nullptr);
    ~MdnMainWindow();

private:
    Ui::MdnMainWindow *ui;
};

#endif // MDNMAINWINDOW_H
----- MdnQtInterface.h -----
#pragma once

/*
    MdnQtInterface
    Provides conversion functionality between elemental type classes in Mdn and conceptually similar
    classes in QT
*/

#include <QString>
#include <QPoint>
#include <QRect>

#include "../library/Coord.h"
#include "../library/Rect.h"


namespace mdn {

class MdnQtInterface {
public:

    // *** Strings

        // Mdn std::string to QString
        static QString toQString(const std::string& s) {
            return QString::fromStdString(s);
        }

        // QString to Mdn std::string
        static std::string fromQString(const QString& qs) {
            return qs.toStdString();
        }


    // *** Coords

        // Mdn Coord to QPoint
        static QPoint toQPoint(const Coord& c) {
            return QPoint(c.x(), c.y());
        }

        // QPoint to Mdn Coord
        static Coord fromQPoint(const QPoint& p) {
            return Coord(p.x(), p.y());
        }


    // *** Rect

        // Mdn Rect to QRect
        static QRect toQRect(const Rect& r) {
            if (!r.isValid())
                return QRect(); // returns an invalid QRect

            Coord min = r.min();
            int width = r.width();   // includes +1 already
            int height = r.height(); // includes +1 already

            return QRect(min.x(), min.y(), width, height);
        }

        // QRect to Mdn Rect
        static Rect toRect(const QRect& q) {
            if (!q.isValid())
                return Rect::GetInvalid();

            Coord min(q.left(), q.top());
            Coord max(q.right(), q.bottom()); // inclusive in MDN

            return Rect(min, max);
        }
};

} // end namespace mdn
----- NumberDisplayWidget.h -----
#pragma once

#include <qnamespace.h>
#include <QWidget>
// #include <QtGui/qcolor.h>
// #include <QColor>

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"

class NumberDisplayWidget : public QWidget {
    Q_OBJECT

public:
    NumberDisplayWidget(QWidget* parent = nullptr);

    void setModel(const mdn::Mdn2d* mdn);
    void setViewCenter(int x, int y);
    void moveCursor(int dx, int dy);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    const mdn::Mdn2d* m_model = nullptr;

    Qt::GlobalColor m_defaultColors_gridLines = Qt::gray;

    // X coordinate, in digit position, of top-left cell in view
    int m_viewOriginX = -16;

    // Y coordinate, in digit position, of top-left cell in view
    int m_viewOriginY = -16;

    // X digit coordinate of the cursor location
    int m_cursorX = 0;

    // Y digit coordinate of the cursor location
    int m_cursorY = 0;

    // Number of columns in view
    int m_cols = 32;

    // Number of rows in view
    int m_rows = 32;

    // Size of each cell in pixels
    int m_cellSize = 20;


};
----- OldMainWindow.h -----
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>

#include <QLabel>

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"

class MDN_API MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addNewMDN();
    void addValueToCurrent();
    void renameTab();
    void duplicateTab();
    void deleteTab();
    void onTabBarDoubleClicked(int index);
    void onTabContextMenuRequested(const QPoint &pos);

private:
    QTabWidget* m_tabWidget;
    QMap<int, mdn::Mdn2d*> m_mdnMap;
    int m_nextMdnId = 0;

    void createMDNTab(int id);
};

#endif // MAINWINDOW_H
----- Project.h -----
#pragma once

#include <vector>
#include <unordered_map>

// QT includes
#include <QMessageBox>

#include "MainWindow.h"

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"
#include "../library/Mdn2dFramework.h"

namespace mdn {

class Project: public Mdn2dFramework {

protected:

    // *** Protected member data

    // Appended to project name 'untitled' when no name provided
    static int m_untitledNumber;

    // The parent main window app in Qt
    MainWindow* m_parent;

    // Name of this project
    std::string m_name;

    // Config for all numbers in this project
    Mdn2dConfig m_config;

    // References to the constituent Mdn2d data, key is its tab position in the gui
    std::unordered_map<int, Mdn2d> m_data;

    // m_addressingNameToIndex[name] = index
    std::unordered_map<std::string, int> m_addressingNameToIndex;

    // m_addressingIndexToName[index] = name
    std::unordered_map<int, std::string> m_addressingIndexToName;

    // Selection, Rect bounds of digits, and list of MDNs coser;
    Selection m_selection;


    // *** Protected member functions

    // Shift Mdn tabs, starting at 'start', ending at 'end', shifting a distance of 'shift' tabs
    //  Exceptions
    //      * InvalidArgument - shift cannot be negative
    void shiftMdnTabsRight(int start, int end=-1, int shift=1);
    void shiftMdnTabsLeft(int start, int end=-1, int shift=1);
    // void shiftMdnTabsRight(int start, int shift);
    // void shiftMdnTabsLeft(int end, int shift);


public:

    // *** Constructors

    // Construct a null project' given its name and the number of empty Mdns to start with
    Project(MainWindow* parent=nullptr, std::string name="", int nStartMdn=3);


    // *** Mdn2dFramework API

        // Returns the framework's derived class type name as a string
        std::string className() const override {
            return "Project";
        }

        // Returns the framework's 'name', used in error messaging
        std::string name() const override {
            return m_name;
        }

        // Set the project name
        void setName(const std::string& nameIn) override {
            m_name = nameIn;
        }

        // Returns true if an Mdn2d exists with the given name, false otherwise
        bool mdnNameExists(const std::string& nameIn) const override {
            return m_addressingNameToIndex.find(nameIn) != m_addressingNameToIndex.cend();
        }

        // Gives framework final say over name changes - given a desired Mdn name change from
        //  origName to newName, returns the allowed name
        //  Messaging
        //      * Information: if newName already exists (i.e. failed to rename)
        //  Exceptions
        //      * InvalidArgument: if origName does not exist (i.e. no number to rename)
        std::string requestMdnNameChange(
            const std::string& origName,
            const std::string& newName
        ) override;


    // *** Project API

        // Accessors for m_config
        const Mdn2dConfig& config() const {
            return m_config;
        }

        // Setter for m_config requires resetting of the Mdn2d's
        void setConfig(Mdn2dConfig newConfig);


        // *** MDN Accessors

        // Checks if the Mdn2d exists in the main m_data array, returns:
        //  true  - number exists
        //  false - number does not exist
        //  false - addressing data bad
        // warnOnFailure, when true, issues a QMessageBox warning if the Mdn2d is missing
        bool contains(std::string name, bool warnOnFailure = false) const;
        bool contains(int i, bool warnOnFailure = false) const;

        // Return the index (tab position) for the Mdn of the given name, -1 = not found
        int indexOfMdn(std::string name) const;

        // Return the name for the Mdn at the given tab index, empty string for bad index
        std::string nameOfMdn(int i) const;

        // Return pointer to the i'th Mdn tab, nullptr on failure
        //  e.g.:
        //      Mdn2d* src = getMdn(fromIndex);
        //      AssertQ(src, "Failed to acquire Mdn2d from index " << fromIndex);
        const Mdn2d* getMdn(int i, bool warnOnFailure=false) const;
        Mdn2d* getMdn(int i, bool warnOnFailure=false);

        // Return pointer to the Mdn tab with the given name, nullptr on failure
        const Mdn2d* getMdn(std::string name, bool warnOnFailure=false) const;
        Mdn2d* getMdn(std::string name, bool warnOnFailure=false);

        // Return pointer to Mdn2d at first tab, nullptr on failure
        const Mdn2d* firstMdn(bool warnOnFailure=false) const;
        Mdn2d* firstMdn(bool warnOnFailure=false);

        // Return pointer to Mdn2d at last tab, nullptr on failure
        const Mdn2d* lastMdn(bool warnOnFailure=false) const;
        Mdn2d* lastMdn(bool warnOnFailure=false);

        // Inserts a new number at the 'end', i.e. the last index
        void appendMdn(Mdn2d& mdn) {
            insertMdn(mdn, -1);
        }

        // Insert a new number at the given index, index == -1 means 'at the end'
        //  Messaging
        //      * Warning: if index is too big
        //          Recover: places number at the end
        //      * Warning: if number's name conflicts
        //          Recover: rename the new mdn, by convention
        void insertMdn(Mdn2d& mdn, int index);

        // Duplicate the Mdn2d at the given index or given name, returns the name of the new mdn.
        //  An empty string return indicates the operation failed
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns empty string, takes no other action
        std::string duplicateMdn(int index);
        std::string duplicateMdn(const std::string& name);

        // Move Mdn2d at 'fromIndex' or given name to  'toIndex'; if toIndex == -1, moves it to the
        //  end.  Returns true or false based on success of operation.
        //  Messaging
        //      * Warning: if fromIndex out of range or name does not exist
        //          Recover: returns false
        //      * Warning: if toIndex is out of range
        //          Recover: places number at the end
        bool moveMdn(int fromIndex, int toIndex);
        bool moveMdn(const std::string& name, int toIndex);

        // eraseMdn the Mdn2d at the given index or given name, shifting all higher entries one lower
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns false
        bool eraseMdn(int index);
        bool eraseMdn(const std::string& name);


    // Selection actions

        // Access current selection
        const Selection& selection() const { return m_selection; }

        // Set new selection
        void setSelection(Selection s) {
            m_selection = std::move(s);
            // TODO
            // emit signal, update UI
        }

        // Perform a 'copy' operation on the selection
        void copySelection() const;

        // Perform a 'cut' operation on the selection - a combination of Copy and Delete
        void cutSelection();

        // Perform a paste operation, using clipboard data and the target:, where the target depends
        //  on the supplied index:
        //      if index < 0 (or missing), use m_selection for target
        //      if index >= 0, target is Mdn tab at given index
        //  Always overwrite destination, rect is anchored to the bottom-left (xmin, ymin())
        //
        //  Source scope (data on the clipboard)
        //  A) "mdn"  - defines an entire Mdn for pasting
        //  B) "rect" - defines a specific area on a specific Mdn
        //
        //  Destination scope (data currently selected, m_selection)
        //  1. selection.hasMdnOnly    - target is the entire Mdn, (index >= 0)
        //  2. selection.hasRectOnly   - invalid - need a Mdn for actual operation
        //  3. selection.hasMdnAndRect - target is the specific area on the selected Mdn
        //
        //  A-1 - Mdn ->  Mdn       - replace entire target Mdn with source Mdn
        //  A-2 - Mdn ->  Rect      - Not valid (app error)
        //  A-3 - Mdn ->  Mdn+Rect  - Not valid (user error - tell user invalid data to paste here)
        //  B-1 - Rect -> Mdn       - replace same rect (absolute) on target with source
        //  B-2 - Rect -> Rect      - Not valid (app error)
        //  B-3 - Rect -> Mdn+Rect  - replace same rect (relative) on target with source, size check
        //      required: if target is 1x1, paste okay, use that as bottom left anchor, otherwise
        //      the size must match exactly
        void pasteOnSelection(int index=-1);

        // Perform 'delete' operation on the selection
        void deleteSelection();

};


} // end namespace mdn----- Selection.h -----
#pragma once

#include <vector>

#include "../library/Rect.h"
#include "../library/Mdn2d.h"
#include "../library/MdnObserver.h"

namespace mdn {

class Selection : public MdnObserver {

public:

    // Coordinate bounds of selection, inclusive: [x0..x1] × [y0..y1]
    Rect rect;


    // * Selection queries

        // True if the selection includes a valid Mdn
        bool hasMdn() const {
            return get() != nullptr;
        }

        // True if the selected rectangular area is valid
        bool hasRect() const {
            return rect.isValid();
        }

        // True if an Mdn is selected, but there's no rectangular area
        bool hasMdnOnly() const {
            return hasMdn() && !hasRect();
        }

        // True if a rectangular area is selected, but not on a valid Mdn
        bool hasRectOnly() const {
            return !hasMdn() && hasRect();
        }

        // True if selection contains a valid rect and a valid Mdn
        bool hasMdnAndRect() const {
            return hasRect() && hasMdn();
        }

        // True if no Mdn is selected, and no rectangular area is valid
        bool isEmpty() const {
            return !hasMdn() && !hasRect();
        }


    // * Selection operations

        // Attach to an MDN
        void attach(Mdn2d* m) {
            if (m_ref == m) return;
            if (m_ref) m_ref->unregisterObserver(this);
            m_ref = m;
            if (m_ref) m_ref->registerObserver(this);
        }

        // Detatch from an MDN
        void detach() {
            if (m_ref) m_ref->unregisterObserver(this);
            m_ref = nullptr;
            rect.clear();
        }

        // Destructor
        ~Selection() { if (m_ref) m_ref->unregisterObserver(this); }


    // *** MdnObserver interface

    // The observed object is being destroyed
    void farewell() override {
        MdnObserver::farewell();
        rect.clear();
    }

    void reallocating(Mdn2d* newRef) override {
        MdnObserver::reallocating(newRef);
    }

};

} // end namespace mdn
