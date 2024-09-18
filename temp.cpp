void OperationManager::addOperation(const QJsonObject& operationData) {
    // Extract fields
    int id = operationData.value("id").toInt(-1);
    QString name = operationData.value("name").toString();
    int priority = operationData.value("priority").toInt(0);
    QJsonValue parentIdVal = operationData.value("parent_id");

    int parentId = -1; // Use -1 to represent null or root
    if (!parentIdVal.isNull()) {
        parentId = parentIdVal.toInt(-1);
    }

    if (id == -1) {
        qWarning() << "Invalid or missing 'id' in operation data";
        return;
    }

    // Check if the operation already exists
    if (idToOperation.contains(id)) {
        qWarning() << "Operation with ID" << id << "already exists. Skipping.";
        return;
    }

    // Create a new Operation instance
    QSharedPointer<Operation> op = QSharedPointer<Operation>::create(id, name, priority, parentId);

    // Add to idToOperation map
    idToOperation.insert(id, op);

    // Set up parent-child relationships
    if (parentId != -1) {
        if (idToOperation.contains(parentId)) {
            // Parent already exists
            QSharedPointer<Operation> parentOp = idToOperation.value(parentId);
            op->parent = parentOp.toWeakRef();
            parentOp->addChild(op);
        } else {
            // Parent not yet available; add to waiting list
            parentIdToChildrenWaiting.insert(parentId, op);
        }
    }

    // Check if this operation is a parent of any existing children waiting for it
    if (parentIdToChildrenWaiting.contains(id)) {
        // Get all children waiting for this parent
        QList<QSharedPointer<Operation>> waitingChildren = parentIdToChildrenWaiting.values(id);

        for (QSharedPointer<Operation>& childOp : waitingChildren) {
            // Set parent for the child
            childOp->parent = op.toWeakRef();
            // Add child to this operation's children list
            op->addChild(childOp);
        }

        // Remove from the waiting list
        parentIdToChildrenWaiting.remove(id);
    }
}