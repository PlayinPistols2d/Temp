#ifndef ASSIGNMENTWIDGET_H
#define ASSIGNMENTWIDGET_H

#include <QWidget>
#include "task_slot.h"
#include "employee_card.h"
#include "post.h"

namespace Ui {
class AssignmentWidget;
}

class AssignmentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AssignmentWidget(Post *post, QWidget *parent = nullptr);
    ~AssignmentWidget();

    // Function to open the widget modally
    void openModal();

private slots:
    void onConfirmButtonClicked();

private:
    Ui::AssignmentWidget *ui;
    Post *m_post;
    QList<TaskSlot*> m_taskSlots;
    QMap<QString, QList<EmployeeCard*>> assignedEmployees;

    void setupTaskSlots();
    void populateSidePanel();
    bool allSlotsFilled() const;
};

#endif // ASSIGNMENTWIDGET_H