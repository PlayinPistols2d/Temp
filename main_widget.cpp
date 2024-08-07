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
    for (int i = 1; i <= 5; ++i) {
        EmployeeCard *card = new EmployeeCard(this);
        card->setEmployeeID(QString("ID: %1").arg(i));
        card->setEmployeePicture(QPixmap(":/path/to/picture.png"));  // Replace with actual path
        card->setEmployeeName(QString("Employee %1").arg(i));
        card->setEmployeePosition(i % 2 == 0 ? "Developer" : "Designer");
        ui->sidePanelLayout->addWidget(card);
    }

    // Create some tasks
    QList<Task> tasks;
    Task task1("Task 1");
    task1.addRequirement("Developer", Task::Hard);
    task1.addRequirement("Designer", Task::Soft);

    Task task2("Task 2");
    task2.addRequirement("Designer", Task::Hard);

    tasks.append(task1);
    tasks.append(task2);

    ui->taskPanel->setTasks(tasks);
}
