QJsonObject operationToJson(const Operation* operation) {
    QJsonObject jsonObject;
    jsonObject["id"] = operation->id;
    jsonObject["name"] = operation->name;
    jsonObject["priority"] = operation->priority;

    QJsonArray childrenArray;

    // Iterate through the children and recursively convert each child to a JSON object
    for (auto it = operation->children.begin(); it != operation->children.end(); ++it) {
        childrenArray.append(operationToJson(it.value()));
    }

    // If the operation has children, add them to the JSON object
    if (!childrenArray.isEmpty()) {
        jsonObject["children"] = childrenArray;
    }

    return jsonObject;
}




void saveOperationsToJson(const QList<Operation*>& rootOperations, const QString& filePath) {
    QJsonArray rootArray;

    // Convert each root operation to JSON and add it to the root array
    for (const Operation* rootOperation : rootOperations) {
        rootArray.append(operationToJson(rootOperation));
    }

    // Create a JSON document from the array of root operations
    QJsonDocument jsonDoc(rootArray);

    // Write the JSON document to the file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file to save operations.");
        return;
    }

    file.write(jsonDoc.toJson(QJsonDocument::Indented));
    file.close();
}



