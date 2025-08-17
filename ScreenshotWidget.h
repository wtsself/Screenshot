#ifndef SCREENSHOTWIDGET_H
#define SCREENSHOTWIDGET_H

#include "catchscreenwidget.h"
#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QSettings>
#ifdef Q_OS_WIN
#include <windows.h>
#include <QAbstractNativeEventFilter>
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class ScreenshotWidget; }
QT_END_NAMESPACE

class ScreenshotWidget
#ifdef Q_OS_WIN
    : public QWidget, public QAbstractNativeEventFilter
#else
    : public QWidget
#endif
{
    Q_OBJECT
public:
    explicit ScreenshotWidget(QWidget *parent = nullptr);
    ~ScreenshotWidget();

    static QString getSavePath();
    static void setSavePath(const QString &path);

private slots:
    void startScreenshot();
    void cancelApp();
    void minimizeToTray();
    void chooseSavePath();

protected:
    void closeEvent(QCloseEvent *event) override;

#ifdef Q_OS_WIN
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    void registerGlobalHotkey();
    void unregisterGlobalHotkey();
#endif

private:
    void setupTrayIcon();

private:
    Ui::ScreenshotWidget *ui;
    CatchScreenWidget *m_catcher;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_screenshotAction;
    QAction *m_exitAction;
};

#endif // SCREENSHOTWIDGET_H
