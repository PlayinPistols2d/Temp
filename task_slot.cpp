#include "task_slot.h"
#include "ui_task_slot.h"

TaskSlot::TaskSlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskSlot),
    m_locked(false),
    m_employeeCard(nullptr),
    m_hardCount(0),
    m_softCount(0)
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
    ui->label_jobRequirement->setText(job);
    updateStyle();
}

QString TaskSlot::jobRequirement() const
{
    return m_jobRequirement;
}

void TaskSlot::setTaskName(const QString &taskName)
{
    ui->label_taskName->setText(taskName);
}

void TaskSlot::setEmployeeCapacity(int hardCount, int softCount)
{
    m_hardCount = hardCount;
    m_softCount = softCount;
}

int TaskSlot::hardEmployeeCount() const
{
    return m_hardCount;
}

int TaskSlot::softEmployeeCount() const
{
    return m_softCount;
}

void TaskSlot::lockSlot()
{
    m_locked = true;
}

void TaskSlot::unlockSlot()
{
    m_locked = false;
}

void TaskSlot::assignEmployeeCard(EmployeeCard *card)
{
    if (m_employeeCards.size() < (m_hardCount + m_softCount)) {
        m_employeeCards.append(card);
        ui->placeholder->layout()->addWidget(card);
        card->showRemoveButton(true);
        card->show();
        connect(card, &EmployeeCard::removeClicked, this, [=](EmployeeCard *card) {
            m_employeeCards.removeAll(card);
            card->setParent(nullptr);
            updateStyle();
            emit card->removeClicked(card);
        });
    }
    updateStyle();
}

EmployeeCard* TaskSlot::assignedEmployeeCard() const
{
    return m_employeeCard;
}

QList<EmployeeCard*> TaskSlot::assignedEmployeeCards() const
{
    return m_employeeCards;
}

void TaskSlot::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText() && event->mimeData()->text() == "EmployeeCard" && !m_locked) {
        EmployeeCard *employeeCard = qobject_cast<EmployeeCard*>(event->source());
        if (employeeCard && (m_jobRequirement.isEmpty() || employeeCard->jobPosition() == m_jobRequirement)) {
            event->acceptProposedAction();
            ui->placeholder->setStyleSheet("QWidget { border: 2px dashed green; border-radius: 10px; background-color: white; }");
        } else {
            event->ignore();
            ui->placeholder->setStyleSheet("QWidget { border: 2px dashed red; border-radius: 10px; background-color: white; }");
        }
    } else {
        event->ignore();
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
            assignEmployeeCard(employeeCard);
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

void TaskSlot::updateStyle()
{
    if (!m_employeeCards.isEmpty()) {
        ui->placeholder->setStyleSheet("QWidget { border: 2px solid gray; border-radius: 10px; background-color: white; }");
    } else {
        ui->placeholder->setStyleSheet("QWidget { border: 2px dashed gray; border-radius: 10px; background-color: white; }");
    }
}
