QList<Operation*> parseOperations(const QJsonArray& operationsData) {
    QMap<int, Operation*> idToOperation; // Map from ID to Operation pointer
    QList<Operation*> rootOperations;    // List of root operations (no parent)

    // First pass: Create Operation instances
    for (const QJsonValue& value : operationsData) {
        if (!value.isObject()) {
            qWarning() << "Invalid operation data, expected JSON object";
            continue;
        }
        QJsonObject obj = value.toObject();

        // Extract fields
        int id = obj.value("id").toInt(-1);
        QString name = obj.value("name").toString();
        int priority = obj.value("priority").toInt(0);
        QJsonValue parentIdVal = obj.value("parent_id");

        int parentId = -1; // Use -1 to represent null or root
        if (!parentIdVal.isNull()) {
            parentId = parentIdVal.toInt(-1);
        }

        if (id == -1) {
            qWarning() << "Invalid or missing 'id' in operation data";
            continue;
        }

        // Create Operation instance
        Operation* op = new Operation(id, name, priority, parentId);
        idToOperation.insert(id, op);
    }

    // Second pass: Assign parents and build hierarchy
    for (Operation* op : idToOperation) {
        int parentId = op->parentId;
        if (parentId == -1) {
            // This is a root operation
            rootOperations.append(op);
        } else {
            // Find parent operation
            Operation* parentOp = idToOperation.value(parentId, nullptr);
            if (parentOp) {
                op->parent = parentOp;
                parentOp->children.append(op);
            } else {
                qWarning() << "Parent with ID" << parentId << "not found for operation ID" << op->id;
                // Optionally, treat this as a root operation or handle as needed
                rootOperations.append(op);
            }
        }
    }

    return rootOperations;
}