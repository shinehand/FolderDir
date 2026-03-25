#include "PreviewPanel.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPlainTextEdit>
#include <QtGui/QImageReader>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtCore/QLocale>
#include <QtWidgets/QFileIconProvider>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>

// ──────────────────────────────────────────────────────────────────────────────
PreviewPanel::PreviewPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void PreviewPanel::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(4, 4, 4, 4);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setMinimumSize(64, 64);
    m_layout->addWidget(m_iconLabel);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(true);
    QFont f = m_nameLabel->font();
    f.setBold(true);
    m_nameLabel->setFont(f);
    m_layout->addWidget(m_nameLabel);

    m_infoLabel = new QLabel(this);
    m_infoLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setTextFormat(Qt::RichText);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_contentWidget = new QWidget;
    auto *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->addWidget(m_infoLabel);
    contentLayout->addStretch();
    m_scrollArea->setWidget(m_contentWidget);
    m_layout->addWidget(m_scrollArea, 1);
}

// ──────────────────────────────────────────────────────────────────────────────
void PreviewPanel::previewFile(const QString &path)
{
    if (path == m_currentPath) return;
    m_currentPath = path;

    const QFileInfo fi(path);
    m_nameLabel->setText(fi.fileName());

    static const QStringList imageExts = {
        QStringLiteral("jpg"), QStringLiteral("jpeg"),
        QStringLiteral("png"), QStringLiteral("bmp"),
        QStringLiteral("gif"), QStringLiteral("webp"),
        QStringLiteral("svg"), QStringLiteral("tiff"),
    };

    static const QStringList textExts = {
        QStringLiteral("txt"), QStringLiteral("md"),
        QStringLiteral("cpp"), QStringLiteral("h"),
        QStringLiteral("c"),   QStringLiteral("hpp"),
        QStringLiteral("py"),  QStringLiteral("js"),
        QStringLiteral("ts"),  QStringLiteral("json"),
        QStringLiteral("xml"), QStringLiteral("html"),
        QStringLiteral("css"), QStringLiteral("sh"),
        QStringLiteral("bat"), QStringLiteral("cmake"),
        QStringLiteral("ini"), QStringLiteral("cfg"),
        QStringLiteral("log"), QStringLiteral("yaml"),
    };

    const QString ext = fi.suffix().toLower();

    if (imageExts.contains(ext)) {
        showImage(path);
    } else if (textExts.contains(ext)) {
        showText(path);
    } else {
        showInfo(path);
    }
}

void PreviewPanel::clear()
{
    m_currentPath.clear();
    m_iconLabel->clear();
    m_nameLabel->clear();
    m_infoLabel->clear();
}

// ──────────────────────────────────────────────────────────────────────────────
void PreviewPanel::showImage(const QString &path)
{
    // Load asynchronously so the UI is never blocked
    auto *watcher = new QFutureWatcher<QPixmap>(this);
    connect(watcher, &QFutureWatcher<QPixmap>::finished, this, [this, watcher, path]() {
        const QPixmap px = watcher->result();
        watcher->deleteLater();
        onThumbnailReady(px, path);
    });

    const QSize maxSize = m_iconLabel->size().expandedTo(QSize(200, 200));
    watcher->setFuture(QtConcurrent::run([path, maxSize]() -> QPixmap {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        const QSize imgSize = reader.size();
        if (imgSize.isValid())
            reader.setScaledSize(imgSize.scaled(maxSize, Qt::KeepAspectRatio));
        const QImage img = reader.read();
        if (img.isNull()) return QPixmap();
        return QPixmap::fromImage(img);
    }));

    m_infoLabel->setText(tr("Loading image…"));
}

void PreviewPanel::onThumbnailReady(const QPixmap &px, const QString &path)
{
    if (path != m_currentPath) return;
    if (px.isNull()) {
        m_iconLabel->setText(tr("[Cannot load image]"));
    } else {
        m_iconLabel->setPixmap(px);
    }

    const QFileInfo fi(path);
    m_infoLabel->setText(
        QStringLiteral("<b>%1</b><br>%2<br><i>%3</i>")
            .arg(fi.fileName().toHtmlEscaped())
            .arg(QLocale::system().formattedDataSize(fi.size(), 2,
                     QLocale::DataSizeTraditionalFormat))
            .arg(QLocale::system().toString(fi.lastModified(), QLocale::ShortFormat)));
}

void PreviewPanel::showText(const QString &path)
{
    m_iconLabel->clear();
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_infoLabel->setText(tr("[Cannot open file]"));
        return;
    }
    // Read up to 8 KB for preview
    const QByteArray raw = f.read(8192);
    const QString text = QString::fromUtf8(raw).toHtmlEscaped();
    m_infoLabel->setText(
        QStringLiteral("<pre style='font-family:monospace;font-size:small;'>%1</pre>")
            .arg(text));
}

void PreviewPanel::showInfo(const QString &path)
{
    m_iconLabel->clear();
    const QFileInfo fi(path);

    const QIcon icon = QFileIconProvider().icon(fi);
    const QPixmap px = icon.pixmap(64, 64);
    m_iconLabel->setPixmap(px);

    m_infoLabel->setText(
        QStringLiteral(
            "<b>%1</b><br>"
            "Type: %2<br>"
            "Size: %3<br>"
            "Modified: %4<br>"
            "Created: %5"
        )
        .arg(fi.fileName().toHtmlEscaped())
        .arg(fi.suffix().isEmpty() ? tr("File") : fi.suffix().toUpper().toHtmlEscaped())
        .arg(QLocale::system().formattedDataSize(fi.size(), 2,
                 QLocale::DataSizeTraditionalFormat))
        .arg(QLocale::system().toString(fi.lastModified(), QLocale::ShortFormat))
        .arg(QLocale::system().toString(fi.birthTime(), QLocale::ShortFormat)));
}
