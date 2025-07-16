#include "imageitem.h"
#include <QPixmap>
#include <QGraphicsSceneHoverEvent>
#include <QCursor>
#include <QDebug>
#include <QGraphicsScene>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QApplication>
#include <QGraphicsView>
#include <QAction>
#include <cmath>

ImageItem::ImageItem(const QString &filePath, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent),
    isResizing(false),
    currentHandle(None),
    currentScaleMode(Proportional)
{
    originalPixmap = QPixmap(filePath);

    // Scale down if too large, but keep original for quality
    QPixmap displayPixmap = originalPixmap;
    if (displayPixmap.width() > 800 || displayPixmap.height() > 600) {
        displayPixmap = displayPixmap.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    setPixmap(displayPixmap);
    initialSize = displayPixmap.size();

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);

    setZValue(0);
}

ImageItem::~ImageItem()
{
}

void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPixmapItem::paint(painter, option, widget);

    if (isSelected()) {
        // Draw selection border
        painter->setPen(QPen(Qt::white, 2, Qt::SolidLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(QGraphicsPixmapItem::boundingRect());

        // Draw small resize handles
        painter->setPen(QPen(Qt::white, 1, Qt::SolidLine));
        painter->setBrush(QBrush(Qt::white));

        painter->drawRect(handleRect(TopLeft));
        painter->drawRect(handleRect(TopRight));
        painter->drawRect(handleRect(BottomLeft));
        painter->drawRect(handleRect(BottomRight));
    }
}

QRectF ImageItem::boundingRect() const
{
    QRectF rect = QGraphicsPixmapItem::boundingRect();
    return rect.adjusted(-HANDLE_MARGIN, -HANDLE_MARGIN, HANDLE_MARGIN, HANDLE_MARGIN);
}

QRectF ImageItem::handleRect(ResizeHandle handle) const
{
    QRectF rect = QGraphicsPixmapItem::boundingRect();
    QRectF handleRect(0, 0, HANDLE_SIZE, HANDLE_SIZE);

    switch (handle) {
    case TopLeft:
        handleRect.moveTopLeft(rect.topLeft());
        break;
    case TopRight:
        handleRect.moveTopRight(rect.topRight());
        break;
    case BottomLeft:
        handleRect.moveBottomLeft(rect.bottomLeft());
        break;
    case BottomRight:
        handleRect.moveBottomRight(rect.bottomRight());
        break;
    default:
        break;
    }

    return handleRect;
}

QRectF ImageItem::handleArea(ResizeHandle handle) const
{
    QRectF rect = QGraphicsPixmapItem::boundingRect();
    QRectF handleArea(0, 0, HANDLE_AREA_SIZE, HANDLE_AREA_SIZE);

    switch (handle) {
    case TopLeft:
        handleArea.moveCenter(rect.topLeft());
        break;
    case TopRight:
        handleArea.moveCenter(rect.topRight());
        break;
    case BottomLeft:
        handleArea.moveCenter(rect.bottomLeft());
        break;
    case BottomRight:
        handleArea.moveCenter(rect.bottomRight());
        break;
    default:
        break;
    }

    return handleArea;
}

ImageItem::ResizeHandle ImageItem::handleAt(const QPointF &pos) const
{
    if (handleArea(TopLeft).contains(pos)) {
        return TopLeft;
    } else if (handleArea(TopRight).contains(pos)) {
        return TopRight;
    } else if (handleArea(BottomLeft).contains(pos)) {
        return BottomLeft;
    } else if (handleArea(BottomRight).contains(pos)) {
        return BottomRight;
    } else {
        return None;
    }
}

void ImageItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        currentHandle = handleAt(event->pos());

        if (currentHandle != None) {
            isResizing = true;
            dragStartPosition = event->pos();
            initialSize = pixmap().size();
            initialPos = pos();
            event->accept();
            return;
        }

        // Clear other selections first
        if (scene()) {
            scene()->clearSelection();
        }
        setSelected(true);
        setFocus();
        setZValue(1);
        QGraphicsPixmapItem::mousePressEvent(event);
    } else if (event->button() == Qt::RightButton) {
        if (scene()) {
            scene()->clearSelection();
        }
        setSelected(true);
        setFocus();
        showContextMenu(event->pos());
        event->accept();
    }
}

void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (isResizing && (event->buttons() & Qt::LeftButton)) {
        resizePixmap(event->pos());
        event->accept();
    } else {
        QGraphicsPixmapItem::mouseMoveEvent(event);
    }
}

void ImageItem::resizePixmap(const QPointF &currentPos)
{
    QPointF delta = currentPos - dragStartPosition;
    QSizeF newSize = initialSize;
    QPointF newPos = initialPos;

    // Calculate new size based on which handle is being dragged
    switch (currentHandle) {
    case TopLeft:
        newSize.setWidth(initialSize.width() - delta.x());
        newSize.setHeight(initialSize.height() - delta.y());
        newPos = QPointF(initialPos.x() + delta.x(), initialPos.y() + delta.y());
        break;
    case TopRight:
        newSize.setWidth(initialSize.width() + delta.x());
        newSize.setHeight(initialSize.height() - delta.y());
        newPos = QPointF(initialPos.x(), initialPos.y() + delta.y());
        break;
    case BottomLeft:
        newSize.setWidth(initialSize.width() - delta.x());
        newSize.setHeight(initialSize.height() + delta.y());
        newPos = QPointF(initialPos.x() + delta.x(), initialPos.y());
        break;
    case BottomRight:
        newSize.setWidth(initialSize.width() + delta.x());
        newSize.setHeight(initialSize.height() + delta.y());
        break;
    default:
        return;
    }

    // Minimum size check
    if (newSize.width() < 20 || newSize.height() < 20) {
        return;
    }

    // Handle proportional vs free scaling
    if (currentScaleMode == Proportional) {
        // Determine which dimension to use as the basis
        qreal scaleX = newSize.width() / initialSize.width();
        qreal scaleY = newSize.height() / initialSize.height();

        // Use the larger scale factor to ensure the image doesn't get smaller than intended
        qreal scale = qMax(qAbs(scaleX), qAbs(scaleY));

        newSize.setWidth(initialSize.width() * scale);
        newSize.setHeight(initialSize.height() * scale);

        // Adjust position for proportional scaling
        if (currentHandle == TopLeft) {
            qreal widthDiff = newSize.width() - initialSize.width();
            qreal heightDiff = newSize.height() - initialSize.height();
            newPos = QPointF(initialPos.x() - widthDiff, initialPos.y() - heightDiff);
        } else if (currentHandle == TopRight) {
            qreal heightDiff = newSize.height() - initialSize.height();
            newPos = QPointF(initialPos.x(), initialPos.y() - heightDiff);
        } else if (currentHandle == BottomLeft) {
            qreal widthDiff = newSize.width() - initialSize.width();
            newPos = QPointF(initialPos.x() - widthDiff, initialPos.y());
        }
    }

    // Create new scaled pixmap
    QPixmap scaledPixmap = originalPixmap.scaled(newSize.toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    setPixmap(scaledPixmap);
    setPos(newPos);
}

void ImageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (isResizing) {
        isResizing = false;
        currentHandle = None;
        event->accept();
    } else {
        QGraphicsPixmapItem::mouseReleaseEvent(event);
    }
}

void ImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (isSelected()) {
        ResizeHandle handle = handleAt(event->pos());

        switch (handle) {
        case TopLeft:
        case BottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case TopRight:
        case BottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }

    QGraphicsPixmapItem::hoverMoveEvent(event);
}

void ImageItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        if (scene()) {
            scene()->removeItem(this);
            delete this;
        }
        event->accept();
    } else {
        QGraphicsPixmapItem::keyPressEvent(event);
    }
}

void ImageItem::showContextMenu(const QPointF &pos)
{
    QMenu *menu = new QMenu();

    QAction *proportionalAction = menu->addAction("Proportional Scaling");
    proportionalAction->setCheckable(true);
    proportionalAction->setChecked(currentScaleMode == Proportional);

    QAction *freeAction = menu->addAction("Free Scaling");
    freeAction->setCheckable(true);
    freeAction->setChecked(currentScaleMode == Free);

    menu->addSeparator();

    QAction *deleteAction = menu->addAction("Delete");

    // Convert position to global coordinates
    QGraphicsView *view = nullptr;
    if (scene() && !scene()->views().isEmpty()) {
        view = scene()->views().first();
        QPoint globalPos = view->mapToGlobal(view->mapFromScene(mapToScene(pos)));

        QAction *selectedAction = menu->exec(globalPos);

        if (selectedAction == proportionalAction) {
            setProportionalMode();
        } else if (selectedAction == freeAction) {
            setFreeMode();
        } else if (selectedAction == deleteAction) {
            if (scene()) {
                scene()->removeItem(this);
                delete this;
                return;
            }
        }
    }

    delete menu;
}

void ImageItem::setProportionalMode()
{
    currentScaleMode = Proportional;
    update();
}

void ImageItem::setFreeMode()
{
    currentScaleMode = Free;
    update();
}
