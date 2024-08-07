#ifndef TASKSLOT_H
#define TASKSLOT_H

#include <QWidget>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QVBoxLayout>
#include "employee_card.h"

namespace Ui {
class TaskSlot;
}

class TaskSlot : public QWidget
{
    Q_OBJECT

public:
    explicit TaskSlot(QWidget *parent = nullptr);
    ~TaskSlot();

    void setJobRequirement(const QString &job);
    QString jobRequirement() const;
    void setTaskName(const QString &taskName);
    void lockSlot();
    void unlockSlot();
    void assignEmployeeCard(EmployeeCard *card);
    EmployeeCard* assignedEmployeeCard() const;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Ui::TaskSlot *ui;
    QString m_jobRequirement;
    bool m_locked;
    EmployeeCard *m_employeeCard;

    void updateStyle();
};

#endif // TASKSLOT_H
