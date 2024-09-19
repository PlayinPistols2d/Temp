void OperationManager::deleteOperations() {
    for (Operation* op : rootOperations) {
        deleteOperation(op);
    }
    rootOperations.clear();
}

void OperationManager::deleteOperation(Operation* op) {
    for (Operation* child : op->children) {
        deleteOperation(child);
    }
    delete op;
}