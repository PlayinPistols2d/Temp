#include "post.h"
#include "ui_post.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QVBoxLayout>

Post::Post(int postNumber, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Post),
    m_postNumber(postNumber),
    m_status(AwaitingTasks)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->label_postNumber->setText(QString("Post #%1").arg(postNumber));

    // Initialize the confirm button but set it to invisible by default
    confirmButton = new QPushButton("Confirm", this);
    confirmButton->setVisible(false);
    ui->verticalLayout->addWidget(confirmButton);

    connect(confirmButton, &QPushButton::clicked, this, &Post::onConfirmButtonClicked);
}

Post::~Post()
{
    delete ui;
}

void Post::addTaskCard(TaskCard *taskCard)
{
    if (!m_taskCards.contains(taskCard)) {
        m_taskCards.append(taskCard);
        ui->tasksLayout->addWidget(taskCard);
        updateConfirmButtonVisibility();
    }
}

QList<TaskCard*> Post::taskCards() const
{
    return m_taskCards;
}

Post::PostStatus Post::status() const
{
    return m_status;
}

void Post::updatePostStatus()
{
    bool allFinished = true;
    bool allNotStarted = true;

    for (TaskCard *taskCard : m_taskCards) {
        if (taskCard->status() != TaskCard::Finished) {
            allFinished = false;
        }
        if (taskCard->status() != TaskCard::NotStarted) {
            allNotStarted = false;
        }
    }

    if (allFinished) {
        m_status = Finished;
    } else if (allNotStarted) {
        m_status = AwaitingTasks;
    } else {
        m_status = Running;
    }

    updatePostUI();
    updateConfirmButtonVisibility();
}

void Post::onConfirmButtonClicked()
{
    if (m_status == AwaitingTasks || m_status == Running) {
        // Create and display the AssignmentWidget modally
        AssignmentWidget *assignmentWidget = new AssignmentWidget(this);
        assignmentWidget->setWindowTitle("Assign Employees");
        assignmentWidget->openModal();
    }
}

void Post::updateConfirmButtonVisibility()
{
    confirmButton->setVisible(!m_taskCards.isEmpty() && m_status != Finished);
}

void Post::updatePostUI()
{
    switch (m_status) {
        case AwaitingTasks:
            this->setStyleSheet("background-color: lightgray;");
            break;
        case Running:
            this->setStyleSheet("background-color: yellow;");
            break;
        case Finished:
            this->setStyleSheet("background-color: lightgreen;");
            break;
    }
}

void Post::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText() && event->mimeData()->text() == "TaskCard") {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void Post::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasText() && event->mimeData()->text() == "TaskCard") {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void Post::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasText() && event->mimeData()->text() == "TaskCard") {
        TaskCard *taskCard = qobject_cast<TaskCard*>(event->source());
        if (taskCard) {
            addTaskCard(taskCard);
            qDebug() << "TaskCard added to Post #" << m_postNumber;
            emit taskCardAdded(taskCard);
            updatePostStatus();
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

void Post::setStatus(PostStatus status)
{
    m_status = status;
    updatePostUI();
}

void Post::setAssignedEmployees(const QMap<QString, QList<EmployeeCard*>>& employees)
{
    m_assignedEmployees = employees;
    qDebug() << "Employees assigned to Post #" << m_postNumber << ":";
    for (const QString& job : employees.keys()) {
        qDebug() << job << ": " << employees[job].size() << " employees";
    }
}

