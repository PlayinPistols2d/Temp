#include <QList>
#include <QMap>
#include <QString>

QList<Node*> parseData(const QList<QMap<QString, QVariant>> &data) {
    QMap<int, Node*> nodeMap;  // Map to store all nodes by their ID
    QList<Node*> rootNodes;    // Store root nodes (with no parent)

    for (const auto& row : data) {
        int id = row["id"].toInt();
        QString name = row["name"].toString();
        int parentId = row["parent_id"].toInt();  // parent_id could be NULL or -1 for root elements
        int priority = row["priority"].toInt();

        Node* parentNode = nullptr;  // Assume null for root nodes

        if (parentId != -1 && !row["parent_id"].isNull()) {
            // If parentId is valid, find the parent node
            if (nodeMap.contains(parentId)) {
                parentNode = nodeMap[parentId];
            } else {
                // Handle error: parent not found (if the data is not well-formed)
                qWarning() << "Parent ID" << parentId << "not found for node" << id;
            }
        }

        // Create the node with a parent pointer
        Node* newNode = new Node(id, name, priority, parentNode);
        nodeMap[id] = newNode;  // Store the node in the map

        if (!parentNode) {
            // If it's a root node (no parent), store it separately
            rootNodes.append(newNode);
        }
    }

    return rootNodes;  // Return the list of root nodes
}