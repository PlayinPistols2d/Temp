#ifndef TASKCARD_H
#define TASKCARD_H

#include <QWidget>
#include <QTreeWidget>
#include "task.h"
#include "employee_card.h"

namespace Ui {
class TaskCard;
}

class TaskCard : public QWidget
{
    Q_OBJECT

public:
    explicit TaskCard(Task *task, QWidget *parent = nullptr);
    ~TaskCard();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Ui::TaskCard *ui;
    Task *m_task;
    QPoint dragStartPosition;

    void populateTree(Task *task, QTreeWidgetItem *parentItem);
    void updateTree();
};

#endif // TASKCARD_H
