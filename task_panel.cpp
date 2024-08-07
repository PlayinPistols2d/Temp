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
    while ((child = ui->taskSlotsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    m_taskSlots.clear();

    // Add task slots for current task
    for (const QString &job : m_currentTask->requirements().keys()) {
        TaskSlot *slot = new TaskSlot(this);
        slot->setJobRequirement(job);
        if (m_currentTask->assignedEmployees().contains(job)) {
            // Mocking an employee card creation for assigned employees
            EmployeeCard *card = new EmployeeCard(this);
            card->setEmployeeID(m_currentTask->assignedEmployees()[job]);
            card->setEmployeeName("Mock Name");  // Use actual employee name in real case
            card->setEmployeePosition(job);
            slot->assignEmployeeCard(card);
        }
        m_taskSlots[job] = slot;
        ui->taskSlotsLayout->addWidget(slot);
    }

    for (const auto &job : m_currentTask->assignedEmployees().keys()) {
        if (!m_taskSlots.contains(job)) {
            TaskSlot *slot = new TaskSlot(this);
            slot->setJobRequirement(job);
            EmployeeCard *card = new EmployeeCard(this);
            card->setEmployeeID(m_currentTask->assignedEmployees()[job]);
            card->setEmployeeName("Mock Name");  // Use actual employee name in real case
            card->setEmployeePosition(job);
            slot->assignEmployeeCard(card);
            m_taskSlots[job] = slot;
            ui->taskSlotsLayout->addWidget(slot);
        }
    }
}

void TaskPanel::onConfirm()
{
    if (!m_currentTask) {
        return;
    }

    for (const QString &job : m_taskSlots.keys()) {
        TaskSlot *slot = m_taskSlots[job];
        if (slot->assignedEmployeeCard()) {
            m_currentTask->assignEmployee(job, slot->assignedEmployeeCard()->employeeID());
        } else {
            m_currentTask->unassignEmployee(job);
        }
    }

    // Additional logic to handle confirmation and validation can be added here
}
