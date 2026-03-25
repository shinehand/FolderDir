#include "SearchDialog.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFileIconProvider>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QRegularExpression>

// ──────────────────────────────────────────────────────────────────────────────
// SearchWorker
// ──────────────────────────────────────────────────────────────────────────────
void SearchWorker::search(const SearchWorker::Criteria &c)
{
    m_cancelled = false;
    int found = 0;

    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;
    if (c.searchSubdirs)
        flags |= QDirIterator::Subdirectories;

    QDir::Filters filters = QDir::Files | QDir::NoDotAndDotDot;
    if (c.searchHidden)
        filters |= QDir::Hidden;

    QDirIterator it(c.rootPath, filters, flags);
    while (it.hasNext() && !m_cancelled) {
        const QString filePath = it.next();
        const QFileInfo fi     = it.fileInfo();

        emit searchProgress(fi.absolutePath());

        // Name filter
        if (!c.namePattern.isEmpty()) {
            if (!matchesName(fi.fileName(), c.namePattern)) continue;
        }

        // Size filter
        if (c.minSizeBytes >= 0 && fi.size() < c.minSizeBytes) continue;
        if (c.maxSizeBytes >= 0 && fi.size() > c.maxSizeBytes) continue;

        // Date filter
        if (c.modifiedAfter.isValid()  &&
            fi.lastModified().date() < c.modifiedAfter)  continue;
        if (c.modifiedBefore.isValid() &&
            fi.lastModified().date() > c.modifiedBefore) continue;

        // Content filter
        if (!c.contentText.isEmpty()) {
            if (!containsText(filePath, c.contentText)) continue;
        }

        emit resultFound(filePath, fi.size(), fi.lastModified());
        ++found;
    }

    emit searchFinished(found);
}

void SearchWorker::cancel()
{
    m_cancelled = true;
}

bool SearchWorker::matchesName(const QString &fileName,
                               const QString &pattern) const
{
    // Support comma-separated patterns
    const QStringList parts = pattern.split(QLatin1Char(','));
    for (const QString &p : parts) {
        const QString trimmed = p.trimmed();
        if (trimmed.isEmpty()) continue;
        const QRegularExpression re(
            QRegularExpression::wildcardToRegularExpression(trimmed),
            QRegularExpression::CaseInsensitiveOption);
        if (re.match(fileName).hasMatch()) return true;
    }
    return false;
}

bool SearchWorker::containsText(const QString &filePath,
                                const QString &text) const
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    // Read in chunks to avoid loading huge files
    while (!f.atEnd()) {
        const QString chunk = QString::fromUtf8(f.read(65536));
        if (chunk.contains(text, Qt::CaseInsensitive)) return true;
    }
    return false;
}

// ──────────────────────────────────────────────────────────────────────────────
// SearchDialog
// ──────────────────────────────────────────────────────────────────────────────
SearchDialog::SearchDialog(const QString &startPath, QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    m_pathEdit->setText(startPath);
    setWindowTitle(tr("Search Files"));
    resize(680, 500);

    // Worker thread
    m_workerThread = new QThread(this);
    m_worker = new SearchWorker;
    m_worker->moveToThread(m_workerThread);

    qRegisterMetaType<SearchWorker::Criteria>("SearchWorker::Criteria");

    connect(this, &SearchDialog::startSearchSignal,
            m_worker, &SearchWorker::search, Qt::QueuedConnection);
    connect(m_worker, &SearchWorker::resultFound,
            this, &SearchDialog::onResultFound, Qt::QueuedConnection);
    connect(m_worker, &SearchWorker::searchFinished,
            this, &SearchDialog::onSearchFinished, Qt::QueuedConnection);
    connect(m_worker, &SearchWorker::searchProgress,
            this, &SearchDialog::onProgressUpdate, Qt::QueuedConnection);

    m_workerThread->start();
}

SearchDialog::~SearchDialog()
{
    m_worker->cancel();
    m_workerThread->quit();
    if (!m_workerThread->wait(5000)) {
        m_workerThread->terminate();
        m_workerThread->wait(1000);
    }
    delete m_worker;
}

void SearchDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    // ── Criteria group ────────────────────────────────────────────────────
    auto *criteriaBox = new QGroupBox(tr("Search Criteria"), this);
    auto *form = new QFormLayout(criteriaBox);

    auto *pathRow = new QHBoxLayout;
    m_pathEdit = new QLineEdit(criteriaBox);
    auto *browseBtn = new QPushButton(tr("…"), criteriaBox);
    browseBtn->setFixedWidth(28);
    connect(browseBtn, &QPushButton::clicked, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(
            this, tr("Select Search Root"), m_pathEdit->text());
        if (!dir.isEmpty()) m_pathEdit->setText(dir);
    });
    pathRow->addWidget(m_pathEdit, 1);
    pathRow->addWidget(browseBtn);
    form->addRow(tr("Search in:"), pathRow);

    m_nameEdit = new QLineEdit(criteriaBox);
    m_nameEdit->setPlaceholderText(tr("e.g. *.cpp, report_*"));
    form->addRow(tr("File name:"), m_nameEdit);

    m_contentEdit = new QLineEdit(criteriaBox);
    m_contentEdit->setPlaceholderText(tr("Text to find inside files"));
    form->addRow(tr("Containing text:"), m_contentEdit);

    m_subdirs = new QCheckBox(tr("Include subdirectories"), criteriaBox);
    m_subdirs->setChecked(true);
    m_hidden  = new QCheckBox(tr("Include hidden files"), criteriaBox);
    auto *optRow = new QHBoxLayout;
    optRow->addWidget(m_subdirs);
    optRow->addWidget(m_hidden);
    form->addRow(optRow);

    // Date range
    m_useDateRange = new QCheckBox(tr("Date range:"), criteriaBox);
    m_dateFrom = new QDateEdit(QDate::currentDate().addYears(-1), criteriaBox);
    m_dateFrom->setEnabled(false);
    m_dateTo   = new QDateEdit(QDate::currentDate(), criteriaBox);
    m_dateTo->setEnabled(false);
    connect(m_useDateRange, &QCheckBox::toggled, m_dateFrom, &QWidget::setEnabled);
    connect(m_useDateRange, &QCheckBox::toggled, m_dateTo,   &QWidget::setEnabled);
    auto *dateRow = new QHBoxLayout;
    dateRow->addWidget(m_useDateRange);
    dateRow->addWidget(m_dateFrom);
    dateRow->addWidget(new QLabel(tr("–"), criteriaBox));
    dateRow->addWidget(m_dateTo);
    form->addRow(dateRow);

    // Size range
    m_useSizeRange = new QCheckBox(tr("Size range:"), criteriaBox);
    m_sizeMin = new QSpinBox(criteriaBox);
    m_sizeMin->setRange(0, 999999);
    m_sizeMin->setEnabled(false);
    m_sizeMax = new QSpinBox(criteriaBox);
    m_sizeMax->setRange(0, 999999);
    m_sizeMax->setValue(999999);
    m_sizeMax->setEnabled(false);
    m_sizeUnit = new QComboBox(criteriaBox);
    m_sizeUnit->addItems({tr("bytes"), tr("KB"), tr("MB")});
    m_sizeUnit->setCurrentIndex(1);
    m_sizeUnit->setEnabled(false);
    connect(m_useSizeRange, &QCheckBox::toggled, m_sizeMin,  &QWidget::setEnabled);
    connect(m_useSizeRange, &QCheckBox::toggled, m_sizeMax,  &QWidget::setEnabled);
    connect(m_useSizeRange, &QCheckBox::toggled, m_sizeUnit, &QWidget::setEnabled);
    auto *sizeRow = new QHBoxLayout;
    sizeRow->addWidget(m_useSizeRange);
    sizeRow->addWidget(m_sizeMin);
    sizeRow->addWidget(new QLabel(tr("–"), criteriaBox));
    sizeRow->addWidget(m_sizeMax);
    sizeRow->addWidget(m_sizeUnit);
    form->addRow(sizeRow);

    mainLayout->addWidget(criteriaBox);

    // ── Buttons ────────────────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    m_searchButton = new QPushButton(
        QIcon::fromTheme(QStringLiteral("system-search")), tr("Search"), this);
    m_searchButton->setDefault(true);
    m_cancelButton = new QPushButton(tr("Stop"), this);
    m_cancelButton->setEnabled(false);
    connect(m_searchButton, &QPushButton::clicked, this, &SearchDialog::onSearch);
    connect(m_cancelButton, &QPushButton::clicked, this, &SearchDialog::onCancel);
    btnRow->addStretch();
    btnRow->addWidget(m_searchButton);
    btnRow->addWidget(m_cancelButton);
    mainLayout->addLayout(btnRow);

    // ── Results ────────────────────────────────────────────────────────────
    m_results = new QTreeWidget(this);
    m_results->setColumnCount(3);
    m_results->setHeaderLabels({tr("Name"), tr("Size"), tr("Modified")});
    m_results->header()->setStretchLastSection(false);
    m_results->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_results->setRootIsDecorated(false);
    m_results->setAlternatingRowColors(true);
    m_results->setSortingEnabled(true);
    connect(m_results, &QTreeWidget::itemActivated,
            this, &SearchDialog::onResultDoubleClicked);
    mainLayout->addWidget(m_results, 1);

    // ── Status + progress ──────────────────────────────────────────────────
    m_statusLabel = new QLabel(tr("Ready"), this);
    mainLayout->addWidget(m_statusLabel);

    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 0); // indeterminate
    m_progress->hide();
    mainLayout->addWidget(m_progress);
}

// ──────────────────────────────────────────────────────────────────────────────
void SearchDialog::onSearch()
{
    m_results->clear();
    m_searching = true;
    m_searchButton->setEnabled(false);
    m_cancelButton->setEnabled(true);
    m_progress->show();
    m_statusLabel->setText(tr("Searching…"));

    SearchWorker::Criteria c;
    c.rootPath     = m_pathEdit->text();
    c.namePattern  = m_nameEdit->text().trimmed();
    c.contentText  = m_contentEdit->text().trimmed();
    c.searchSubdirs = m_subdirs->isChecked();
    c.searchHidden  = m_hidden->isChecked();

    if (m_useDateRange->isChecked()) {
        c.modifiedAfter  = m_dateFrom->date();
        c.modifiedBefore = m_dateTo->date();
    }

    if (m_useSizeRange->isChecked()) {
        const qint64 unit = (m_sizeUnit->currentIndex() == 0) ? 1LL :
                            (m_sizeUnit->currentIndex() == 1) ? 1024LL : 1024LL * 1024;
        c.minSizeBytes = m_sizeMin->value() * unit;
        c.maxSizeBytes = m_sizeMax->value() * unit;
    }

    emit startSearchSignal(c);
}

void SearchDialog::onCancel()
{
    m_worker->cancel();
}

void SearchDialog::onResultFound(const QString &path, qint64 size,
                                 const QDateTime &modified)
{
    auto *item = new QTreeWidgetItem(m_results);
    item->setText(0, path);
    item->setText(1, QLocale::system().formattedDataSize(
                         size, 1, QLocale::DataSizeTraditionalFormat));
    item->setText(2, modified.toString(Qt::DefaultLocaleShortDate));
    item->setData(0, Qt::UserRole, path);
    // Show folder icon or file icon
    item->setIcon(0, QFileIconProvider().icon(QFileInfo(path)));
    m_statusLabel->setText(
        tr("%n result(s) found", "", m_results->topLevelItemCount()));
}

void SearchDialog::onSearchFinished(int total)
{
    m_searching = false;
    m_searchButton->setEnabled(true);
    m_cancelButton->setEnabled(false);
    m_progress->hide();
    m_statusLabel->setText(tr("Search complete. %n result(s).", "", total));
}

void SearchDialog::onResultDoubleClicked()
{
    auto *item = m_results->currentItem();
    if (!item) return;
    emit navigateRequested(item->data(0, Qt::UserRole).toString());
}

void SearchDialog::onProgressUpdate(const QString &dir)
{
    m_statusLabel->setText(tr("Searching: %1").arg(dir));
}
