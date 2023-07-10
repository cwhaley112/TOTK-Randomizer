#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>
#include "indexMap.h"
#include "randomize.h"


int main(int argc, char *argv[])
{
    std::string romfsPath = "/mnt/w/yuzu/dump/0100F2C0115B6000/decompressed";
    std::string targetPath = "..";
    bool chaos = false; // maintains distinct object counts
    bool debug = false; // fixes seed
    bool reindex = false;

    std::string curArg;
    if (argc>1) {
        for (int i=1; i<argc; i++) {
            curArg = std::string(argv[i]);
            if (curArg=="--chaos")
                chaos = true;
            else if (curArg=="--debug")
                debug = true;
            else if (curArg=="--reindex")
                reindex = true;
        }
    }

    std::cout << "chaos: " << chaos << " debug: " << debug << " reindex " << reindex << std::endl;

    // path data
    // TODO fix relative paths and hard-coded paths/params
    std::filesystem::path romfsDir(romfsPath); // path to DECOMPRESSED romfs directory
    std::filesystem::path targetDir(targetPath);
    std::filesystem::path gameData("../data");
    std::vector<TrackedFile> filesToEdit;

    // run these 4 lines for each thing being randomized
    // enemies
    std::unordered_map<std::string, unsigned int> enemyLookUp;
    readObjects("../data/unique/enemies.txt", enemyLookUp);
    std::regex enemyPattern("(Enemy_)[a-zA-Z0-9_]+");
    GameObjTracker enemies = {"enemy", "enemy", &enemyLookUp, enemyPattern};

    // weapons
    std::unordered_map<std::string, unsigned int> weaponLookUp;
    readObjects("../data/unique/allweapons.txt", weaponLookUp);
    std::regex weaponPattern("(Weapon_(Lsword|Spear|Sword|Shield|Bow)_)[a-zA-Z0-9_]+");
    GameObjTracker weapons = {"weapon", "enemy", &weaponLookUp, weaponPattern};
    

    GameObjTracker trackers[] = {
        enemies
        ,weapons
    };

    int nTrackers = sizeof(trackers)/sizeof(trackers[0]); // TIL you need to calc this in same scope that array is declared

    WeaponClass weaponTypes;
    readObjectsToSet("../data/unique/onehandedweapons.txt", weaponTypes.oneHanded);
    readObjectsToSet("../data/unique/twohandedweapons.txt", weaponTypes.twoHanded);
    readObjectsToSet("../data/unique/bows.txt", weaponTypes.bow);
    readObjectsToSet("../data/unique/shields.txt", weaponTypes.shield);
    readObjectsToSet("../data/unique/weaponattachments.txt", weaponTypes.weaponAttachments);
    readObjectsToSet("../data/unique/arrowattachments.txt", weaponTypes.bowAttachments);
    readObjectsToSet("../data/unique/canholdweapons.txt", weaponTypes.actorCandidates);

    WeaponLists weaponLists = {
        std::vector(weaponTypes.oneHanded.begin(), weaponTypes.oneHanded.end()),
        std::vector(weaponTypes.shield.begin(), weaponTypes.shield.end()),
        std::vector(weaponTypes.weaponAttachments.begin(), weaponTypes.weaponAttachments.end()),
        std::vector(weaponTypes.bowAttachments.begin(), weaponTypes.bowAttachments.end())
    };

    // try to read data from files
    if (reindex || !(readGameFileData(gameData, filesToEdit) && readGameObjectData(gameData, trackers, nTrackers)) // TODO test that exceptions are handled if files don't exist in orig banc folder (like if it gets moved)
    ) {
        if (!reindex)
            std::cout << "Couldn't read game object data from disk. ";
        std::cout << "Indexing files..." << std::endl;
        indexGameData(romfsDir, filesToEdit, trackers, nTrackers, debug);
        if (!writeGameData(gameData, filesToEdit, trackers, nTrackers))
            std::cout << "Couldn't write game object data to disk." << std::endl;
    }

    randomize(romfsDir, targetDir, filesToEdit, trackers, nTrackers, weaponTypes, weaponLists, chaos, debug);
}