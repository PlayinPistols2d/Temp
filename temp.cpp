void OperationManager::addOperation(const QJsonObject& operationData) {
    // Extract fields directly as int
    if (!operationData.contains("id")) {
        qWarning() << "Missing 'id' in operation data";
        return;
    }
    int id = operationData["id"].toInt();

    QString name = operationData.value("name").toString();

    int priority = operationData.value("priority").toInt(0);

    int parentId = -1; // Use -1 to represent null or root
    if (operationData.contains("parent_id")) {
        QJsonValue parentIdVal = operationData["parent_id"];
        if (!parentIdVal.isNull()) {
            parentId = parentIdVal.toInt();
        }
    }

    // Check if the operation already exists
    if (idToOperation.contains(id)) {
        qWarning() << "Operation with ID" << id << "already exists. Skipping.";
        return;
    }

    // Create a new Operation instance
    Operation* op = new Operation(id, name, priority, parentId);

    // Add to idToOperation map
    idToOperation.insert(id, op);

    // Set up parent-child relationships
    if (parentId != -1) {
        if (idToOperation.contains(parentId)) {
            // Parent already exists
            Operation* parentOp = idToOperation.value(parentId);
            op->parent = parentOp;
            parentOp->addChild(op);
        } else {
            // Parent not yet available; add to waiting list
            parentIdToChildrenWaiting.insert(parentId, op);
        }
    } else {
        // This is a root operation
        rootOperations.append(op);
    }

    // Check if this operation is a parent of any existing children waiting for it
    if (parentIdToChildrenWaiting.contains(id)) {
        // Get all children waiting for this parent
        QList<Operation*> waitingChildren = parentIdToChildrenWaiting.values(id);

        for (Operation* childOp : waitingChildren) {
            // Set parent for the child
            childOp->parent = op;
            // Add child to this operation's children list
            op->addChild(childOp);
        }

        // Remove from the waiting list
        parentIdToChildrenWaiting.remove(id);
    }
}