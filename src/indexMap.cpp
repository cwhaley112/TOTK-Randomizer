#include "indexMap.h"
#include <oead/byml.h>

void readObjects(std::string file, std::unordered_map<std::string, unsigned int> &objectLookUp)
{
    std::ifstream gameData(file);
    std::string line;
    unsigned int key = 0;

    while (std::getline(gameData, line))
    {
        objectLookUp[trim(line)] = key; // fuck carriage returns
    }
}

void readObjectsToSet(std::string file, std::unordered_set<std::string> &objectLookUp)
{
    std::ifstream gameData(file);
    std::string line;

    while (std::getline(gameData, line))
    {
        objectLookUp.insert(trim(line));
    }
}

bool checkBymlFileExtension(std::filesystem::path file)
{
    std::string extension = file.extension().string();
    return !std::filesystem::is_directory(file) && (extension == ".bgyml" || extension == ".byml");
}

std::unordered_map<std::string, unsigned int> getTrackerLookupTable(GameObjTracker trackers[], const int nTrackers)
{
    std::unordered_map<std::string, unsigned int> trackerLookup;
    for (int i = 0; i < nTrackers; i++)
    {
        for (auto &[objName, count] : (*trackers[i].lookUpTable))
        {
            trackerLookup[objName] = i;
        }
    }

    return trackerLookup;
}

bool logFileData(std::string filePath, GameObjTracker trackers[], TrackedFile &fileData, std::unordered_map<std::string, unsigned int> &trackerLookup, int parentPathLen, WeaponClass weaponTypes)
{

    bool trackFile, loggedTracker, match;
    int i;
    YAML::Node yaml, actors, dynamic;
    oead::Byml byml;
    std::vector<u8> buffer;
    std::string gyaml, key, val;
    std::unordered_map<std::string, unsigned int>::iterator iter;
    GameObjTracker tracker, dynamicTracker;

    trackFile = false;
    loggedTracker = false;
    byml = openByml(filePath, buffer);
    yaml = YAML::Load(byml.ToText());
    actors = yaml["Actors"]; // sequence of maps

    for (i = 0; i < actors.size(); i++)
    {

        match = false;

        gyaml = actors[i]["Gyaml"].as<std::string>();
        iter = trackerLookup.find(gyaml);
        if (iter == trackerLookup.end())
            continue;

        tracker = trackers[iter->second];

        if (!trackFile)
        {
            fileData = {filePath.substr(parentPathLen), tracker.name};
            trackFile = true;
            loggedTracker = true;
        }
        else if (!loggedTracker)
        {
            fileData.matches.append(",");
            fileData.matches.append(tracker.name);
            loggedTracker = true;
        }

        (*tracker.lookUpTable)[gyaml] += 1;

        if (tracker.dynamicContains == NULL || actors[i]["Dynamic"].IsNull() || (tracker.name == "enemy" && weaponTypes.actorCandidates.find(gyaml) == weaponTypes.actorCandidates.end()))
            continue;

        dynamic = actors[i]["Dynamic"];
        dynamicTracker = *tracker.dynamicContains;

        for (YAML::const_iterator it = dynamic.begin(); it != dynamic.end(); ++it)
        {
            key = it->first.as<std::string>();
            // val not guaranteed to be string, have to check key
            if (dynamicTracker.name == "weapon" && (key == "EquipmentUser_Weapon" || key == "EquipmentUser_Shield" || key == "EquipmentUser_Bow"))
                val = it->second.as<std::string>();
            else
                continue;

            iter = (*dynamicTracker.lookUpTable).find(val);
            if (iter == (*dynamicTracker.lookUpTable).end())
                continue;

            match |= true;
            (*dynamicTracker.lookUpTable)[val] += 1;
        }
        if (!match)
            (*dynamicTracker.lookUpTable)["None"] += 1;
    }

    return trackFile;
}

void indexMapData(const std::filesystem::path romfsDir, std::vector<TrackedFile> &filesToEdit, GameObjTracker trackers[], const int nTrackers, WeaponClass weaponTypes)
{

    TrackedFile fileData;
    bool trackFile, loggedTracker;
    int i;

    std::filesystem::path bancDir = romfsDir / "Banc";
    int parentPathLen = bancDir.string().size() + 1; // +1 for '/'
    assert(std::filesystem::exists(bancDir));

    std::unordered_map<std::string, unsigned int> trackerLookup = getTrackerLookupTable(trackers, nTrackers);

    for (const auto &path_ : std::filesystem::recursive_directory_iterator(bancDir))
    {
        std::filesystem::path file = path_.path();

        if (!checkBymlFileExtension(file))
            continue;

        trackFile = logFileData(file.string(), trackers, fileData, trackerLookup, parentPathLen, weaponTypes);

        if (trackFile)
        {
            filesToEdit.push_back(fileData);
            std::cout << "Added: " << fileData.filePath << std::endl;
        }
    }
}

bool writeGameFileData(const std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit)
{
    std::filesystem::path toEditFile = targetDir / "filesToEdit.txt";

    std::ofstream file(toEditFile.string());
    if (!file.is_open())
        return false;

    std::string line;
    char buf[MAX_PATH_LEN];
    for (int i = 0; i < filesToEdit.size(); i++)
    {
        std::snprintf(buf, sizeof(buf), "%s %s\n", filesToEdit[i].filePath.c_str(), filesToEdit[i].matches.c_str());
        file << std::string(buf);
    }

    return true;
}

bool writeGameObjectData(const std::filesystem::path targetDir, const GameObjTracker tracker)
{
    char fname[MAX_PATH_LEN];
    std::snprintf(fname, sizeof(fname), "%s/%s.txt", targetDir.string().c_str(), tracker.name.c_str());

    std::ofstream file(fname);
    if (!file.is_open())
        return false;

    std::string line;
    char buf[MAX_PATH_LEN];
    for (auto &[obj, count] : (*tracker.lookUpTable))
    {
        std::snprintf(buf, sizeof(buf), "%s %s\n", obj.c_str(), std::to_string(count).c_str());
        file << buf;
    }

    return true;
}

bool writeGameData(const std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit, const GameObjTracker trackers[], const int nTrackers)
{

    bool wrote = true;

    wrote &= writeGameFileData(targetDir, filesToEdit);

    for (int i = 0; i < nTrackers; i++)
    {
        wrote &= writeGameObjectData(targetDir, trackers[i]);
    }

    return wrote;
}

bool readGameFileData(const std::filesystem::path dataDir, std::vector<TrackedFile> &filesToEdit)
{

    std::filesystem::path filePath = dataDir / "filesToEdit.txt";

    std::ifstream file(filePath.string());
    if (!file.is_open())
        return false;

    std::string line;
    TrackedFile fileData;
    int pos;
    while (std::getline(file, line))
    {
        line = trim(line);
        pos = line.find(" ");

        fileData = {line.substr(0, pos), line.substr(pos + 1)};
        filesToEdit.push_back(fileData);
    }

    return true;
}

bool readGameObjectData(const std::filesystem::path dataDir, GameObjTracker trackers[], const int nTrackers)
{

    char fname[MAX_PATH_LEN];
    char buf[MAX_PATH_LEN];
    GameObjTracker tracker;
    std::ifstream file;
    std::string line, objName;
    int pos;
    unsigned int count;

    for (int i = 0; i < nTrackers; i++)
    {
        tracker = trackers[i];
        std::snprintf(fname, sizeof(fname), "%s/%s.txt", dataDir.string().c_str(), tracker.name.c_str());

        file = std::ifstream(fname);
        if (!file.is_open())
            return false;

        while (std::getline(file, line))
        {
            line = trim(line);
            pos = line.find(" ");

            objName = line.substr(0, pos);
            count = std::stoi(line.substr(pos + 1));
            if ((*tracker.lookUpTable).find(objName) != (*tracker.lookUpTable).end())
                (*tracker.lookUpTable)[objName] = count;
        }
    }

    return true;
}