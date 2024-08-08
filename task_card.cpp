#include "task_card.h"
#include "ui_task_card.h"

TaskCard::TaskCard(Task *task, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskCard),
    m_task(task)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    updateTree();
    updateEmployeeCounts();
    updateRequirementsTree();
}

TaskCard::~TaskCard()
{
    delete ui;
}

void TaskCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void TaskCard::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setText("TaskCard");
    drag->setMimeData(mimeData);

    QPixmap pixmap = this->grab();
    mimeData->setImageData(pixmap);
    drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos()); // Set cursor in the middle of the card

    drag->exec(Qt::MoveAction);
}

void TaskCard::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText() && event->mimeData()->text() == "EmployeeCard") {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void TaskCard::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasText() && event->mimeData()->text() == "EmployeeCard") {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void TaskCard::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasText() && event->mimeData()->text() == "EmployeeCard") {
        EmployeeCard *employeeCard = qobject_cast<EmployeeCard*>(event->source());
        if (employeeCard) {
            // Add employee to the task
            m_task->assignEmployee(employeeCard->jobPosition(), employeeCard);
            updateTree();
            updateEmployeeCounts();
            updateRequirementsTree();
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

void TaskCard::populateTree(Task *task, QTreeWidgetItem *parentItem)
{
    QTreeWidgetItem *taskItem = new QTreeWidgetItem(parentItem);
    taskItem->setText(0, task->name());
    parentItem->addChild(taskItem);

    for (Task *childTask : task->children()) {
        populateTree(childTask, taskItem);
    }
}

void TaskCard::updateTree()
{
    ui->treeWidget_tasks->clear();
    populateTree(m_task, ui->treeWidget_tasks->invisibleRootItem());
    ui->treeWidget_tasks->expandAll();
}

void TaskCard::updateEmployeeCounts()
{
    int hardCount = 0;
    int totalCount = 0;

    std::function<void(Task*)> calculateCounts = [&](Task *task) {
        for (const QString &job : task->requirements().keys()) {
            int hard = task->requirements()[job].first;
            int soft = task->assignedEmployees()[job].size() - hard;
            hardCount += hard;
            totalCount += (hard + soft);
        }
        for (Task *childTask : task->children()) {
            calculateCounts(childTask);
        }
    };

    calculateCounts(m_task);

    ui->label_minEmployees->setText("Required Min: " + QString::number(hardCount) + " employees");
    ui->label_maxEmployees->setText("Required Max: " + QString::number(totalCount) + " employees");
}

void TaskCard::updateRequirementsTree()
{
    ui->treeWidget_requirements->clear();

    QTreeWidgetItem *requirementsRoot = new QTreeWidgetItem(ui->treeWidget_requirements);
    requirementsRoot->setText(0, "Requirements");
    ui->treeWidget_requirements->addTopLevelItem(requirementsRoot);

    QTreeWidgetItem *hardRoot = new QTreeWidgetItem(requirementsRoot);
    hardRoot->setText(0, "Hard Requirements");

    QTreeWidgetItem *softRoot = new QTreeWidgetItem(requirementsRoot);
    softRoot->setText(0, "Soft Requirements");

    QMap<QString, int> hardRequirements;
    QMap<QString, int> softRequirements;

    std::function<void(Task*)> populateRequirements = [&](Task *task) {
        for (const QString &job : task->requirements().keys()) {
            int hard = task->requirements()[job].first;
            int soft = task->assignedEmployees()[job].size() - hard;

            if (hard > 0) {
                hardRequirements[job] += hard;
            }

            if (soft > 0) {
                softRequirements[job] += soft;
            }
        }

        for (Task *childTask : task->children()) {
            populateRequirements(childTask);
        }
    };

    populateRequirements(m_task);

    for (const QString &job : hardRequirements.keys()) {
        QTreeWidgetItem *hardItem = new QTreeWidgetItem(hardRoot);
        hardItem->setText(0, job + ": " + QString::number(hardRequirements[job]));
    }

    for (const QString &job : softRequirements.keys()) {
        QTreeWidgetItem *softItem = new QTreeWidgetItem(softRoot);
        softItem->setText(0, job + ": " + QString::number(softRequirements[job]));
    }

    hardRoot->setText(0, "Hard Requirements (" + QString::number(hardRequirements.values().size()) + ")");
    softRoot->setText(0, "Soft Requirements (" + QString::number(softRequirements.values().size()) + ")");

    ui->treeWidget_requirements->expandAll();
}
