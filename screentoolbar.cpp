#include "screentoolbar.h"
#include <QDebug>

ScreenToolBar::ScreenToolBar(QWidget *parent)
    : QToolBar(parent)
{
    setIconSize(QSize(40, 40));
    setStyleSheet("QToolBar { background: white; border-radius:6px;font-size:10px}");

    // 添加动作并设置图标
    QString toolbar_pre=":/res/img/";
    rectAction   = addAction(QIcon(toolbar_pre+"rect.png"), "矩形");
    circleAction = addAction(QIcon(toolbar_pre+"circle.png"), "圆形");
    textAction   = addAction(QIcon(toolbar_pre+"text.png"), "文字");
    undoAction   = addAction(QIcon(toolbar_pre+"undo.png"), "撤销");
    saveAction   = addAction(QIcon(toolbar_pre+"save.png"), "保存");
    cancelAction = addAction(QIcon(toolbar_pre+"cancel.png"), "取消");

    connect(rectAction,   &QAction::triggered, this, &ScreenToolBar::rectClicked);
    connect(circleAction, &QAction::triggered, this, &ScreenToolBar::circleClicked);
    connect(textAction,   &QAction::triggered, this, &ScreenToolBar::textClicked);
    connect(undoAction,   &QAction::triggered, this, &ScreenToolBar::undoClicked);
    connect(saveAction,   &QAction::triggered, this, &ScreenToolBar::saveRequested);
    connect(cancelAction, &QAction::triggered, this, &ScreenToolBar::cancelRequested);

}
