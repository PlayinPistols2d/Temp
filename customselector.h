#pragma once

#include <QLineEdit>
#include <QCompleter>
#include <QStandardItemModel>
#include <QToolTip>

class CustomLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit CustomLineEdit(QWidget *parent = nullptr);

    void addItem(const QString &text, const QString &hint);
    void setItems(const QList<QPair<QString, QString>> &items);
    void clearItems();
    void setFilterCaseSensitivity(Qt::CaseSensitivity cs);

signals:
    void itemSelected(const QString &text, const QString &hint);

protected:
    void focusInEvent(QFocusEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onCompletionSelected(const QString &completion);
    void onHighlighted(const QString &highlightedText);

private:
    void showCompleterPopup();
    QString hintForIndex(const QModelIndex &index) const;
    void showHintForIndex(const QModelIndex &index);

    QCompleter *m_completer;
    QStandardItemModel *m_model;
};