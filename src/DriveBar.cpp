#include "DriveBar.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QSizePolicy>
#include <QtCore/QStorageInfo>
#include <QtCore/QTimer>

// ──────────────────────────────────────────────────────────────────────────────
DriveBar::DriveBar(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    refresh();

    m_timer = new QTimer(this);
    m_timer->setInterval(2000);
    connect(m_timer, &QTimer::timeout, this, &DriveBar::refresh);
    m_timer->start();
}

void DriveBar::setupUi()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(2, 1, 2, 1);
    m_layout->setSpacing(2);
    m_layout->addStretch();

    m_group = new QButtonGroup(this);
    m_group->setExclusive(false);
    connect(m_group, &QButtonGroup::buttonClicked,
            this, &DriveBar::onButtonClicked);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(28);
}

// ──────────────────────────────────────────────────────────────────────────────
void DriveBar::refresh()
{
    // Remove old buttons
    const auto oldButtons = m_group->buttons();
    for (auto *b : oldButtons) {
        m_group->removeButton(b);
        m_layout->removeWidget(b);
        b->deleteLater();
    }

    const QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    int insertPos = 0; // insert before the trailing stretch
    for (const QStorageInfo &vol : volumes) {
        if (!vol.isValid() || !vol.isReady()) continue;
        auto *btn = new QPushButton(buttonLabel(vol), this);
        btn->setFixedHeight(22);
        btn->setToolTip(
            QStringLiteral("%1  [%2]  Free: %3")
                .arg(vol.displayName())
                .arg(QString::fromUtf8(vol.fileSystemType()))
                .arg(QLocale::system().formattedDataSize(
                    vol.bytesFree(), 1, QLocale::DataSizeTraditionalFormat)));
        btn->setProperty("rootPath", vol.rootPath());
        m_group->addButton(btn);
        m_layout->insertWidget(insertPos++, btn);
    }
}

void DriveBar::onButtonClicked(QAbstractButton *button)
{
    const QString root = button->property("rootPath").toString();
    if (!root.isEmpty())
        emit driveSelected(root);
}

QString DriveBar::buttonLabel(const QStorageInfo &info)
{
    QString label = info.displayName();
    if (label.isEmpty()) label = info.rootPath();
    // Append drive letter on Windows
#ifdef Q_OS_WIN
    if (!info.rootPath().isEmpty())
        label = info.rootPath().left(2) + QStringLiteral(" ") + label;
#endif
    return label;
}
