# FolderDir

**FolderDir** is a multi-pane file manager written in C++17 with Qt, inspired by [Q-Dir](https://www.softwareok.com/?seite=Freeware/Q-Dir).  
It lets you view up to **four directories simultaneously** in a resizable split-pane layout, with tabs, bookmarks, file operations, and more.

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| **Multi-pane layout** | 1 / 2 / 3 / 4 panes, resizable splitters (2×2 grid by default) |
| **Tabs per pane** | Each pane supports unlimited tabs; Ctrl+T / Ctrl+W to open/close |
| **Address bar** | Editable path with validation; navigate by typing or pressing Enter |
| **Drive bar** | One-click jump to any mounted drive/volume |
| **File operations** | Copy (F5), Move (F6), Delete (F8/Del), Rename (F2), New Folder (F7) |
| **Async progress** | Large copies/moves run in a worker thread with speed & ETA display |
| **Conflict resolution** | Per-file dialog: Overwrite / Skip / Rename / Cancel (with "All" options) |
| **Bookmarks** | Ctrl+D to add; sidebar list; JSON export/import |
| **Search** | Full search dialog: name wildcard, content text, date & size range |
| **Preview panel** | Images (async thumbnail), plain text/code, generic file info |
| **Filter bar** | Per-pane glob filter (e.g. `*.cpp`) shown at the bottom of each pane |
| **View modes** | Details / List / Icons / Thumbnails — switchable per pane |
| **Show hidden** | Ctrl+H toggle |
| **Dark / Light theme** | System / Light / Dark — applied via QApplication palette |
| **Session restore** | Re-opens every pane's tabs and paths on next launch |
| **Settings dialog** | General · Appearance · File Operations · Shortcuts |
| **Open terminal** | Opens the system terminal in the current directory |

---

## 🗂 Project Structure

```
FolderDir/
├── CMakeLists.txt            # CMake build file (Qt5 or Qt6)
├── docs/
│   └── planning.md           # Detailed planning document (Korean + English)
├── resources/
│   ├── resources.qrc         # Qt resource file
│   └── icons/
│       └── app.png           # Application icon
└── src/
    ├── main.cpp
    ├── MainWindow.h/cpp      # Main window: menu, toolbar, pane layout
    ├── FolderPane.h/cpp      # Individual pane: tabs + address bar + view
    ├── AddressBar.h/cpp      # Editable path bar
    ├── FileSystemBrowser.h/cpp  # File list (Details/List/Icons views)
    ├── FileSystemModel.h/cpp    # QFileSystemModel wrapper + filter
    ├── FileOperations.h/cpp     # Async copy/move/delete worker
    ├── FileOperationDialog.h/cpp  # Progress dialog with conflict resolution
    ├── BookmarkManager.h/cpp    # Bookmark persistence (JSON)
    ├── PreviewPanel.h/cpp       # Async image/text/info preview
    ├── SettingsManager.h/cpp    # Typed settings via QSettings
    ├── SettingsDialog.h/cpp     # Settings UI
    ├── SearchDialog.h/cpp       # Search UI + background worker
    └── DriveBar.h/cpp           # Drive/volume quick-access bar
```

---

## 🔧 Build Requirements

| Dependency | Version | Notes |
|------------|---------|-------|
| C++ compiler | C++17 | GCC 8+, Clang 7+, MSVC 2019+ |
| CMake | 3.16+ | |
| Qt | **6.x** (preferred) or **5.15** | Widgets, Concurrent modules |

### Install Qt

**Ubuntu/Debian**
```bash
sudo apt install qt6-base-dev qt6-base-dev-tools cmake ninja-build
# or Qt5:
sudo apt install qtbase5-dev qt5-default cmake
```

**macOS (Homebrew)**
```bash
brew install qt cmake ninja
export CMAKE_PREFIX_PATH=$(brew --prefix qt)
```

**Windows**
Download the Qt Online Installer from https://www.qt.io/download and install the *MSVC* or *MinGW* component.

---

## 🚀 Build & Run

```bash
# Clone
git clone https://github.com/shinehand/FolderDir.git
cd FolderDir

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Run
./build/FolderDir          # Linux / macOS
build\FolderDir.exe        # Windows
```

### Debug build
```bash
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
```

---

## ⌨️ Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| **Alt+←** | Navigate back |
| **Alt+→** | Navigate forward |
| **Alt+↑** | Go up one directory |
| **F2** | Rename selected |
| **F5** | Copy |
| **F6** | Move |
| **F7** / **Ctrl+Shift+N** | New folder |
| **Delete** / **F8** | Delete selected |
| **Ctrl+C** | Copy to clipboard |
| **Ctrl+X** | Cut |
| **Ctrl+V** | Paste |
| **Ctrl+A** | Select all |
| **Ctrl+Shift+C** | Copy full path to clipboard |
| **Ctrl+D** | Add current folder to bookmarks |
| **Ctrl+F** | Open search dialog |
| **Ctrl+H** | Toggle hidden files |
| **Ctrl+P** | Toggle preview panel |
| **Ctrl+T** | New tab |
| **Ctrl+W** | Close current tab |
| **Ctrl+1..4** | Activate pane 1–4 |
| **/** or **\\** | Toggle filter bar |
| **F5** (in view) | Refresh |
| **Backspace** | Navigate up |

---

## 🏗 Architecture Notes

- **Thread safety**: All file operations run in a dedicated `QThread` via `FileOperation`.  
  Progress signals use `Qt::QueuedConnection` to marshal results back to the GUI thread safely.
- **Virtual model**: `QFileSystemModel` handles on-demand directory loading, keeping the UI
  responsive even for directories with 100 000+ files.
- **Async thumbnails**: `PreviewPanel` uses `QtConcurrent::run` + `QFutureWatcher` to decode
  images without blocking the event loop.
- **Conflict resolution**: Blocked worker waits on a mutex; the GUI resolves and calls
  `FileOperation::resolveConflict()` to unblock it — no busy-spin on the GUI thread.
- **Session persistence**: `QSettings` stores pane count, tab paths, geometry, and all
  preferences. Restored automatically at startup when the option is enabled.

---

## 📋 Roadmap

- [x] Phase 1 ✅ — Multi-pane layout, file navigation, details view
- [x] Phase 2 ✅ — Tabs, file operations, address bar
- [x] Phase 3 ✅ — Bookmarks, search, filter, settings
- [x] Phase 4 ✅ — Preview panel, drive bar, async thumbnails
- [ ] Phase 5 🔴 — F-key shortcuts completion, breadcrumb address bar, folder tree sidebar, properties dialog, Ctrl+Tab
- [ ] Phase 6 🔴 — F6 move dialog, F9 copy-with-rename, CRC verification, bookmark export/import UI
- [ ] Phase 7 🟠 — File/folder color coding, folder size column, tab drag between panes, tab rename
- [ ] Phase 8 🟠 — Pane sync, pane lock/clone, layout favorites (64 slots), PDF preview, regex search
- [ ] Phase 9 🟡 — ZIP browsing, folder compare, directory export (CSV/TXT), portable mode (INI), Korean i18n
- [ ] Phase 10 🟡 — Toolbar customization, external tool slots, mouse gestures, high-contrast/accessibility

> See [docs/planning.md](docs/planning.md) for the full Q-Dir gap analysis, planning team findings, and sprint plan.

---

## 📄 License

MIT — see [LICENSE](LICENSE) for details.
