#include "task_slot.h"
#include "ui_task_slot.h"

TaskSlot::TaskSlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskSlot),
    m_locked(false),
    m_employeeCard(nullptr)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    updateStyle();
}

TaskSlot::~TaskSlot()
{
    delete ui;
}

void TaskSlot::setJobRequirement(const QString &job)
{
    m_jobRequirement = job;
    updateStyle();
}

QString TaskSlot::jobRequirement() const
{
    return m_jobRequirement;
}

void TaskSlot::lockSlot()
{
    m_locked = true;
}

void TaskSlot::unlockSlot()
{
    m_locked = false;
}

void TaskSlot::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText() && event->mimeData()->text() == "EmployeeCard" && !m_locked) {
        event->acceptProposedAction();
        setStyleSheet("QWidget { border: 2px dashed green; border-radius: 10px; background-color: white; }");
    } else {
        event->ignore();
        setStyleSheet("QWidget { border: 2px dashed red; border-radius: 10px; background-color: white; }");
    }
}

void TaskSlot::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    updateStyle();
}

void TaskSlot::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasText() && event->mimeData()->text() == "EmployeeCard" && !m_locked) {
        EmployeeCard *employeeCard = qobject_cast<EmployeeCard*>(event->source());
        if (employeeCard && (m_jobRequirement.isEmpty() || employeeCard->jobPosition() == m_jobRequirement)) {
            event->setDropAction(Qt::MoveAction);
            event->accept();

            if (m_employeeCard) {
                m_employeeCard->setParent(nullptr);  // Remove current card
            }

            m_employeeCard = employeeCard;
            m_employeeCard->setParent(this);
            m_employeeCard->show();

            updateStyle();
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

void TaskSlot::updateStyle()
{
    if (m_employeeCard) {
        setStyleSheet("QWidget { border: 2px solid gray; border-radius: 10px; background-color: white; }");
    } else {
        setStyleSheet("QWidget { border: 2px dashed gray; border-radius: 10px; background-color: white; }");
    }
}
