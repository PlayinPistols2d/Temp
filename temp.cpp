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
        QJsonValue parentIdVal = operationData.value("parent_id");
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
    Operation op(id, name, priority, parentId);

    // Set up parent-child relationships
    if (parentId != -1) {
        if (idToOperation.contains(parentId)) {
            // Parent already exists
            Operation& parentOp = idToOperation[parentId];
            parentOp.childIds.append(id);
        } else {
            // Parent not yet available; add to waiting list
            parentIdToChildrenWaiting.insert(parentId, id);
        }
    }

    // Add to idToOperation map
    idToOperation.insert(id, op);

    // Check if this operation is a parent of any existing children waiting for it
    if (parentIdToChildrenWaiting.contains(id)) {
        // Get all children waiting for this parent
        QList<int> waitingChildren = parentIdToChildrenWaiting.values(id);

        for (int childId : waitingChildren) {
            // Update the child operation's parentId
            if (idToOperation.contains(childId)) {
                Operation& childOp = idToOperation[childId];
                childOp.parentId = id;
                // Add childId to this operation's children list
                idToOperation[id].childIds.append(childId);
            } else {
                qWarning() << "Child operation with ID" << childId << "not found in idToOperation.";
            }
        }

        // Remove from the waiting list
        parentIdToChildrenWaiting.remove(id);
    }
}