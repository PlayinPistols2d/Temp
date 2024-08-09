#ifndef POST_H
#define POST_H

#include <QWidget>
#include <QList>
#include "task_card.h"

namespace Ui {
class Post;
}

class Post : public QWidget
{
    Q_OBJECT

public:
    explicit Post(int postNumber, QWidget *parent = nullptr);
    ~Post();

    void addTaskCard(TaskCard *taskCard);
    QList<TaskCard*> taskCards() const;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void taskCardAdded(TaskCard *taskCard);

private:
    Ui::Post *ui;
    int m_postNumber;
    QList<TaskCard*> m_taskCards;
};

#endif // POST_H
