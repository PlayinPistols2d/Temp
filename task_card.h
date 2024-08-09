#ifndef TASKCARD_H
#define TASKCARD_H

#include <QWidget>

namespace Ui {
class TaskCard;
}

class TaskCard : public QWidget
{
    Q_OBJECT

public:
    enum TaskStatus { NotStarted, Ongoing, Finished };

    explicit TaskCard(Task *task, QWidget *parent = nullptr);
    ~TaskCard();

    TaskStatus status() const;
    void setStatus(TaskStatus status);

private:
    Ui::TaskCard *ui;
    Task *m_task;
    TaskStatus m_status;

    void updateTaskCardUI();
};

#endif // TASKCARD_H
