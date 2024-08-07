#ifndef TASKPANEL_H
#define TASKPANEL_H

#include <QWidget>
#include <QMap>
#include <QScrollArea>
#include "task.h"
#include "task_slot.h"

namespace Ui {
class TaskPanel;
}

class TaskPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TaskPanel(QWidget *parent = nullptr);
    ~TaskPanel();

    void setTasks(const QList<Task> &tasks);

private slots:
    void onTaskChanged(int index);
    void onConfirm();

private:
    Ui::TaskPanel *ui;
    QList<Task> m_tasks;
    QMap<QString, QList<TaskSlot*>> m_taskSlots;
    Task *m_currentTask;

    void updateTaskSlots();
};

#endif // TASKPANEL_H
