void TaskCard::setRequirementsFromMap(const QMap<QString, QPair<int, int>>& requirementsMap, 
                                      QMap<QString, int>& hardRequirements, 
                                      QMap<QString, int>& softRequirements) const
{
    // Clear existing data
    hardRequirements.clear();
    softRequirements.clear();

    // Iterate through the requirements map
    for (const QString& job : requirementsMap.keys()) {
        const QPair<int, int>& reqPair = requirementsMap[job];

        // Set up the hard requirements
        if (reqPair.first > 0) {
            hardRequirements[job] = reqPair.first;
        }

        // Set up the soft requirements
        if (reqPair.second > 0) {
            softRequirements[job] = reqPair.second;
        }
    }
}
