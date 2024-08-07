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
    if (m_employeeCard) {
        m_employeeCard->setParent(nullptr);  // Remove current card
    }

    m_employeeCard = card;
    m_employeeCard->setParent(this);
    m_employeeCard->showRemoveButton(true);
    ui->placeholder->layout()->addWidget(m_employeeCard);
    m_employeeCard->show();
    connect(m_employeeCard, &EmployeeCard::removeClicked, this, [=](EmployeeCard *card) {
        if (m_employeeCard == card) {
            m_employeeCard->setParent(nullptr);
            m_employeeCard = nullptr;
            updateStyle();
            emit card->removeClicked(card); // Emit the removeClicked signal
        }
    });
    updateStyle();
}

EmployeeCard* TaskSlot::assignedEmployeeCard() const
{
    return m_employeeCard;
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
        if (employeeCard && (m_jobRequirement.isEmpty() || employeeCard->jobPosition() == m
