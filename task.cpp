#include "task.h"

Task::Task(const QString &name)
    : m_name(name)
{
}

QString Task::name() const
{
    return m_name;
}

void Task::addRequirement(const QString &job, RequirementType type)
{
    m_requirements[job] = type;
}

QMap<QString, Task::RequirementType> Task::requirements() const
{
    return m_requirements;
}

QMap<QString, QString> Task::assignedEmployees() const
{
    return m_assignedEmployees;
}

void Task::assignEmployee(const QString &job, const QString &employeeId)
{
    m_assignedEmployees[job] = employeeId;
}

void Task::unassignEmployee(const QString &job)
{
    m_assignedEmployees.remove(job);
}

bool Task::isComplete() const
{
    for (const auto &req : m_requirements.keys()) {
        if (m_requirements[req] == Hard && !m_assignedEmployees.contains(req)) {
            return false;
        }
    }
    return true;
}
