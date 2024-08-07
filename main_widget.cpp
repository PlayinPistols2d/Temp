#include "main_widget.h"
#include "ui_main_widget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    setupTestData();
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::setupTestData()
{
    // Create some employee cards
    for (int i = 1; i <= 10; ++i) {
        EmployeeCard *card = new EmployeeCard(this);
        card->setEmployeeID(QString("ID: %1").arg(i));
        card->setEmployeePicture(QPixmap(":/path/to/picture.png"));  // Replace with actual path
        card->setEmployeeName(QString("Employee %1").arg(i));
        card->setEmployeePosition(i % 2 == 0 ? "Developer" : "Designer");
        ui->sidePanel->addEmployeeCard(card);
    }

    // Create some tasks
    QList<Task> tasks;
    Task task1("Task 1");
    task1.addRequirement("Developer", 3, Task::Hard);
    task1.addRequirement("Designer", 2, Task::Soft);

    Task task2("Task 2");
    task2.addRequirement("Designer", 2, Task::Hard);

    tasks.append(task1);
    tasks.append(task2);

    ui->taskPanel->setTasks(tasks);
}
