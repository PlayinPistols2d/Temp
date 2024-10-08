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
                m_currentTask->unassignEmployee(job, card);
                ui->sidePanel->addEmployeeCard(card);
            });
            slots.append(slot);
            ui->scrollAreaLayout->addWidget(slot);
        }
        m_taskSlots[job] = slots;
    }

    // Add already assigned employees to their respective slots
    for (const QString &job : m_currentTask->assignedEmployees().keys()) {
        const QList<EmployeeCard*> &employees = m_currentTask->assignedEmployees()[job];
        for (int i = 0; i < employees.size(); ++i) {
            if (i < m_taskSlots[job].size()) {
                EmployeeCard *card = employees[i];
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
                m_currentTask->assignEmployee(slot->jobRequirement(), slot->assignedEmployeeCard());
            }
        }
    }

    // Additional logic to handle confirmation and validation can be added here
}
