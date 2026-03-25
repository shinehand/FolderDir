#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QPushButton>
#include <QtCore/QStorageInfo>
#include <QtCore/QList>

/**
 * @brief DriveBar — horizontal row of buttons for each mounted drive/volume.
 *
 * Clicking a button navigates the active pane to that drive's root.
 * The bar refreshes automatically when storage changes (polling every 2 s).
 */
class DriveBar : public QWidget
{
    Q_OBJECT

public:
    explicit DriveBar(QWidget *parent = nullptr);

signals:
    void driveSelected(const QString &rootPath);

private slots:
    void refresh();
    void onButtonClicked(QAbstractButton *button);

private:
    void setupUi();
    static QString buttonLabel(const QStorageInfo &info);

    QHBoxLayout  *m_layout{nullptr};
    QButtonGroup *m_group{nullptr};
    class QTimer *m_timer{nullptr};
};
