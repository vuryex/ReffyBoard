#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>

class ImageItem : public QGraphicsPixmapItem
{
public:
    explicit ImageItem(const QString &filePath, QGraphicsItem *parent = nullptr);
    ~ImageItem() override;

    enum ResizeHandle {
        None,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override;

private:
    QPointF dragStartPosition;
    QRectF originalRect;
    bool isResizing;
    ResizeHandle currentHandle;
    QRectF handleRect(ResizeHandle handle) const;
    ResizeHandle handleAt(const QPointF &pos) const;

    static const int HANDLE_SIZE = 10;
    static const int HANDLE_MARGIN = 5;
};

#endif
