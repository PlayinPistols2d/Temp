QList<TaskCard*> parseTasksToTaskCards(const QList<QList<QVariant>>& rows, QWidget *parent = nullptr)
{
    QList<TaskCard*> taskCards;
    QMap<int, Task*> taskMap; // Map to store tasks by their ID
    Task* currentParentTask = nullptr;

    for (const QList<QVariant>& row : rows) {
        // Assuming row contains: Id, Name, Type, Group, Poid, Priority
        int id = row[0].toInt();
        QString name = row[1].toString();
        QString type = row[2].toString();
        QString group = row[3].toString();
        QVariant poidVariant = row[4]; // Can be null
        int priority = row[5].toInt();

        int poid = poidVariant.isNull() ? -1 : poidVariant.toInt();

        // Create the new task
        Task* task = new Task(id, name, type, group, priority);

        if (poid == -1) {
            // This is a root task
            if (currentParentTask != nullptr) {
                // If there's an existing parent task, finalize it by creating a TaskCard
                TaskCard* taskCard = new TaskCard(currentParentTask, parent);
                taskCards.append(taskCard);
            }
            // Set this as the new parent task
            currentParentTask = task;
        } else {
            // This is a child task, find its parent
            Task* parentTask = taskMap.value(poid, nullptr);
            if (parentTask) {
                parentTask->addChild(task);
                task->setParent(parentTask);
            } else {
                qWarning() << "Parent task with ID" << poid << "not found for task" << id;
            }
        }

        // Store the task in the map
        taskMap[id] = task;
    }

    // After the loop, create a TaskCard for the last parent task
    if (currentParentTask != nullptr) {
        TaskCard* taskCard = new TaskCard(currentParentTask, parent);
        taskCards.append(taskCard);
    }

    return taskCards;
}