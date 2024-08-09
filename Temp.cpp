QMap<QString, QPair<int, int>> TaskCard::getAllRequirements() const
{
    QMap<QString, QPair<int, int>> requirements;

    std::function<void(const Task*)> collectRequirements = [&](const Task* task) {
        for (const QString& job : task->requirements().keys()) {
            int hardCount = task->requirements()[job].first;
            int softCount = task->assignedEmployees()[job].size() - hardCount;

            if (requirements.contains(job)) {
                requirements[job].first += hardCount;
                requirements[job].second += softCount;
            } else {
                requirements[job] = qMakePair(hardCount, softCount);
            }
        }

        // Recursively collect requirements from child tasks
        for (const Task* childTask : task->children()) {
            collectRequirements(childTask);
        }
    };

    // Start collecting requirements from the main task
    collectRequirements(m_task);

    return requirements;
}
