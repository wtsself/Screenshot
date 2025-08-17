#pragma once
#include <QTextEdit>
#include <QMouseEvent>
#include <QTimer>

class TextOverlay : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextOverlay(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void adjustSizeToContent();

private:
    bool m_dragging;
    QPoint m_dragStartPos;
    QTimer m_resizeTimer; // 定时器避免频繁调整大小
};
