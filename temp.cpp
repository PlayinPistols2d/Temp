class MyTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    MyTableModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}

    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if (!index.isValid() || role != Qt::EditRole)
            return false;

        // Проверяем, изменились ли данные
        if (value != data(index, role)) {
            // Устанавливаем новое значение
            emit userEdited(index, value);
        }

        return QAbstractTableModel::setData(index, value, role);
    }

signals:
    void userEdited(const QModelIndex &index, const QVariant &newValue);
};



