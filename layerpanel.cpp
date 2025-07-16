#include "layerpanel.h"
#include "imageitem.h"
#include <QMouseEvent>
#include <QPainter>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QSlider>
#include <QAbstractItemView>
#include <QGraphicsView>

// LayerItem Implementation
LayerItem::LayerItem(ImageItem *imageItem, LayerPanel *panel, QWidget *parent)
    : QWidget(parent), imageItem(imageItem), layerPanel(panel), selected(false)
{
    setupUI();
    updateThumbnail();
}

void LayerItem::setupUI()
{
    setFixedHeight(60);
    setStyleSheet(
        "LayerItem {"
        "    background-color: #2a2a2a;"
        "    border: 1px solid #404040;"
        "    border-radius: 5px;"
        "}"
        "LayerItem:hover {"
        "    background-color: #353535;"
        "}"
        );

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(8);

    // Thumbnail
    thumbnailLabel = new QLabel();
    thumbnailLabel->setFixedSize(48, 48);
    thumbnailLabel->setStyleSheet(
        "QLabel {"
        "    border: 1px solid #555;"
        "    border-radius: 3px;"
        "    background-color: #1a1a1a;"
        "}"
        );
    thumbnailLabel->setScaledContents(true);
    layout->addWidget(thumbnailLabel);

    // Layer info
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    nameLabel = new QLabel(QString("Layer %1").arg(rand() % 1000));
    nameLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    infoLayout->addWidget(nameLabel);

    // Opacity slider
    QHBoxLayout *opacityLayout = new QHBoxLayout();
    QLabel *opacityLabel = new QLabel("Opacity:");
    opacityLabel->setStyleSheet("QLabel { color: #ccc; font-size: 10px; }");

    opacitySlider = new QSlider(Qt::Horizontal);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(100);
    opacitySlider->setFixedWidth(80);
    opacitySlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    border: 1px solid #555;"
        "    height: 4px;"
        "    background: #333;"
        "    border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: #007AFF;"
        "    border: 1px solid #005AC7;"
        "    width: 12px;"
        "    height: 12px;"
        "    border-radius: 6px;"
        "    margin: -4px 0;"
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: #007AFF;"
        "    border-radius: 2px;"
        "}"
        );

    // Connect slider manually
    QObject::connect(opacitySlider, &QSlider::valueChanged, [this](int value) {
        onOpacityChanged(value);
    });

    opacityLayout->addWidget(opacityLabel);
    opacityLayout->addWidget(opacitySlider);
    opacityLayout->addStretch();

    infoLayout->addLayout(opacityLayout);
    layout->addLayout(infoLayout);

    layout->addStretch();

    // Visibility checkbox
    visibilityCheckBox = new QCheckBox();
    visibilityCheckBox->setChecked(true);
    visibilityCheckBox->setStyleSheet(
        "QCheckBox::indicator {"
        "    width: 18px;"
        "    height: 18px;"
        "}"
        "QCheckBox::indicator:unchecked {"
        "    background-color: #333;"
        "    border: 2px solid #555;"
        "    border-radius: 3px;"
        "}"
        "QCheckBox::indicator:checked {"
        "    background-color: #007AFF;"
        "    border: 2px solid #005AC7;"
        "    border-radius: 3px;"
        "}"
        );

    // Connect checkbox manually
    QObject::connect(visibilityCheckBox, &QCheckBox::toggled, [this](bool checked) {
        onVisibilityToggled(checked);
    });

    layout->addWidget(visibilityCheckBox);
}

void LayerItem::updateThumbnail()
{
    if (!imageItem) return;

    QPixmap pixmap = imageItem->pixmap();
    if (pixmap.isNull()) return;

    // Create thumbnail
    QPixmap thumbnail = pixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Create a square background
    QPixmap squareThumbnail(48, 48);
    squareThumbnail.fill(Qt::transparent);

    QPainter painter(&squareThumbnail);
    painter.setRenderHint(QPainter::Antialiasing);

    // Center the thumbnail
    int x = (48 - thumbnail.width()) / 2;
    int y = (48 - thumbnail.height()) / 2;
    painter.drawPixmap(x, y, thumbnail);

    thumbnailLabel->setPixmap(squareThumbnail);
}

void LayerItem::setSelected(bool selected)
{
    this->selected = selected;

    if (selected) {
        setStyleSheet(
            "LayerItem {"
            "    background-color: #007AFF;"
            "    border: 2px solid #005AC7;"
            "    border-radius: 5px;"
            "}"
            );
    } else {
        setStyleSheet(
            "LayerItem {"
            "    background-color: #2a2a2a;"
            "    border: 1px solid #404040;"
            "    border-radius: 5px;"
            "}"
            "LayerItem:hover {"
            "    background-color: #353535;"
            "}"
            );
    }
}

void LayerItem::setOpacity(int opacity)
{
    opacitySlider->setValue(opacity);
}

int LayerItem::getOpacity() const
{
    return opacitySlider->value();
}

void LayerItem::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void LayerItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && layerPanel) {
        layerPanel->selectLayer(imageItem);
    }
    QWidget::mousePressEvent(event);
}

void LayerItem::onVisibilityToggled(bool visible)
{
    if (imageItem) {
        imageItem->setVisible(visible);
    }
}

void LayerItem::onOpacityChanged(int value)
{
    if (imageItem) {
        imageItem->setOpacity(value / 100.0);
    }
}

// LayerPanel Implementation
LayerPanel::LayerPanel(QGraphicsScene *scene, QWidget *parent)
    : QWidget(parent), scene(scene), selectedLayer(nullptr), layerCounter(1)
{
    setupUI();
}

void LayerPanel::setupUI()
{
    setFixedWidth(280);
    setStyleSheet(
        "LayerPanel {"
        "    background-color: #1a1a1a;"
        "    border-left: 1px solid #404040;"
        "}"
        "QPushButton {"
        "    background-color: #333;"
        "    border: 1px solid #555;"
        "    border-radius: 6px;"
        "    color: white;"
        "    padding: 8px 12px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #444;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #222;"
        "}"
        "QLabel {"
        "    color: white;"
        "}"
        );

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // Title
    titleLabel = new QLabel("Layers");
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    color: white;"
        "    padding: 8px 0px;"
        "}"
        );
    mainLayout->addWidget(titleLabel);

    // Layer list
    layerList = new QListWidget();
    layerList->setStyleSheet(
        "QListWidget {"
        "    background-color: transparent;"
        "    border: none;"
        "    outline: none;"
        "}"
        "QListWidget::item {"
        "    background-color: transparent;"
        "    border: none;"
        "    padding: 2px;"
        "}"
        );
    layerList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mainLayout->addWidget(layerList);

    // Button layout
    QGridLayout *buttonLayout = new QGridLayout();
    buttonLayout->setSpacing(6);

    addButton = new QPushButton("+ Add");
    duplicateButton = new QPushButton("Duplicate");
    deleteButton = new QPushButton("Delete");
    moveUpButton = new QPushButton("↑");
    moveDownButton = new QPushButton("↓");

    moveUpButton->setFixedWidth(40);
    moveDownButton->setFixedWidth(40);

    buttonLayout->addWidget(addButton, 0, 0);
    buttonLayout->addWidget(duplicateButton, 0, 1);
    buttonLayout->addWidget(deleteButton, 1, 0);
    buttonLayout->addWidget(moveUpButton, 1, 1);
    buttonLayout->addWidget(moveDownButton, 1, 2);

    mainLayout->addLayout(buttonLayout);

    // Connect buttons using lambdas
    QObject::connect(addButton, &QPushButton::clicked, [this]() { addNewLayer(); });
    QObject::connect(duplicateButton, &QPushButton::clicked, [this]() { duplicateLayer(); });
    QObject::connect(deleteButton, &QPushButton::clicked, [this]() { deleteLayer(); });
    QObject::connect(moveUpButton, &QPushButton::clicked, [this]() { moveLayerUp(); });
    QObject::connect(moveDownButton, &QPushButton::clicked, [this]() { moveLayerDown(); });
}

void LayerPanel::addLayer(ImageItem *imageItem)
{
    LayerItem *layerItem = new LayerItem(imageItem, this);

    QListWidgetItem *listItem = new QListWidgetItem();
    listItem->setSizeHint(layerItem->sizeHint());

    layerList->insertItem(0, listItem); // Add to top
    layerList->setItemWidget(listItem, layerItem);

    updateLayerZValues();
    selectLayer(imageItem);
}

void LayerPanel::removeLayer(ImageItem *imageItem)
{
    LayerItem *layerItem = getLayerItem(imageItem);
    if (!layerItem) return;

    for (int i = 0; i < layerList->count(); ++i) {
        QListWidgetItem *listItem = layerList->item(i);
        if (layerList->itemWidget(listItem) == layerItem) {
            layerList->takeItem(i);
            delete listItem;
            break;
        }
    }

    if (selectedLayer == layerItem) {
        selectedLayer = nullptr;
    }

    updateLayerZValues();
}

void LayerPanel::selectLayer(ImageItem *imageItem)
{
    if (selectedLayer) {
        selectedLayer->setSelected(false);
    }

    LayerItem *layerItem = getLayerItem(imageItem);
    if (layerItem) {
        selectedLayer = layerItem;
        layerItem->setSelected(true);
    }
}

void LayerPanel::updateLayerOrder()
{
    updateLayerZValues();
}

void LayerPanel::addNewLayer()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Image",
        "",
        "Image Files (*.png *.jpg *.jpeg *.bmp *.gif *.tiff)"
        );

    if (!fileName.isEmpty()) {
        ImageItem *newItem = new ImageItem(fileName);
        scene->addItem(newItem);
        addLayer(newItem);
    }
}

void LayerPanel::duplicateLayer()
{
    // Simplified for now - would need to enhance ImageItem to support copying
}

void LayerPanel::deleteLayer()
{
    if (!selectedLayer) return;

    ImageItem *imageItem = selectedLayer->getImageItem();
    if (imageItem) {
        scene->removeItem(imageItem);
        removeLayer(imageItem);
        delete imageItem;
    }
}

void LayerPanel::moveLayerUp()
{
    if (!selectedLayer) return;

    for (int i = 0; i < layerList->count(); ++i) {
        QListWidgetItem *listItem = layerList->item(i);
        if (layerList->itemWidget(listItem) == selectedLayer && i > 0) {
            QListWidgetItem *item = layerList->takeItem(i);
            layerList->insertItem(i - 1, item);
            layerList->setItemWidget(item, selectedLayer);
            updateLayerZValues();
            break;
        }
    }
}

void LayerPanel::moveLayerDown()
{
    if (!selectedLayer) return;

    for (int i = 0; i < layerList->count(); ++i) {
        QListWidgetItem *listItem = layerList->item(i);
        if (layerList->itemWidget(listItem) == selectedLayer && i < layerList->count() - 1) {
            QListWidgetItem *item = layerList->takeItem(i);
            layerList->insertItem(i + 1, item);
            layerList->setItemWidget(item, selectedLayer);
            updateLayerZValues();
            break;
        }
    }
}

void LayerPanel::updateLayerZValues()
{
    // Update Z-values based on position in list (top = highest Z)
    for (int i = 0; i < layerList->count(); ++i) {
        QListWidgetItem *listItem = layerList->item(i);
        LayerItem *layerItem = static_cast<LayerItem*>(layerList->itemWidget(listItem));
        if (layerItem && layerItem->getImageItem()) {
            layerItem->getImageItem()->setZValue(layerList->count() - i);
        }
    }
}

LayerItem* LayerPanel::getLayerItem(ImageItem *imageItem)
{
    for (int i = 0; i < layerList->count(); ++i) {
        QListWidgetItem *listItem = layerList->item(i);
        LayerItem *layerItem = static_cast<LayerItem*>(layerList->itemWidget(listItem));
        if (layerItem && layerItem->getImageItem() == imageItem) {
            return layerItem;
        }
    }
    return nullptr;
}
