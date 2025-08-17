#include "ScreenshotWidget.h"
#include "ui_ScreenshotWidget.h"
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>

ScreenshotWidget::ScreenshotWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScreenshotWidget)
{
    ui->setupUi(this);

    // 设置窗口图标
    QString iconPath = ":/res/ico/icon.ico";
    setWindowIcon(QIcon(iconPath));
    setWindowTitle("WTS的截图软件");

    // 初始化 label
    QString initImgPath =  ":/res/img/init.png";
    QPixmap initPix(initImgPath);
    ui->init_img->setPixmap(initPix.scaled(ui->init_img->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->init_img->setAlignment(Qt::AlignCenter);

    setupTrayIcon();

    // 始终创建截图窗口，父对象 nullptr 避免被主窗口隐藏影响
    m_catcher = new CatchScreenWidget(nullptr);
    connect(m_catcher, &CatchScreenWidget::captureFinished, m_catcher, &QWidget::hide);

    connect(ui->screen_btn, &QPushButton::clicked, this, &ScreenshotWidget::startScreenshot);
    connect(ui->cancel_btn, &QPushButton::clicked, this, &ScreenshotWidget::cancelApp);
    connect(ui->mini_btn, &QPushButton::clicked, this, &ScreenshotWidget::minimizeToTray);
    connect(ui->save_btn, &QPushButton::clicked, this, &ScreenshotWidget::chooseSavePath);

    this->hide();

#ifdef Q_OS_WIN
    registerGlobalHotkey();
    qApp->installNativeEventFilter(this);
#endif
}

ScreenshotWidget::~ScreenshotWidget()
{
#ifdef Q_OS_WIN
    unregisterGlobalHotkey();
#endif
    delete ui;
}

void ScreenshotWidget::startScreenshot()
{
    if (m_catcher)
        m_catcher->showFullScreen();
}

void ScreenshotWidget::cancelApp()
{
    qApp->quit();
}

QString ScreenshotWidget::getSavePath()
{
    QSettings settings("WTS", "ScreenshotApp");
    QString dir = settings.value("savePath", "").toString();
    if (dir.isEmpty())
        dir = QCoreApplication::applicationDirPath() + "/Capture image";
    return dir;
}

void ScreenshotWidget::setSavePath(const QString &path)
{
    QSettings settings("WTS", "ScreenshotApp");
    settings.setValue("savePath", path);
}

void ScreenshotWidget::chooseSavePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择截图保存路径",
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    if (!dir.isEmpty()) {
        setSavePath(dir);
        QMessageBox::information(this, "路径已保存", "截图保存路径:\n" + dir);
    }
}

void ScreenshotWidget::minimizeToTray()
{
    hide();
    if (m_trayIcon)
        m_trayIcon->showMessage("WTS截图软件", "已最小化到托盘", QSystemTrayIcon::Information, 2000);
}

void ScreenshotWidget::closeEvent(QCloseEvent *event)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("退出应用");
    msgBox.setText("彻底关闭应用还是最小化到托盘？");
    QPushButton *closeBtn = msgBox.addButton("彻底关闭", QMessageBox::AcceptRole);
    QPushButton *minimizeBtn = msgBox.addButton("最小化到托盘", QMessageBox::RejectRole);
    msgBox.setDefaultButton(minimizeBtn);
    msgBox.exec();

    if (msgBox.clickedButton() == closeBtn)
        event->accept();
    else {
        hide();
        event->ignore();
    }
}

void ScreenshotWidget::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(QIcon(":/res/ico/icon.ico"), this);
    m_trayMenu = new QMenu(this);

    m_screenshotAction = new QAction("截图", this);
    m_exitAction = new QAction("退出", this);

    m_trayMenu->addAction(m_screenshotAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_exitAction);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();

    connect(m_screenshotAction, &QAction::triggered, this, &ScreenshotWidget::startScreenshot);
    connect(m_exitAction, &QAction::triggered, this, &ScreenshotWidget::cancelApp);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason){
        if(reason == QSystemTrayIcon::Trigger){
            if(this->isVisible()) hide();
            else show();
        }
    });
}

#ifdef Q_OS_WIN
void ScreenshotWidget::registerGlobalHotkey()
{
    if(!RegisterHotKey(HWND(winId()), 1, MOD_SHIFT, 0x50))
        qDebug() << "注册全局热键失败";
}

void ScreenshotWidget::unregisterGlobalHotkey()
{
    UnregisterHotKey(HWND(winId()),1);
}

bool ScreenshotWidget::nativeEventFilter(const QByteArray&, void *message, long*)
{
    MSG* msg = static_cast<MSG*>(message);
    if(msg->message == WM_HOTKEY && msg->wParam == 1){
        startScreenshot();
        return true;
    }
    return false;
}
#endif
