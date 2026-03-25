#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtCore/QThread>
#include <QtCore/QDate>
#include <QtCore/QDateTime>

/**
 * @brief SearchWorker — runs file search in a background thread.
 */
class SearchWorker : public QObject
{
    Q_OBJECT
public:
    struct Criteria {
        QString rootPath;
        QString namePattern;    ///< wildcard, case-insensitive
        QString contentText;    ///< empty = don't search content
        bool    searchSubdirs{true};
        bool    searchHidden{false};
        qint64  minSizeBytes{-1};
        qint64  maxSizeBytes{-1};
        QDate   modifiedAfter;
        QDate   modifiedBefore;
    };

    explicit SearchWorker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void search(const SearchWorker::Criteria &criteria);
    void cancel();

signals:
    void resultFound(const QString &path, qint64 size,
                     const QDateTime &modified);
    void searchFinished(int totalFound);
    void searchProgress(const QString &currentDir);

private:
    bool matchesName(const QString &fileName,
                     const QString &pattern) const;
    bool containsText(const QString &filePath,
                      const QString &text) const;

    bool m_cancelled{false};
};

Q_DECLARE_METATYPE(SearchWorker::Criteria)

/**
 * @brief SearchDialog — full file-search UI with result list.
 */
class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(const QString &startPath, QWidget *parent = nullptr);
    ~SearchDialog() override;

signals:
    /** Emitted when the user double-clicks a result. */
    void navigateRequested(const QString &path);
    void startSearchSignal(const SearchWorker::Criteria &criteria);

private slots:
    void onSearch();
    void onCancel();
    void onResultFound(const QString &path, qint64 size,
                       const QDateTime &modified);
    void onSearchFinished(int total);
    void onResultDoubleClicked();
    void onProgressUpdate(const QString &dir);

private:
    void setupUi();

    QLineEdit    *m_pathEdit{nullptr};
    QLineEdit    *m_nameEdit{nullptr};
    QLineEdit    *m_contentEdit{nullptr};
    QCheckBox    *m_subdirs{nullptr};
    QCheckBox    *m_hidden{nullptr};
    QCheckBox    *m_useDateRange{nullptr};
    QDateEdit    *m_dateFrom{nullptr};
    QDateEdit    *m_dateTo{nullptr};
    QCheckBox    *m_useSizeRange{nullptr};
    QSpinBox     *m_sizeMin{nullptr};
    QSpinBox     *m_sizeMax{nullptr};
    QComboBox    *m_sizeUnit{nullptr};

    QTreeWidget  *m_results{nullptr};
    QLabel       *m_statusLabel{nullptr};
    QProgressBar *m_progress{nullptr};
    QPushButton  *m_searchButton{nullptr};
    QPushButton  *m_cancelButton{nullptr};

    QThread      *m_workerThread{nullptr};
    SearchWorker *m_worker{nullptr};
    bool          m_searching{false};
};
