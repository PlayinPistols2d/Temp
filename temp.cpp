QJsonObject buildJson(Operation* op) {
    QJsonObject opJson = op->toJson(); // Use your existing function

    if (!op->children.isEmpty()) {
        QJsonArray childrenArray;
        for (Operation* childOp : op->children) {
            childrenArray.append(buildJson(childOp));
        }
        opJson["children"] = childrenArray;
    }

    return opJson;
}



class OperationManager {
public:
    QMap<int, Operation*> idToOperation; // Map of operation ID to Operation*
    QMap<int, Operation*> rootOperations; // Map of root operation ID to Operation*
    QMap<int, QList<Operation*>> waitingChildren; // Map of parent ID to children waiting for their parent

    void addOperation(int id, const QString& name, int priority, int parentId);
    Operation* findOperationById(int id);
    QJsonObject toJson(int number, const QString& name); // New method
    void printOperations();
    void deleteOperations();

private:
    void printOperation(Operation* op, int depth);
    void deleteOperation(Operation* op);
};

// Implementation of toJson method
QJsonObject OperationManager::toJson(int number, const QString& name) {
    QJsonObject json;
    json["Number"] = number;
    json["Name"] = name;

    QJsonArray operationsArray;
    for (Operation* op : rootOperations) {
        operationsArray.append(buildJson(op));
    }

    json["Operations"] = operationsArray;
    return json;
}