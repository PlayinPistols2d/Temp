#include "task_panel.h"
#include "ui_task_panel.h"

TaskPanel::TaskPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskPanel),
    m_currentTask(nullptr)
{
    ui->setupUi(this);
    connect(ui->comboBox_tasks, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskPanel::onTaskChanged);
    connect(ui->button_confirm, &QPushButton::clicked, this, &TaskPanel::onConfirm);
}

TaskPanel::~TaskPanel()
{
    delete ui;
}

void TaskPanel::setTasks(const QList<Task> &tasks)
{
    m_tasks = tasks;
    ui->comboBox_tasks->clear();
    for (const Task &task : tasks) {
        ui->comboBox_tasks->addItem(task.name());
    }
    if (!tasks.isEmpty()) {
        onTaskChanged(0);
    }
}

void TaskPanel::onTaskChanged(int index)
{
    if (index < 0 || index >= m_tasks.size()) {
        return;
    }

    m_currentTask = &m_tasks[index];
    updateTaskSlots();
}

void TaskPanel::updateTaskSlots()
{
    // Clear existing task slots
    QLayoutItem *child;
    while ((child = ui->scrollAreaLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    m_taskSlots.clear();

    // Add task slots for current task
    for (const QString &job : m_currentTask->requirements().keys()) {
        int count = m_currentTask->requirements()[job].first;
        QLabel *jobLabel = new QLabel(job, this);
        jobLabel->setStyleSheet("font-weight: bold;");
        ui->scrollAreaLayout->addWidget(jobLabel);
        
        QList<TaskSlot*> slots;
        for (int i = 0; i < count; ++i) {
            TaskSlot *slot = new TaskSlot(this);
            slot->setJobRequirement(job);
            slot->setTaskName(m_currentTask->name());
            connect(slot, &TaskSlot::removeClicked, this, [=](EmployeeCard *card) {
                m_currentTask->unassignEmployee(job, card->employeeID());
                emit card->removeClicked(card);
            });
            slots.append(slot);
            ui->scrollAreaLayout->addWidget(slot);
        }
        m_taskSlots[job] = slots;
    }

    // Add already assigned employees to their respective slots
    for (const QString &job : m_currentTask->assignedEmployees().keys()) {
        const QList<QString> &employees = m_currentTask->assignedEmployees()[job];
        for (int i = 0; i < employees.size(); ++i) {
            if (i < m_taskSlots[job].size()) {
                EmployeeCard *card = new EmployeeCard(this);
                card->setEmployeeID(employees[i]);
                card->setEmployeeName("Mock Name");  // Use actual employee name in real case
                card->setEmployeePosition(job);
                m_taskSlots[job][i]->assignEmployeeCard(card);
            }
        }
    }
}

void TaskPanel::onConfirm()
{
    if (!m_currentTask) {
        return;
    }

    // Clear current assignments
    for (const QString &job : m_currentTask->requirements().keys()) {
        m_currentTask->assignedEmployees()[job].clear();
    }

    // Assign employees to task
    for (const QString &job : m_taskSlots.keys()) {
        for (TaskSlot *slot : m_taskSlots[job]) {
            if (slot->assignedEmployeeCard()) {
                m_currentTask->assignEmployee(slot->jobRequirement(), slot->assignedEmployeeCard()->employeeID());
            }
        }
    }
}
