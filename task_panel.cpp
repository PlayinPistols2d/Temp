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
        int count = m_currentTask->requirements()[job].first;
        for (int i = 0; i < count; ++i) {
            TaskSlot *slot = new TaskSlot(this);
            slot->setJobRequirement(job);
            slot->setTaskName(m_currentTask->name());
            connect(slot, &TaskSlot::removeClicked, this, [=](EmployeeCard *card) {
                m_currentTask->unassignEmployee(job, card->employeeID());
                ui->sidePanel->addEmployeeCard(card);
            });
            m_taskSlots[job + QString::number(i)] = slot;
            ui->taskSlotsLayout->addWidget(slot);
        }
    }

    // Add already assigned employees to their respective slots
    for (const QString &job : m_currentTask->assignedEmployees().keys()) {
        const QList<QString> &employees = m_currentTask->assignedEmployees()[job];
        for (int i = 0; i < employees.size(); ++i) {
            if (m_taskSlots.contains(job + QString::number(i))) {
                EmployeeCard *card = new EmployeeCard(this);
                card->setEmployeeID(employees[i]);
                card->setEmployeeName("Mock Name");  // Use actual employee name in real case
                card->setEmployeePosition(job);
                m_taskSlots[job + QString::number(i)]->assignEmployeeCard(card);
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
    for (const QString &key : m_taskSlots.keys()) {
        TaskSlot *slot = m_taskSlots[key];
        if (slot->assignedEmployeeCard()) {
            m_currentTask->assignEmployee(slot->jobRequirement(), slot->assignedEmployeeCard()->employeeID());
        }
    }

    // Additional logic to handle confirmation and validation can be added here
}
