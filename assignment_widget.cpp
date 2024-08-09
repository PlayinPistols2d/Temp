#include "assignment_widget.h"
#include "ui_assignment_widget.h"
#include <QDebug>

AssignmentWidget::AssignmentWidget(Post *post, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssignmentWidget),
    m_post(post)
{
    ui->setupUi(this);
    setupTaskSlots();
    populateSidePanel();

    connect(ui->confirmButton, &QPushButton::clicked, this, &AssignmentWidget::onConfirmButtonClicked);
}

AssignmentWidget::~AssignmentWidget()
{
    delete ui;
}

void AssignmentWidget::setupTaskSlots()
{
    // For each job position and requirement, create the appropriate number of task slots
    for (const QString &job : m_post->totalHardRequirements.keys()) {
        int hardCount = m_post->totalHardRequirements[job];
        for (int i = 0; i < hardCount; ++i) {
            TaskSlot *slot = new TaskSlot(this);
            slot->setJobRequirement(job);
            slot->setTaskName(job + " Slot " + QString::number(i + 1));
            m_taskSlots.append(slot);
            ui->taskSlotsLayout->addWidget(slot);
        }
    }

    for (const QString &job : m_post->totalSoftRequirements.keys()) {
        int softCount = m_post->totalSoftRequirements[job];
        for (int i = 0; i < softCount; ++i) {
            TaskSlot *slot = new TaskSlot(this);
            slot->setJobRequirement(job);
            slot->setTaskName(job + " Slot " + QString::number(i + 1));
            m_taskSlots.append(slot);
            ui->taskSlotsLayout->addWidget(slot);
        }
    }
}

void AssignmentWidget::populateSidePanel()
{
    // Populate the side panel with all available employee cards
    for (int i = 1; i <= 10; ++i) {
        EmployeeCard *card = new EmployeeCard(this);
        card->setEmployeeID(QString("ID: %1").arg(i));
        card->setEmployeePicture(QPixmap(":/path/to/picture.png"));  // Replace with actual path
        card->setEmployeeName(QString("Employee %1").arg(i));
        card->setEmployeePosition(i % 2 == 0 ? "Developer" : "Designer");
        ui->sidePanelLayout->addWidget(card);
    }
}

bool AssignmentWidget::allSlotsFilled() const
{
    for (TaskSlot *slot : m_taskSlots) {
        if (!slot->assignedEmployeeCard()) {
            return false;
        }
    }
    return true;
}

void AssignmentWidget::onConfirmButtonClicked()
{
    if (!allSlotsFilled()) {
        qDebug() << "All slots must be filled before confirming.";
        return;
    }

    // Confirm the assignment
    qDebug() << "All slots filled. Confirming assignment.";

    // Update the post status to running
    m_post->setStatus(Post::Running);

    // Close the assignment widget
    this->close();
}
