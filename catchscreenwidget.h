#ifndef CATCHSCREENWIDGET_H
#define CATCHSCREENWIDGET_H

#include <QWidget>
#include <QRect>
#include "screentoolbar.h"
#include"textoverlay.h"

class CatchScreenWidget : public QWidget
{
    Q_OBJECT
    enum class ShapeMode {
        None,
        Rect,
        Ellipse,
        Text

    };
public:
    explicit CatchScreenWidget(QWidget *parent = nullptr);
     void setTextMode();
     void keyPressEvent(QKeyEvent *event) override;
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

signals:
    void captureFinished();

private:
    TextOverlay *textOverlay;   // 新增文字编辑组件

    QString textInput;
    ShapeMode currentMode;
    bool hasSelection;  // 是否存在选区
    bool isSelecting;
    QPoint startPoint;
    QRect selectionRect;
    QPixmap fullScreenPixmap;
    ScreenToolBar *toolbar;


};

#endif // CATCHSCREENWIDGET_H
