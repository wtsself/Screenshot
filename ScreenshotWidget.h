#ifndef SCREENSHOTWIDGET_H
#define SCREENSHOTWIDGET_H

#include "catchscreenwidget.h"
#include <QWidget>
#include <QPixmap>
#include <QScreen>
#include <QRubberBand>
#include <QMouseEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
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
    ScreenshotWidget(QWidget *parent = nullptr);
    ~ScreenshotWidget();
    static QString getSavePath();   // 获取保存路径
    static void setSavePath(const QString &path);  // 设置保存路径


private slots:
    void startScreenshot();  // 触发截图
    void cancelApp();        // 退出应用
    void minimizeToTray();
    void chooseSavePath();   // 选择保存路径


#ifdef Q_OS_WIN
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    void registerGlobalHotkey();
    void unregisterGlobalHotkey();
#endif
private:
    void setupTrayIcon();

protected:
    void closeEvent(QCloseEvent *event) override;  // 捕获关闭事件
private:

    Ui::ScreenshotWidget *ui;
    CatchScreenWidget *m_catcher = nullptr;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_screenshotAction;
    QAction *m_exitAction;
    // 截图相关
    QPixmap m_screenPixmap;
    QRubberBand *m_rubberBand;
    QPoint m_origin;
    QRect m_selectionRect;

    bool m_isCapturing;  // 是否在截图模式
};

#endif // SCREENSHOTWIDGET_H
