QJsonObject operationToJson(const Operation* operation) {
    QJsonObject jsonObject;

    // Add basic properties of the operation
    jsonObject["id"] = operation->id;
    jsonObject["name"] = operation->name;
    jsonObject["priority"] = operation->priority;

    // If the operation has a parent, include the parent's ID
    if (operation->parent) {
        jsonObject["parent_id"] = operation->parent->id;
    } else {
        jsonObject["parent_id"] = QJsonValue::Null;  // Null for root nodes
    }

    // Process children recursively, if any, and add them to "operations"
    if (!operation->children.isEmpty()) {
        QJsonArray childrenArray;

        for (auto it = operation->children.begin(); it != operation->children.end(); ++it) {
            childrenArray.append(operationToJson(it.value()));  // Recursively convert child operations
        }

        jsonObject["operations"] = childrenArray;  // Add the child operations as an array
    }

    return jsonObject;
}




QJsonObject generateOperationsJson(const QList<Operation*>& rootOperations) {
    QJsonObject allOperationsJson;

    // Loop through all root operations and convert them to JSON
    for (const Operation* rootOperation : rootOperations) {
        QJsonObject rootJson = operationToJson(rootOperation);

        // Add each root operation as a top-level key in the final JSON object, using its ID as the key
        allOperationsJson[QString::number(rootOperation->id)] = rootJson;
    }

    return allOperationsJson;
}