#include "catchscreenwidget.h"
#include <QGuiApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QScreen>
#include "screenshotwidget.h"

CatchScreenWidget::CatchScreenWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    fullScreenPixmap = QGuiApplication::primaryScreen()->grabWindow(0);
    isSelecting = false;
    hasSelection = false;
    currentMode = ShapeMode::Rect;
    textOverlay = nullptr;

    toolbar = new ScreenToolBar(this);
    toolbar->hide();

    connect(toolbar, &ScreenToolBar::saveRequested, this, &CatchScreenWidget::saveCapture);
    connect(toolbar, &ScreenToolBar::cancelRequested, this, &CatchScreenWidget::closewindow);
    connect(toolbar, &ScreenToolBar::rectClicked, this, &CatchScreenWidget::setRectMode);
    connect(toolbar, &ScreenToolBar::circleClicked, this, &CatchScreenWidget::setEllipseMode);
    connect(toolbar, &ScreenToolBar::undoClicked, this, &CatchScreenWidget::undoSelection);
    connect(toolbar, &ScreenToolBar::textClicked, this, &CatchScreenWidget::setTextMode);
}

void CatchScreenWidget::setTextMode()
{
    currentMode = ShapeMode::Text;
    if (!hasSelection) return;

    if (!textOverlay){
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

void CatchScreenWidget::undoSelection()
{
    isSelecting = false;
    hasSelection = false;
    selectionRect = QRect();
    toolbar->hide();
    if(textOverlay){
        textOverlay->hide();
        textOverlay->deleteLater();
        textOverlay=nullptr;
    }
    update();
}

void CatchScreenWidget::closewindow()
{
    hide();
    emit captureFinished();
}

void CatchScreenWidget::saveCapture()
{
    if (selectionRect.isNull() || fullScreenPixmap.isNull())
        return;

    QRect r = selectionRect.normalized();
    QPixmap selectedPixmap(r.size());
    selectedPixmap.fill(Qt::transparent);

    QPainter painter(&selectedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    if(currentMode==ShapeMode::Ellipse) path.addEllipse(selectedPixmap.rect());
    else path.addRect(selectedPixmap.rect());
    painter.setClipPath(path);

    painter.drawPixmap(0,0, fullScreenPixmap.copy(r));

    if(textOverlay && !textOverlay->toPlainText().isEmpty()){
        QPixmap textPixmap(textOverlay->size());
        textPixmap.fill(Qt::transparent);
        textOverlay->render(&textPixmap);
        QPoint relativePos = textOverlay->pos()-r.topLeft();
        painter.drawPixmap(relativePos, textPixmap);
    }
    painter.end();

    QString saveDir = ScreenshotWidget::getSavePath();
    QDir dir(saveDir); if(!dir.exists()) dir.mkpath(".");
    QString filePath = saveDir + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")+".png";
    if(selectedPixmap.save(filePath)) qDebug() << "截图已保存:" << filePath;

    closewindow();
}

void CatchScreenWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawPixmap(0,0, fullScreenPixmap);
    p.fillRect(rect(), QColor(0,0,0,120));

    if(hasSelection || isSelecting){
        QRect r = selectionRect.normalized();
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(r.topLeft(), fullScreenPixmap.copy(r));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

        p.setPen(QPen(Qt::green,2));
        if(currentMode==ShapeMode::Rect) p.drawRect(r);
        else if(currentMode==ShapeMode::Ellipse) p.drawEllipse(r);
    }

    if(textOverlay && textOverlay->isVisible())
        textOverlay->render(&p);
}

void CatchScreenWidget::mousePressEvent(QMouseEvent *e)
{
    if(currentMode==ShapeMode::Text && hasSelection){
        if(textOverlay){
            textOverlay->setFocus();
            return;
        }
    }
    if(e->button()==Qt::LeftButton){
        isSelecting=true;
        hasSelection=false;
        selectionRect=QRect(e->pos(),e->pos());
        toolbar->hide();
        if(textOverlay){
            textOverlay->hide();
            textOverlay->deleteLater();
            textOverlay=nullptr;
        }
        update();
    }
}

void CatchScreenWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(isSelecting){
        selectionRect.setBottomRight(e->pos());
        update();
    }
}

void CatchScreenWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(isSelecting && e->button()==Qt::LeftButton){
        isSelecting=false;
        hasSelection=true;
        selectionRect.setBottomRight(e->pos());

        if(currentMode==ShapeMode::Text){
            if(!textOverlay) textOverlay=new TextOverlay(this);
            textOverlay->setGeometry(selectionRect.normalized());
            textOverlay->show();
            textOverlay->setFocus();
        }

        toolbar->adjustSize();
        QPoint toolPos=selectionRect.normalized().bottomRight();
        toolPos.setX(toolPos.x()-toolbar->width());
        toolPos.setY(toolPos.y());
        toolbar->move(toolPos);
        toolbar->show();

        update();
    }
}

void CatchScreenWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Escape || event->key()==Qt::Key_Q)
        closewindow();
    else QWidget::keyPressEvent(event);
}
