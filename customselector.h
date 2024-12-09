// CustomSelector.h
#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QListView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QFrame>
#include <QEvent>
#include <QHelpEvent>
#include <QToolTip>

class CustomSelector : public QWidget
{
    Q_OBJECT
public:
    explicit CustomSelector(QWidget *parent = nullptr);

    void addItem(const QString &text, const QString &hint);
    void setItems(const QList<QPair<QString, QString>> &items);
    void clearItems();
    
    void setFilterCaseSensitivity(Qt::CaseSensitivity caseSensitivity);

    QString currentText() const;
    QString currentHint() const;

signals:
    void itemSelected(const QString &text, const QString &hint);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onTextChanged(const QString &text);
    void onItemClicked(const QModelIndex &index);
    void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    QLineEdit *m_lineEdit;
    QFrame *m_popupFrame;
    QListView *m_listView;
    QStandardItemModel *m_model;
    QSortFilterProxyModel *m_proxyModel;

    // To store hints separately for quick lookup; alternatively, store them directly in item data
    // e.g., item->setData(hint, Qt::UserRole)
    QString hintForIndex(const QModelIndex &index) const;
    void showPopup();
    void hidePopup();
    void updateHintDisplay(const QModelIndex &index);
};