#ifndef LAYERPANEL_H
#define LAYERPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QSlider>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPainter>
#include <QPixmap>

class ImageItem;

class LayerItem : public QWidget
{
public:
    explicit LayerItem(ImageItem *imageItem, LayerPanel *panel, QWidget *parent = nullptr);

    ImageItem* getImageItem() const { return imageItem; }
    void updateThumbnail();
    void setSelected(bool selected);
    bool isSelected() const { return selected; }
    void setOpacity(int opacity);
    int getOpacity() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    ImageItem *imageItem;
    LayerPanel *layerPanel;
    QLabel *thumbnailLabel;
    QCheckBox *visibilityCheckBox;
    QSlider *opacitySlider;
    QLabel *nameLabel;
    bool selected;

    void setupUI();
    void onVisibilityToggled(bool visible);
    void onOpacityChanged(int value);
};

class LayerPanel : public QWidget
{
public:
    explicit LayerPanel(QGraphicsScene *scene, QWidget *parent = nullptr);

    void addLayer(ImageItem *imageItem);
    void removeLayer(ImageItem *imageItem);
    void selectLayer(ImageItem *imageItem);
    void updateLayerOrder();

private:
    QGraphicsScene *scene;
    QVBoxLayout *mainLayout;
    QListWidget *layerList;
    QPushButton *addButton;
    QPushButton *duplicateButton;
    QPushButton *deleteButton;
    QPushButton *moveUpButton;
    QPushButton *moveDownButton;
    QLabel *titleLabel;

    LayerItem *selectedLayer;
    int layerCounter;

    void setupUI();
    void updateLayerZValues();
    LayerItem* getLayerItem(ImageItem *imageItem);
    void addNewLayer();
    void duplicateLayer();
    void deleteLayer();
    void moveLayerUp();
    void moveLayerDown();
    void onLayerSelected(LayerItem *layer);
    void onLayerVisibilityChanged(LayerItem *layer, bool visible);
    void onLayerOpacityChanged(LayerItem *layer, int opacity);
};

#endif // LAYERPANEL_H
