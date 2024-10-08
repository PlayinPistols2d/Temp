#ifndef POST_H
#define POST_H

#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QMap>
#include "task_card.h"
#include "employee_card.h"

namespace Ui {
class Post;
}

class Post : public QWidget
{
    Q_OBJECT

public:
    enum PostStatus { AwaitingTasks, Running, Finished };

    explicit Post(int postNumber, QWidget *parent = nullptr);
    ~Post();

    void addTaskCard(TaskCard *taskCard);
    QList<TaskCard*> taskCards() const;
    PostStatus status() const;

    void setStatus(PostStatus status);
    void setAssignedEmployees(const QMap<QString, QList<EmployeeCard*>>& employees);

public slots:
    void updatePostStatus();
    void onConfirmButtonClicked();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void taskCardAdded(TaskCard *taskCard);

private:
    Ui::Post *ui;
    int m_postNumber;
    QList<TaskCard*> m_taskCards;
    PostStatus m_status;
    QMap<QString, QList<EmployeeCard*>> m_assignedEmployees;
    QPushButton *confirmButton;

    QMap<QString, int> totalHardRequirements;
    QMap<QString, int> totalSoftRequirements;

    void updateConfirmButtonVisibility();
    void updatePostUI();
    void calculateRequirements();
    void updateRequirementsDisplay();
};

#endif // POST_H