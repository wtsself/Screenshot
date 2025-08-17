#include "ScreenshotWidget.h"
#include "ui_ScreenshotWidget.h"

#include <QGuiApplication>
#include <QApplication>
#include <QPainter>
#include <QFileDialog>
#include <QClipboard>
#include <QPixmap>
#include <QDebug>

ScreenshotWidget::ScreenshotWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScreenshotWidget)
    , m_rubberBand(nullptr)
    , m_isCapturing(false)
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


    // 设置托盘
    setupTrayIcon();

    // 信号槽绑定
    connect(ui->screen_btn, &QPushButton::clicked, this, &ScreenshotWidget::startScreenshot);
    connect(ui->cancel_btn, &QPushButton::clicked, this, &ScreenshotWidget::cancelApp);
    connect(ui->mini_btn, &QPushButton::clicked, this, &ScreenshotWidget::minimizeToTray);
    connect(ui->save_btn, &QPushButton::clicked, this, &ScreenshotWidget::chooseSavePath);

    // 启动时隐藏窗口
    this->hide();

#ifdef Q_OS_WIN
    // 注册全局热键 Shift+P
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
    if (!m_catcher) {
        m_catcher = new CatchScreenWidget();

        // 截图窗口关闭时释放指针
        connect(m_catcher, &CatchScreenWidget::captureFinished, this, [=]() {
            m_catcher = nullptr;
        });
    }

    m_catcher->showFullScreen();
}

void ScreenshotWidget::cancelApp()
{
    qApp->quit();  // 退出应用
}
QString ScreenshotWidget::getSavePath()
{
    QSettings settings("WTS", "ScreenshotApp");
    QString dir = settings.value("savePath", "").toString();

    // 如果用户没设置，就用默认路径（程序目录/Capture image）
    if (dir.isEmpty()) {
        dir = QCoreApplication::applicationDirPath() + "/Capture image";
    }
    return dir;
}

void ScreenshotWidget::setSavePath(const QString &path)
{
    QSettings settings("WTS", "ScreenshotApp");
    settings.setValue("savePath", path);
}

void ScreenshotWidget::chooseSavePath()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "选择截图保存路径",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
    );

    if (!dir.isEmpty()) {
        ScreenshotWidget::setSavePath(dir);
        QMessageBox::information(this, "路径已保存", "截图保存路径:\n" + dir);
    }
}
void ScreenshotWidget::minimizeToTray()
{
    this->hide();   // 隐藏主窗口
    if (m_trayIcon)
        m_trayIcon->showMessage("WTS截图软件", "已最小化到托盘", QSystemTrayIcon::Information, 2000);
}
void ScreenshotWidget::closeEvent(QCloseEvent *event)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("退出应用");
    msgBox.setText("你想要彻底关闭应用还是最小化到托盘继续运行？");
    QPushButton *closeBtn = msgBox.addButton("彻底关闭", QMessageBox::AcceptRole);
    QPushButton *minimizeBtn = msgBox.addButton("最小化到托盘", QMessageBox::RejectRole);

    msgBox.setDefaultButton(minimizeBtn);

    msgBox.exec();

    if (msgBox.clickedButton() == closeBtn) {
        // 用户选择彻底关闭
        event->accept();
    } else {
        // 用户选择最小化到托盘
        this->hide();           // 隐藏主窗口
        event->ignore();        // 忽略关闭事件，继续运行
    }
}
void ScreenshotWidget::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon("D:/qt-pro/Screenshot/res/ico/icon.ico"));
    m_trayIcon->setToolTip("WTS截图软件");

    m_trayMenu = new QMenu(this);

    m_screenshotAction = new QAction("截图", this);
    m_exitAction = new QAction("退出", this);

    m_trayMenu->addAction(m_screenshotAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_exitAction);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();

    // 信号槽
    connect(m_screenshotAction, &QAction::triggered, this, &ScreenshotWidget::startScreenshot);
    connect(m_exitAction, &QAction::triggered, this, &ScreenshotWidget::cancelApp);

    // 点击托盘图标可显示或隐藏主窗口（可选）
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason){
        if(reason == QSystemTrayIcon::Trigger) {
            if(this->isVisible())
                this->hide();
            else
                this->show();
        }
    });
}

#ifdef Q_OS_WIN
void ScreenshotWidget::registerGlobalHotkey()
{
    // ID: 1, MOD_SHIFT + 'P'
    if (!RegisterHotKey(HWND(winId()), 1, MOD_SHIFT, 0x50)) {
        qDebug() << "注册全局热键失败";
    } else {
        qDebug() << "Shift+P 全局热键注册成功";
    }
}

void ScreenshotWidget::unregisterGlobalHotkey()
{
    UnregisterHotKey(HWND(winId()), 1);
}

bool ScreenshotWidget::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_HOTKEY) {
        if (msg->wParam == 1) { // 热键ID
            startScreenshot();   // 触发截图
            return true;
        }
    }
    return false;
}
#endif
