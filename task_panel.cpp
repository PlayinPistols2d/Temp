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
        slot->setTaskName(m_currentTask->name());
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
            slot->setTaskName(m_currentTask->name());
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
