#include <QJsonObject>
#include <QString>
#include <QVariant>

// Helper function to insert key-value pairs into a QJsonObject
template <typename Key, typename Value>
void addKeyValue(QJsonObject &jsonObject, const Key &key, const Value &value) {
    jsonObject.insert(QString::fromStdString(std::to_string(key)), QJsonValue::fromVariant(QVariant::fromValue(value)));
}

// Variadic function to handle multiple key-value pairs
template <typename Key, typename Value, typename... Args>
void addKeyValue(QJsonObject &jsonObject, const Key &key, const Value &value, Args... args) {
    // Insert the current key-value pair
    addKeyValue(jsonObject, key, value);

    // Recurse for the remaining arguments
    addKeyValue(jsonObject, args...);
}

// Main function to create QJsonObject
template <typename... Args>
QJsonObject createJsonObject(Args... args) {
    QJsonObject jsonObject;
    addKeyValue(jsonObject, args...);
    return jsonObject;
}

// Usage example
int main() {
    // Create a QJsonObject with key-value pairs
    QJsonObject json = createJsonObject("name", "John", "age", 30, "city", "New York");

    // Print the QJsonObject
    for (const QString &key : json.keys()) {
        qDebug() << key << ":" << json[key].toString();
    }

    return 0;
}