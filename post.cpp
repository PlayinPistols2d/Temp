#include "post.h"
#include "ui_post.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QVBoxLayout>

Post::Post(int postNumber, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Post),
    m_postNumber(postNumber)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->label_postNumber->setText(QString("Post #%1").arg(postNumber));
}

Post::~Post()
{
    delete ui;
}

void Post::addTaskCard(TaskCard *taskCard)
{
    ui->tasksLayout->addWidget(taskCard);
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
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}
