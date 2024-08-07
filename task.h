#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QMap>
#include <QPair>
#include <QList>
#include "employee_card.h"

class Task
{
public:
    enum RequirementType { Hard, Soft };

    Task(const QString &name);
    
    QString name() const;
    void addRequirement(const QString &job, int count, RequirementType type);
    void addChild(Task *child);

    QMap<QString, QPair<int, RequirementType>> requirements() const;
    QMap<QString, QList<EmployeeCard*>> assignedEmployees() const;
    QList<Task*> children() const;

    void assignEmployee(const QString &job, EmployeeCard *employee);
    void unassignEmployee(const QString &job, EmployeeCard *employee);
    bool isComplete() const;

private:
    QString m_name;
    QMap<QString, QPair<int, RequirementType>> m_requirements; // job -> (count, type)
    QMap<QString, QList<EmployeeCard*>> m_assignedEmployees; // job -> list of EmployeeCard*
    QList<Task*> m_children; // List of child tasks
};

#endif // TASK_H
