#include "customview.h"
#include <QFileInfo>
#include <QImageReader>
#include <QKeyEvent>
#include <QScrollBar>
#include <QApplication>
#include <QWheelEvent>

CustomView::CustomView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent),
    isPanning(false),
    ctrlPressed(false)
{
    setAcceptDrops(true);
    setMouseTracking(true);

    // Enable keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // Set scene rect to something very large to allow unrestricted panning
    if (scene) {
        scene->setSceneRect(-10000, -10000, 20000, 20000);
    }

    // Set anchors for smooth zooming
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
}

void CustomView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CustomView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CustomView::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        foreach (const QUrl &url, mimeData->urls()) {
            QString filePath = url.toLocalFile();
            QFileInfo fileInfo(filePath);

            QList<QByteArray> formatsByteArray = QImageReader::supportedImageFormats();
            QStringList formats;

            foreach (const QByteArray &format, formatsByteArray) {
                formats.append(QString::fromUtf8(format));
            }

            if (formats.contains(fileInfo.suffix().toLower())) {
                emit imageDropped(filePath);
            }
        }
    }

    event->acceptProposedAction();
}

void CustomView::wheelEvent(QWheelEvent *event)
{
    // Handle zooming with the mouse wheel
    double scaleFactor = 1.15;

    if (event->angleDelta().y() < 0) {
        // Zoom out
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    } else {
        // Zoom in
        scale(scaleFactor, scaleFactor);
    }

    event->accept();
}

void CustomView::mousePressEvent(QMouseEvent *event)
{
    // Check for middle button or Ctrl+left button for panning
    bool canPan = (event->button() == Qt::MiddleButton) ||
                  ((QApplication::keyboardModifiers() & Qt::ControlModifier) &&
                   event->button() == Qt::LeftButton);

    if (canPan) {
        // Enable panning mode
        isPanning = true;
        lastPanPosition = event->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void CustomView::mouseMoveEvent(QMouseEvent *event)
{
    if (isPanning &&
        ((event->buttons() & Qt::MiddleButton) ||
         ((QApplication::keyboardModifiers() & Qt::ControlModifier) &&
          (event->buttons() & Qt::LeftButton)))) {

        // Calculate how much to scroll
        QPoint delta = event->pos() - lastPanPosition;
        lastPanPosition = event->pos();

        // Apply the scroll with more direct control
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        event->accept();
        return;
    }

    // Update cursor based on Ctrl state
    if (!isPanning) {
        bool currentCtrlState = (QApplication::keyboardModifiers() & Qt::ControlModifier);
        if (currentCtrlState) {
            viewport()->setCursor(Qt::OpenHandCursor);
        } else {
            // Only reset cursor if we're not over a resize handle or other special item
            if (cursor().shape() == Qt::OpenHandCursor ||
                cursor().shape() == Qt::ClosedHandCursor) {
                viewport()->setCursor(Qt::ArrowCursor);
            }
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

void CustomView::mouseReleaseEvent(QMouseEvent *event)
{
    if ((event->button() == Qt::MiddleButton) ||
        (event->button() == Qt::LeftButton && isPanning)) {

        isPanning = false;

        // Get current ctrl state
        bool currentCtrlState = (QApplication::keyboardModifiers() & Qt::ControlModifier);

        if (currentCtrlState) {
            viewport()->setCursor(Qt::OpenHandCursor);
        } else {
            viewport()->setCursor(Qt::ArrowCursor);
        }

        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void CustomView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        ctrlPressed = true;
        if (!isPanning) {
            viewport()->setCursor(Qt::OpenHandCursor);
        }
    } else if (event->key() == Qt::Key_Space) {
        // Alternative: Space+mouse to pan
        if (!isPanning) {
            viewport()->setCursor(Qt::OpenHandCursor);
        }
    }

    QGraphicsView::keyPressEvent(event);
}

void CustomView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        ctrlPressed = false;
        if (!isPanning) {
            viewport()->setCursor(Qt::ArrowCursor);
        }
    } else if (event->key() == Qt::Key_Space) {
        if (!isPanning) {
            viewport()->setCursor(Qt::ArrowCursor);
        }
    }

    QGraphicsView::keyReleaseEvent(event);
}
