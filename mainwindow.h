#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QToolBar>
#include <QAction>
#include <QTimer>
#include <QMouseEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QImageReader>
#include <QScrollBar>

// Forward declaration to avoid circular dependency
class CustomView;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void toggleToolbar();
    void checkMousePosition();
    void togglePinned();
    void handleImageDropped(const QString &filePath);

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    CustomView *canvas;
    QToolBar *toolbar;
    QTimer *toolbarTimer;

    bool isPinned = false;
    bool isMoving = false;
    QPoint dragPosition;
    QPointF lastPanPosition;
    bool isPanning = false;

    // Window resizing
    enum ResizeDirection { None, Top, Bottom, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
    ResizeDirection resizeDir = None;
    const int RESIZE_MARGIN = 5;

    void setupCanvas();
    void setupToolbar();
    void loadImageFile(const QString &filePath);
};
#endif // MAINWINDOW_H
