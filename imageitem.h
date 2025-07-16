#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QKeyEvent>
#include <QMenu>

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

    enum ScaleMode {
        Proportional,    // Maintains aspect ratio
        Free            // Allows stretching
    };

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QPointF dragStartPosition;
    QSizeF initialSize;
    QPointF initialPos;
    bool isResizing;
    ResizeHandle currentHandle;
    ScaleMode currentScaleMode;
    QPixmap originalPixmap;

    QRectF handleRect(ResizeHandle handle) const;
    QRectF handleArea(ResizeHandle handle) const;
    ResizeHandle handleAt(const QPointF &pos) const;
    void showContextMenu(const QPointF &pos);
    void resizePixmap(const QPointF &currentPos);
    void setProportionalMode();
    void setFreeMode();

    static const int HANDLE_SIZE = 8;
    static const int HANDLE_MARGIN = 5;
    static const int HANDLE_AREA_SIZE = 25;
};

#endif
