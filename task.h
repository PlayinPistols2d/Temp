#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QMap>
#include <QPair>

class Task
{
public:
    enum RequirementType { Hard, Soft };

    Task(const QString &name);
    
    QString name() const;
    void addRequirement(const QString &job, RequirementType type);

    QMap<QString, RequirementType> requirements() const;
    QMap<QString, QString> assignedEmployees() const;

    void assignEmployee(const QString &job, const QString &employeeId);
    void unassignEmployee(const QString &job);
    bool isComplete() const;

private:
    QString m_name;
    QMap<QString, RequirementType> m_requirements;
    QMap<QString, QString> m_assignedEmployees; // job -> employeeId
};

#endif // TASK_H
