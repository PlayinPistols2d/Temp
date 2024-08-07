#include "task.h"

Task::Task(const QString &name)
    : m_name(name)
{
}

QString Task::name() const
{
    return m_name;
}

void Task::addRequirement(const QString &job, int count, RequirementType type)
{
    m_requirements[job] = qMakePair(count, type);
}

QMap<QString, QPair<int, Task::RequirementType>> Task::requirements() const
{
    return m_requirements;
}

QMap<QString, QList<QString>> Task::assignedEmployees() const
{
    return m_assignedEmployees;
}

void Task::assignEmployee(const QString &job, const QString &employeeId)
{
    if (!m_assignedEmployees[job].contains(employeeId)) {
        m_assignedEmployees[job].append(employeeId);
    }
}

void Task::unassignEmployee(const QString &job, const QString &employeeId)
{
    m_assignedEmployees[job].removeAll(employeeId);
}

bool Task::isComplete() const
{
    for (const QString &job : m_requirements.keys()) {
        if (m_requirements[job].second == Hard && m_assignedEmployees[job].size() < m_requirements[job].first) {
            return false;
        }
    }
    return true;
}
