#ifndef CATCHSCREENWIDGET_H
#define CATCHSCREENWIDGET_H

#include <QWidget>
#include <QRect>
#include <QPixmap>
#include "screentoolbar.h"
#include "textoverlay.h"

class CatchScreenWidget : public QWidget
{
    Q_OBJECT
    enum class ShapeMode { None, Rect, Ellipse, Text };

public:
    explicit CatchScreenWidget(QWidget *parent = nullptr);
    void setTextMode();

public slots:
    void saveCapture();
    void closewindow();
    void setRectMode() { currentMode = ShapeMode::Rect; }
    void setEllipseMode() { currentMode = ShapeMode::Ellipse; }
    void undoSelection();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void captureFinished();

private:
    TextOverlay *textOverlay;
    ShapeMode currentMode;
    bool hasSelection;
    bool isSelecting;
    QPoint startPoint;
    QRect selectionRect;
    QPixmap fullScreenPixmap;
    ScreenToolBar *toolbar;
};

#endif
