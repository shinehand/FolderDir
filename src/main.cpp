#include <QtWidgets/QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "MainWindow.h"
#include "SettingsManager.h"

#ifdef Q_OS_WIN
// On Windows the app is built with the WIN32 subsystem (no console window).
// If a fatal Qt error occurs before the main window is shown (e.g. a missing
// Qt platform plugin such as platforms/qwindows.dll), Qt calls qFatal() and
// the process exits silently — the user sees nothing.  This handler intercepts
// fatal messages and shows a native MessageBox so the error is always visible.
static void qtFatalMessageHandler(QtMsgType type,
                                   const QMessageLogContext & /*ctx*/,
                                   const QString &msg)
{
    if (type == QtFatalMsg) {
        MessageBoxW(nullptr,
                    msg.toStdWString().c_str(),
                    L"FolderDir - Fatal Error",
                    MB_OK | MB_ICONERROR);
        abort();
    }
}
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // Must be installed before QApplication so that platform-plugin errors
    // (and other fatal Qt messages that fire during QApplication construction)
    // are shown to the user as a dialog rather than silently killing the process.
    qInstallMessageHandler(qtFatalMessageHandler);
#endif

    // High-DPI support (Qt5: must set before QApplication)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("FolderDir"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("FolderDir"));
    app.setOrganizationDomain(QStringLiteral("folderdir.app"));

    // Load translation
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = QStringLiteral("FolderDir_") + QLocale(locale).name();
        if (translator.load(QStringLiteral(":/i18n/") + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    // Create and show main window
    MainWindow w;
    w.show();

    return app.exec();
}
