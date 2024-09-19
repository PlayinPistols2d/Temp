void OperationManager::addOperation(int id, const QString& name, int priority, int parentId) {
    // Check for duplicate IDs
    if (findOperationById(id)) {
        qWarning() << "Operation with ID" << id << "already exists. Skipping.";
        return;
    }

    // Create the new operation
    Operation* op = new Operation(id, name, priority);

    if (parentId == -1) {
        // No parent, this is a root operation
        rootOperations.insert(id, op);
    } else {
        // Try to find the parent in the existing tree
        Operation* parentOp = findOperationById(parentId);
        if (parentOp) {
            // Parent found, assign relationships
            op->parent = parentOp;
            parentOp->children.insert(id, op);
        } else {
            // Parent not found, add to waiting list
            waitingChildren[parentId].append(op);
        }
    }

    // Check if any operations are waiting for this operation as their parent
    if (waitingChildren.contains(id)) {
        QList<Operation*> childrenList = waitingChildren.take(id);
        for (Operation* childOp : childrenList) {
            childOp->parent = op;
            op->children.insert(childOp->id, childOp);
        }
    }
}