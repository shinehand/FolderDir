#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QPixmap>

/**
 * @brief PreviewPanel — shows a preview of the selected file.
 *
 * Supported types:
 *  - Images (JPEG, PNG, BMP, GIF, WebP via Qt)
 *  - Plain text / source code
 *  - Unknown: shows file info (name, size, type, dates)
 */
class PreviewPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewPanel(QWidget *parent = nullptr);

public slots:
    void previewFile(const QString &path);
    void clear();

private slots:
    void onThumbnailReady(const QPixmap &px, const QString &path);

private:
    void showImage(const QString &path);
    void showText(const QString &path);
    void showInfo(const QString &path);
    void setupUi();

    QLabel       *m_iconLabel{nullptr};
    QLabel       *m_nameLabel{nullptr};
    QLabel       *m_infoLabel{nullptr};
    QScrollArea  *m_scrollArea{nullptr};
    QWidget      *m_contentWidget{nullptr};
    QVBoxLayout  *m_layout{nullptr};

    QString       m_currentPath;
};
