#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imageitem.h"
#include "customview.h"
#include "layerpanel.h"
#include <QDebug>
#include <QKeyEvent>
#include <QGraphicsPixmapItem>
#include <QScreen>
#include <QApplication>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Use Qt::Window to allow resizing but still frameless
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMaximizeButtonHint);

    // Allow resizing
    setAttribute(Qt::WA_NoSystemBackground, false);
    setAttribute(Qt::WA_TranslucentBackground, false);

    // Set a reasonable default size (not fullscreen)
    resize(1024, 768);

    setMouseTracking(true);

    setupCanvas();
    setupToolbar();

    toolbarTimer = new QTimer(this);
    connect(toolbarTimer, &QTimer::timeout, this, &MainWindow::checkMousePosition);
    toolbarTimer->start(200);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupCanvas()
{
    scene = new QGraphicsScene(this);

    // Create main widget with horizontal layout
    QWidget *centralWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create canvas
    canvas = new CustomView(scene, this);
    canvas->setRenderHint(QPainter::Antialiasing);
    canvas->setRenderHint(QPainter::SmoothPixmapTransform);
    canvas->setDragMode(QGraphicsView::NoDrag);
    canvas->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    canvas->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    canvas->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    canvas->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    canvas->setBackgroundBrush(QColor("#2e2e2e"));
    canvas->viewport()->installEventFilter(this);
    canvas->setMouseTracking(true);
    canvas->setFocusPolicy(Qt::StrongFocus);

    // Create layer panel
    layerPanel = new LayerPanel(scene, this);
    layerPanel->setVisible(false); // Initially hidden

    // Add to layout
    mainLayout->addWidget(canvas);
    mainLayout->addWidget(layerPanel);

    connect(canvas, &CustomView::imageDropped, this, &MainWindow::handleImageDropped);

    setCentralWidget(centralWidget);
}

void MainWindow::setupToolbar()
{
    toolbar = new QToolBar("Main Toolbar", this);
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->setIconSize(QSize(24, 24));

    QAction *exitAction = toolbar->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);

    QAction *pinAction = toolbar->addAction("Pin");
    connect(pinAction, &QAction::triggered, this, &MainWindow::togglePinned);

    QAction *layersAction = toolbar->addAction("Layers");
    connect(layersAction, &QAction::triggered, this, &MainWindow::toggleLayerPanel);

    toolbar->setVisible(false);

    addToolBar(Qt::TopToolBarArea, toolbar);
}

void MainWindow::togglePinned()
{
    isPinned = !isPinned;

    Qt::WindowFlags flags = windowFlags();
    QPoint pos = this->pos();

    if (isPinned) {
        setWindowFlags(flags | Qt::WindowStaysOnTopHint);
    } else {
        setWindowFlags(flags & ~Qt::WindowStaysOnTopHint);
    }

    show();

    move(pos);
}

void MainWindow::toggleToolbar()
{
    toolbar->setVisible(!toolbar->isVisible());
}

void MainWindow::toggleLayerPanel()
{
    layerPanel->setVisible(!layerPanel->isVisible());
}

void MainWindow::checkMousePosition()
{
    QPoint mousePos = mapFromGlobal(QCursor::pos());
    if (rect().contains(mousePos)) {
        bool shouldShow = mousePos.y() < 40;
        if (toolbar->isVisible() != shouldShow) {
            toolbar->setVisible(shouldShow);
        }
    }

    // Set cursor based on position for resizing
    if (!isMoving && resizeDir == None) {
        const int x = mousePos.x();
        const int y = mousePos.y();
        const int width = this->width();
        const int height = this->height();

        if (x <= RESIZE_MARGIN && y <= RESIZE_MARGIN) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (x >= width - RESIZE_MARGIN && y <= RESIZE_MARGIN) {
            setCursor(Qt::SizeBDiagCursor);
        } else if (x <= RESIZE_MARGIN && y >= height - RESIZE_MARGIN) {
            setCursor(Qt::SizeBDiagCursor);
        } else if (x >= width - RESIZE_MARGIN && y >= height - RESIZE_MARGIN) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (x <= RESIZE_MARGIN) {
            setCursor(Qt::SizeHorCursor);
        } else if (x >= width - RESIZE_MARGIN) {
            setCursor(Qt::SizeHorCursor);
        } else if (y <= RESIZE_MARGIN) {
            setCursor(Qt::SizeVerCursor);
        } else if (y >= height - RESIZE_MARGIN) {
            setCursor(Qt::SizeVerCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Check if we're on an edge for resizing
        const int x = event->pos().x();
        const int y = event->pos().y();
        const int width = this->width();
        const int height = this->height();

        resizeDir = None;

        // Determine resize direction
        if (x <= RESIZE_MARGIN && y <= RESIZE_MARGIN) {
            resizeDir = TopLeft;
            setCursor(Qt::SizeFDiagCursor);
        } else if (x >= width - RESIZE_MARGIN && y <= RESIZE_MARGIN) {
            resizeDir = TopRight;
            setCursor(Qt::SizeBDiagCursor);
        } else if (x <= RESIZE_MARGIN && y >= height - RESIZE_MARGIN) {
            resizeDir = BottomLeft;
            setCursor(Qt::SizeBDiagCursor);
        } else if (x >= width - RESIZE_MARGIN && y >= height - RESIZE_MARGIN) {
            resizeDir = BottomRight;
            setCursor(Qt::SizeFDiagCursor);
        } else if (x <= RESIZE_MARGIN) {
            resizeDir = Left;
            setCursor(Qt::SizeHorCursor);
        } else if (x >= width - RESIZE_MARGIN) {
            resizeDir = Right;
            setCursor(Qt::SizeHorCursor);
        } else if (y <= RESIZE_MARGIN) {
            resizeDir = Top;
            setCursor(Qt::SizeVerCursor);
        } else if (y >= height - RESIZE_MARGIN) {
            resizeDir = Bottom;
            setCursor(Qt::SizeVerCursor);
        }

        if (resizeDir != None) {
            // We're resizing
            dragPosition = event->globalPosition().toPoint();
            event->accept();
            return;
        }

        // Check if we're clicking on an empty area (for moving the window)
        if (!canvas->itemAt(canvas->mapFromParent(event->pos()))) {
            isMoving = true;
            dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        }
        event->accept();
    }

    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && resizeDir != None) {
        // We're resizing
        QRect newGeom = geometry();
        QPoint globalPos = event->globalPosition().toPoint();
        QPoint delta = globalPos - dragPosition;
        dragPosition = globalPos;

        // Apply the appropriate resize based on direction
        switch (resizeDir) {
        case Left:
            newGeom.setLeft(newGeom.left() + delta.x());
            break;
        case Right:
            newGeom.setRight(newGeom.right() + delta.x());
            break;
        case Top:
            newGeom.setTop(newGeom.top() + delta.y());
            break;
        case Bottom:
            newGeom.setBottom(newGeom.bottom() + delta.y());
            break;
        case TopLeft:
            newGeom.setTopLeft(newGeom.topLeft() + delta);
            break;
        case TopRight:
            newGeom.setTopRight(newGeom.topRight() + QPoint(delta.x(), delta.y()));
            break;
        case BottomLeft:
            newGeom.setBottomLeft(newGeom.bottomLeft() + QPoint(delta.x(), delta.y()));
            break;
        case BottomRight:
            newGeom.setBottomRight(newGeom.bottomRight() + delta);
            break;
        default:
            break;
        }

        // Ensure minimum size
        if (newGeom.width() >= minimumWidth() && newGeom.height() >= minimumHeight()) {
            setGeometry(newGeom);
        }

        event->accept();
    } else if (isMoving && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }

    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isMoving = false;
        resizeDir = None;
    }

    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    double scaleFactor = 1.15;
    if (event->angleDelta().y() < 0) {
        canvas->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    } else {
        canvas->scale(scaleFactor, scaleFactor);
    }
    event->accept();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == canvas->viewport()) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Control) {
                canvas->viewport()->setCursor(Qt::OpenHandCursor);
                return true;
            }
        } else if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Control) {
                canvas->viewport()->setCursor(Qt::ArrowCursor);
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::handleImageDropped(const QString &filePath)
{
    loadImageFile(filePath);
}

void MainWindow::loadImageFile(const QString &filePath)
{
    ImageItem *item = new ImageItem(filePath);

    QPointF dropPos = canvas->mapToScene(canvas->mapFromGlobal(QCursor::pos()));
    item->setPos(dropPos);

    scene->addItem(item);

    // Add to layer panel
    layerPanel->addLayer(item);

    // Select the newly added item and give it focus
    scene->clearSelection();
    item->setSelected(true);
    item->setFocus();
}
