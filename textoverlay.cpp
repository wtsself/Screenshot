#include "textoverlay.h"
#include <QPainter>

TextOverlay::TextOverlay(QWidget *parent)
    : QTextEdit(parent), m_dragging(false)
{
    setStyleSheet("background: rgba(255,255,255,0); border: 1px dashed green;; color: white;");
    setAlignment(Qt::AlignCenter);

    setFrameStyle(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAcceptRichText(false);

    // 定时器，每 50ms 调整一次大小
    m_resizeTimer.setInterval(50);
    m_resizeTimer.setSingleShot(false);
    connect(&m_resizeTimer, &QTimer::timeout, this, &TextOverlay::adjustSizeToContent);
    m_resizeTimer.start();
}

void TextOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStartPos = event->globalPos() - frameGeometry().topLeft();
    }
    QTextEdit::mousePressEvent(event);
}

void TextOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        move(event->globalPos() - m_dragStartPos);
    }
    QTextEdit::mouseMoveEvent(event);
}

void TextOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QTextEdit::mouseReleaseEvent(event);
}

void TextOverlay::paintEvent(QPaintEvent *event)
{
    QTextEdit::paintEvent(event);
    QPainter p(viewport());
    p.setPen(QPen(Qt::green, 1, Qt::DashLine));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

// 自动调整文字框大小
void TextOverlay::adjustSizeToContent()
{
    if (toPlainText().isEmpty()) return;

    document()->adjustSize();
    QSize docSize = document()->size().toSize();
    // 加上边框和 padding
    QSize newSize = docSize + QSize(10, 10);
    if (newSize != size())
        setFixedSize(newSize);
}
