QList<Task*> parseTasksFromDatabase(const QList<QList<QVariant>>& rows)
{
    QList<Task*> taskList;
    QMap<int, Task*> taskMap; // To keep track of tasks by their ID

    Task* lastTask = nullptr;

    for (const QList<QVariant>& row : rows) {
        // Assuming row contains: Id, Name, Type, Group, Parent_Id, Priority
        int id = row[0].toInt();
        QString name = row[1].toString();
        QString type = row[2].toString();
        QString group = row[3].toString();
        int parentId = row[4].toInt(); // Assuming 0 or -1 for no parent
        int priority = row[5].toInt();

        // Create the new task
        Task* task = new Task(id, name, type, group, priority);

        // Add the task to the map for future reference
        taskMap[id] = task;

        if (parentId == 0 || parentId == -1) {
            // This is a root-level task, add it to the main list
            taskList.append(task);
        } else {
            // This task has a parent, find the parent task
            Task* parentTask = taskMap.value(parentId, nullptr);
            if (parentTask) {
                parentTask->addChild(task);
                task->setParent(parentTask);
            } else {
                qWarning() << "Parent task with ID" << parentId << "not found for task" << id;
            }
        }

        lastTask = task;
    }

    return taskList;
}