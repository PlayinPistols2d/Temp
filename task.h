#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QMap>
#include <QPair>
#include <QList>

class Task
{
public:
    enum RequirementType { Hard, Soft };

    Task(const QString &name);
    
    QString name() const;
    void addRequirement(const QString &job, int count, RequirementType type);

    QMap<QString, QPair<int, RequirementType>> requirements() const;
    QMap<QString, QList<QString>> assignedEmployees() const;

    void assignEmployee(const QString &job, const QString &employeeId);
    void unassignEmployee(const QString &job, const QString &employeeId);
    bool isComplete() const;

private:
    QString m_name;
    QMap<QString, QPair<int, RequirementType>> m_requirements; // job -> (count, type)
    QMap<QString, QList<QString>> m_assignedEmployees; // job -> list of employeeIds
};

#endif // TASK_H
