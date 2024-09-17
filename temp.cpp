QJsonObject operationToFlatJson(const Operation* operation) {
    QJsonObject jsonObject;

    // For the current operation, create a JSON object with its properties
    QJsonObject operationJson;
    operationJson["id"] = operation->id;
    operationJson["name"] = operation->name;
    operationJson["priority"] = operation->priority;

    // If the operation has a parent, include the parent's ID
    if (operation->parent) {
        operationJson["parent_id"] = operation->parent->id;
    } else {
        operationJson["parent_id"] = QJsonValue::Null;  // Null for root nodes
    }

    // Add this operation to the main JSON object using its ID as the key
    jsonObject[QString::number(operation->id)] = operationJson;

    // Recursively process all children and add them to the main object
    for (auto it = operation->children.begin(); it != operation->children.end(); ++it) {
        jsonObject.insert(operationToFlatJson(it.value()));
    }

    return jsonObject;
}




void saveOperationsToFlatJson(const QList<Operation*>& rootOperations, const QString& filePath) {
    QJsonObject allOperations;

    // Convert each root operation and all its descendants to a flat JSON object
    for (const Operation* rootOperation : rootOperations) {
        allOperations.insert(operationToFlatJson(rootOperation));
    }

    // Create a JSON document from the combined object
    QJsonDocument jsonDoc(allOperations);

    // Write the JSON document to the file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file to save operations.");
        return;
    }

    file.write(jsonDoc.toJson(QJsonDocument::Indented));
    file.close();
}