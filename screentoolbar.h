#ifndef SCREENTOOLBAR_H
#define SCREENTOOLBAR_H

#include <QToolBar>
#include <QAction>

class ScreenToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit ScreenToolBar(QWidget *parent = nullptr);

signals:
    void rectClicked();
    void circleClicked();
    void textClicked();
    void undoClicked();
    void saveRequested();   // 新增信号，保存截图
    void cancelRequested();
private:
    QAction *rectAction;
    QAction *circleAction;
    QAction *textAction;
    QAction *undoAction;
    QAction *saveAction;
    QAction *cancelAction;
};

#endif // SCREENTOOLBAR_H
