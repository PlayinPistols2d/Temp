class OperationManager {
public:
    QMap<int, Operation*> operationMap;  // Map to store operations by ID
    QList<Operation*> rootOperations;    // List to store root operations (those with no parent)

    // Function to process one row and assign it to the correct parent
    void processRow(const QMap<QString, QVariant>& row) {
        int id = row["id"].toInt();
        QString name = row["name"].toString();
        int parentId = row["parent_id"].toInt();  // parent_id could be NULL or -1 for root elements
        int priority = row["priority"].toInt();

        Operation* parentNode = nullptr;  // Assume null for root nodes

        if (parentId != -1 && !row["parent_id"].isNull()) {
            // If parentId is valid, find the parent operation
            if (operationMap.contains(parentId)) {
                parentNode = operationMap[parentId];
            } else {
                qWarning() << "Parent ID" << parentId << "not found for operation" << id;
            }
        }

        // Create the operation
        Operation* newOperation = new Operation(id, name, priority, parentNode);

        if (parentNode) {
            // Add this operation as a child of the parent using the QMap
            parentNode->addChild(newOperation);
        } else {
            // If there's no parent, it's a root operation
            rootOperations.append(newOperation);
        }

        // Store this operation in the map
        operationMap[id] = newOperation;
    }
};