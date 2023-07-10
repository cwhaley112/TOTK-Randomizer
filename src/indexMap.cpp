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

bool checkFileExtension(std::filesystem::path file) {
    std::string extension = file.extension().string();
    return !std::filesystem::is_directory(file) && (extension == ".bgyml" || extension == ".byml");
}

void indexGameData(const std::filesystem::path romfsDir, std::vector<TrackedFile> &filesToEdit, GameObjTracker trackers[], const int nTrackers, const bool debug) {

    std::filesystem::path bancDir = romfsDir / "Banc";
    int parentPathLen = bancDir.string().size() + 1; // +1 for '/'
    assert(std::filesystem::exists(bancDir));

    oead::Byml byml;
    std::vector<u8> buffer;
    std::string parsed, bymlText, object;
    GameObjTracker tracker;
    TrackedFile fileData;
    std::smatch m;
    bool trackFile, loggedTracker;
    int i;
    // int j=0; // DEBUG

    for (const auto& path_ : std::filesystem::recursive_directory_iterator(bancDir)) {
        std::filesystem::path file = path_.path();
        
        if (!checkFileExtension(file))
            continue;
        
        trackFile = false;
        loggedTracker = false;
        byml = openByml(file.string(), buffer);
        bymlText = byml.ToText();

        for (i=0; i<nTrackers; i++) {
            tracker = trackers[i];
            parsed = bymlText;
            loggedTracker = (i==0 || tracker.category != trackers[i-1].category);
            while (std::regex_search(parsed, m, tracker.filePattern)) {
                object = parsed.substr(m.position(), m.length());
                
                if ((*tracker.lookUpTable).find(object) != (*tracker.lookUpTable).end()) {
                    
                    if (!trackFile) {
                        fileData = {file.string().substr(parentPathLen), tracker.name};
                        trackFile = true;
                        loggedTracker = true;
                    }
                    else if (!loggedTracker) {
                        fileData.matches.append(",");
                        fileData.matches.append(tracker.name);
                        loggedTracker = true;
                    }

                    (*tracker.lookUpTable)[object]+=1;
                }
                // else std::cout << object << " object matched regex but was not found in lookup table" << std::endl;

                parsed = m.suffix().str();
            }
        }

        if (trackFile) 
            filesToEdit.push_back(fileData);
    }
}

bool writeGameFileData(const std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit) {
    std::filesystem::path toEditFile = targetDir / "filesToEdit.txt";

    std::ofstream file(toEditFile.string());
    if (!file.is_open())
        return false;
    
    std::string line;
    char buf[MAX_PATH_LEN];
    for (int i=0; i<filesToEdit.size(); i++) {
        std::snprintf(buf, sizeof(buf), "%s %s\n", filesToEdit[i].filePath.c_str(), filesToEdit[i].matches.c_str());
        file << std::string(buf);
    }

    return true;
}

bool writeGameObjectData(const std::filesystem::path targetDir, const GameObjTracker tracker) {
    char fname[MAX_PATH_LEN];
    std::snprintf(fname, sizeof(fname), "%s/%s.txt", targetDir.string().c_str(), tracker.name.c_str());

    std::ofstream file(fname);
    if (!file.is_open())
        return false;
    
    std::string line;
    char buf[MAX_PATH_LEN];
    for (auto& [obj, count] : (*tracker.lookUpTable)) {
        std::snprintf(buf, sizeof(buf), "%s %s\n", obj.c_str(), std::to_string(count).c_str());
        file << buf;
    }

    return true;
}

bool writeGameData(const std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit, const GameObjTracker trackers[], const int nTrackers) {

    bool wrote = true;

    wrote &= writeGameFileData(targetDir, filesToEdit);
    
    for (int i=0; i<nTrackers; i++) {
        wrote &= writeGameObjectData(targetDir, trackers[i]);
    }

    return wrote;
}

bool readGameFileData(const std::filesystem::path dataDir, std::vector<TrackedFile> &filesToEdit) {
    
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

        fileData = {line.substr(0,pos), line.substr(pos+1)};
        filesToEdit.push_back(fileData);
    }

    return true;    
}

bool readGameObjectData(const std::filesystem::path dataDir, GameObjTracker trackers[], const int nTrackers) {

    char fname[MAX_PATH_LEN];
    char buf[MAX_PATH_LEN];
    GameObjTracker tracker;
    std::ifstream file;
    std::string line, objName;
    int pos;
    unsigned int count;

    for (int i=0; i<nTrackers; i++) {
        tracker = trackers[i];
        std::snprintf(fname, sizeof(fname), "%s/%s.txt", dataDir.string().c_str(), tracker.name.c_str());

        file = std::ifstream(fname);
        if (!file.is_open())
            return false;
        
        while (std::getline(file, line))
        {
            line = trim(line);
            pos = line.find(" ");

            objName = line.substr(0,pos);
            count = std::stoi(line.substr(pos+1));
            if ((*tracker.lookUpTable).find(objName) != (*tracker.lookUpTable).end())
                (*tracker.lookUpTable)[objName] = count; 
        }
    }

    return true;

}