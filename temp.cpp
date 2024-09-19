Operation* OperationManager::findOperationById(int id) {
    // Search in root operations
    for (Operation* op : rootOperations) {
        if (op->id == id) {
            return op;
        }
        // Search recursively in children
        Operation* foundOp = findOperationByIdRecursive(id, op);
        if (foundOp) {
            return foundOp;
        }
    }
    return nullptr;
}

Operation* OperationManager::findOperationByIdRecursive(int id, Operation* currentOp) {
    for (Operation* child : currentOp->children) {
        if (child->id == id) {
            return child;
        }
        Operation* foundOp = findOperationByIdRecursive(id, child);
        if (foundOp) {
            return foundOp;
        }
    }
    return nullptr;
}