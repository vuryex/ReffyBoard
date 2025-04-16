#include "imageitem.h"
#include <QPixmap>
#include <QGraphicsSceneHoverEvent>
#include <QCursor>
#include <QDebug>
#include <QGraphicsScene>
#include <QTransform>

ImageItem::ImageItem(const QString &filePath, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent),
    isResizing(false),
    currentHandle(None)
{
    QPixmap pixmap(filePath);

    if (pixmap.width() > 800 || pixmap.height() > 600) {
        pixmap = pixmap.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    setPixmap(pixmap);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
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
        painter->setPen(QPen(Qt::white, 1, Qt::SolidLine));
        painter->setBrush(QBrush(Qt::white));

        painter->drawRect(handleRect(TopLeft));
        painter->drawRect(handleRect(TopRight));
        painter->drawRect(handleRect(BottomLeft));
        painter->drawRect(handleRect(BottomRight));

        painter->setPen(QPen(Qt::white, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
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

ImageItem::ResizeHandle ImageItem::handleAt(const QPointF &pos) const
{
    if (handleRect(TopLeft).contains(pos)) {
        return TopLeft;
    } else if (handleRect(TopRight).contains(pos)) {
        return TopRight;
    } else if (handleRect(BottomLeft).contains(pos)) {
        return BottomLeft;
    } else if (handleRect(BottomRight).contains(pos)) {
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
            originalRect = boundingRect();
            event->accept();
            return;
        }

        setSelected(true);
        setZValue(1);
        QGraphicsPixmapItem::mousePressEvent(event);
    }
}

void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (isResizing && (event->buttons() & Qt::LeftButton)) {
        QPointF delta = event->pos() - dragStartPosition;
        QRectF rect = originalRect;
        QTransform transform;

        switch (currentHandle) {
        case TopLeft:
            rect.setTopLeft(rect.topLeft() + delta);
            break;
        case TopRight:
            rect.setTopRight(rect.topRight() + delta);
            break;
        case BottomLeft:
            rect.setBottomLeft(rect.bottomLeft() + delta);
            break;
        case BottomRight:
            rect.setBottomRight(rect.bottomRight() + delta);
            break;
        default:
            break;
        }

        qreal scaleX = rect.width() / originalRect.width();
        qreal scaleY = rect.height() / originalRect.height();

        if (rect.width() < 20 || rect.height() < 20) {
            event->accept();
            return;
        }

        setTransform(QTransform::fromScale(scaleX, scaleY), true);

        event->accept();
    } else {
        QGraphicsPixmapItem::mouseMoveEvent(event);
    }
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
