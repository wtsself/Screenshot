#include "catchscreenwidget.h"
#include "textoverlay.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include <QDir>
#include <QDateTime>
#include "screenshotwidget.h"

CatchScreenWidget::CatchScreenWidget(QWidget *parent)
    : QWidget(parent)
{
    isSelecting = false;
    hasSelection = false;
    currentMode = ShapeMode::Rect;
    textOverlay = nullptr; // 初始化为空

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    fullScreenPixmap = QGuiApplication::primaryScreen()->grabWindow(0);

    toolbar = new ScreenToolBar(this);
    toolbar->hide();

    connect(toolbar, &ScreenToolBar::saveRequested, this, &CatchScreenWidget::saveCapture);
    connect(toolbar, &ScreenToolBar::cancelRequested, this, &CatchScreenWidget::closewindow);
    connect(toolbar, &ScreenToolBar::rectClicked, this, &CatchScreenWidget::setRectMode);
    connect(toolbar, &ScreenToolBar::circleClicked, this, &CatchScreenWidget::setEllipseMode);
    connect(toolbar, &ScreenToolBar::undoClicked, this, &CatchScreenWidget::undoSelection);
    connect(toolbar, &ScreenToolBar::textClicked, this, &CatchScreenWidget::setTextMode);
}

// 切换到文字模式
void CatchScreenWidget::setTextMode()
{
    currentMode = ShapeMode::Text;
    if (!hasSelection) return;

    if (!textOverlay) {
        textOverlay = new TextOverlay(this);
        textOverlay->setGeometry(selectionRect.normalized());
        textOverlay->show();
        textOverlay->setFocus();
    } else {
        textOverlay->setGeometry(selectionRect.normalized());
        textOverlay->show();
        textOverlay->setFocus();
    }
}

// 撤销
void CatchScreenWidget::undoSelection()
{
    isSelecting = false;
    hasSelection = false;
    selectionRect = QRect();
    toolbar->hide();

    if (textOverlay) {
        textOverlay->hide();
        textOverlay->deleteLater();
        textOverlay = nullptr;
    }

    update(); // 强制刷新
}

// 关闭窗口
void CatchScreenWidget::closewindow()
{
    emit captureFinished();
    close();
}


// 保存截图
void CatchScreenWidget::saveCapture()
{
    if (selectionRect.isNull() || fullScreenPixmap.isNull())
        return;

    QRect r = selectionRect.normalized();
    QPixmap selectedPixmap(r.size());
    selectedPixmap.fill(Qt::transparent);

    QPainter painter(&selectedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制选区形状（矩形或圆形）
    QPainterPath path;
    if (currentMode == ShapeMode::Ellipse)
        path.addEllipse(selectedPixmap.rect());
    else
        path.addRect(selectedPixmap.rect());

    painter.setClipPath(path);
    painter.drawPixmap(0, 0, fullScreenPixmap.copy(r));

    // 绘制文字Overlay
    if (textOverlay && !textOverlay->toPlainText().isEmpty()) {
        QPixmap textPixmap(textOverlay->size());
        textPixmap.fill(Qt::transparent);
        textOverlay->render(&textPixmap);
        QPoint relativePos = textOverlay->pos() - r.topLeft();
        painter.drawPixmap(relativePos, textPixmap);
    }

    painter.end();

    // ========= 使用静态工具函数获取保存路径 =========
    QString saveDir = ScreenshotWidget::getSavePath();

    QDir dir(saveDir);
    if (!dir.exists()) dir.mkpath(".");

    QString filePath = saveDir + "/" +
                       QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") +
                       ".png";
    if (selectedPixmap.save(filePath))
        qDebug() << "截图已保存到:" << filePath;
    else
        qDebug() << "保存失败!";

    closewindow();
}


// 绘制截图与选区
void CatchScreenWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.drawPixmap(0, 0, fullScreenPixmap);
    p.fillRect(rect(), QColor(0,0,0,120));

    if (hasSelection || isSelecting) {
        QRect r = selectionRect.normalized();
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(r.topLeft(), fullScreenPixmap.copy(r));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

        p.setPen(QPen(Qt::green, 2));
        if (currentMode == ShapeMode::Rect)
            p.drawRect(r);
        else if (currentMode == ShapeMode::Ellipse)
            p.drawEllipse(r);
    }

    // 绘制文字
    if (textOverlay && textOverlay->isVisible())
        textOverlay->render(&p);
}

// 鼠标按下
void CatchScreenWidget::mousePressEvent(QMouseEvent *e)
{
    if (currentMode == ShapeMode::Text && hasSelection) {
        if (textOverlay) {
            textOverlay->setFocus();
            return;
        }
    }

    if (e->button() == Qt::LeftButton) {
        isSelecting = true;
        hasSelection = false;
        selectionRect = QRect(e->pos(), e->pos());
        toolbar->hide();

        if (textOverlay) {
            textOverlay->hide();
            textOverlay->deleteLater();
            textOverlay = nullptr;
        }

        update();
    }
}

// 鼠标移动
void CatchScreenWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (isSelecting) {
        selectionRect.setBottomRight(e->pos());
        update();
    }
}

// 鼠标释放
void CatchScreenWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (isSelecting && event->button() == Qt::LeftButton) {
        isSelecting = false;
        hasSelection = true;
        selectionRect.setBottomRight(event->pos());

        if (currentMode == ShapeMode::Text) {
            if (!textOverlay) {
                textOverlay = new TextOverlay(this);
            }
            textOverlay->setGeometry(selectionRect.normalized());
            textOverlay->show();
            textOverlay->setFocus();
        }

        toolbar->adjustSize();
        QPoint toolPos = selectionRect.normalized().bottomRight();
        toolPos.setX(toolPos.x() - toolbar->width());
        toolPos.setY(toolPos.y());
        toolbar->move(toolPos);
        toolbar->show();

        update();
    }
}
void CatchScreenWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Q) {
        closewindow();  // 调用已有的关闭窗口函数
    } else {
        QWidget::keyPressEvent(event); // 其它按键保持默认行为
    }
}
