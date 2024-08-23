#include <QWidget>
#include <QTreeView>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QDrag>
#include <QMimeData>

class DraggableWidget : public QWidget {
    Q_OBJECT

public:
    DraggableWidget(QWidget *parent = nullptr) : QWidget(parent), isDragging(false) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        QTreeView *treeView = new QTreeView(this);

        // Add the tree view to the layout
        layout->addWidget(treeView);

        // Populate tree view with some data (optional)
        // ...

        setLayout(layout);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            dragStartPosition = event->pos();
            isDragging = true;
        }
        QWidget::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (isDragging && (event->buttons() & Qt::LeftButton)) {
            int distance = (event->pos() - dragStartPosition).manhattanLength();
            if (distance >= QApplication::startDragDistance()) {
                QDrag *drag = new QDrag(this);
                QMimeData *mimeData = new QMimeData;
                drag->setMimeData(mimeData);

                drag->exec(Qt::MoveAction);
                isDragging = false;
            }
        }
        QWidget::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        isDragging = false;
        QWidget::mouseReleaseEvent(event);
    }

private:
    QPoint dragStartPosition;
    bool isDragging;
};