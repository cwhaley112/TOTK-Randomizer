#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>
#include "indexMap.h"
#include "randomize.h"
// #include <boost/program_options.hpp>
// namespace po = boost::program_options;


int main(int argc, char *argv[])
{
    std::string romfsPath = "/mnt/w/yuzu/dump/0100F2C0115B6000/decompressed";
    std::string targetPath = "..";
    bool chaos = false; // maintains distinct object counts
    bool debug = true; // fixes seed
    bool reindex = true;

    // po::options_description desc("Program Options");
    // desc.add_options()
    //     ("romfs", po::value<std::string>(romfsPath), "path to *decompressed* romfs directory")
    //     ("target,t", po::value<std::string>(targetPath)->default_value("."), "path where new romfs directory will be saved")
    //     ("chaos,c", po::value<bool>(chaos)->default_value(false), "chaos mode: all enemies are equally likely")
    //     ("debug", po::value<bool>(debug)->default_value(false), "fixes random seed")
    //     ("reindex", po::value<bool>(reindex)->default_value(false), "iterate through romfs directory and index files that should change")
    //     ;
    // po::variables_map vmap;
    // po::store(po::parse_command_line(argc, argv, desc), vmap);
    // po::notify(vmap);

    // path data
    // TODO fix relative paths and hard-coded paths/params
    std::filesystem::path romfsDir(romfsPath); // path to DECOMPRESSED romfs directory
    std::filesystem::path targetDir(targetPath);
    std::filesystem::path gameData("../data");
    std::vector<TrackedFile> filesToEdit;

    // run these 4 lines for each thing being randomized
    std::unordered_map<std::string, unsigned int> enemyLookUp;
    readObjects("../data/unique/enemies.txt", enemyLookUp);
    std::regex enemyPattern("(Enemy_)[a-zA-Z0-9_]+");
    GameObjTracker enemies = {"enemy", &enemyLookUp, enemyPattern};

    // TODO bugfix: weapons are prepended with "EquipmentUser_{weapontype}" --> need to replace that too
    // std::unordered_map<std::string, unsigned int> weaponLookUp;
    // readObjects("../data/unique/weapons.txt", weaponLookUp);
    // std::regex weaponPattern("(Weapon_(Lsword|Spear|Sword|Shield|Bow)_)[a-zA-Z0-9_]+");
    // GameObjTracker weapons = {"weapon", &weaponLookUp, weaponPattern};
    

    GameObjTracker trackers[] = {
        enemies
        // ,weapons
    };

    int nTrackers = sizeof(trackers)/sizeof(trackers[0]); // TIL you need to calc this in same scope that array is declared

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

    randomize(romfsDir, targetDir, filesToEdit, trackers, nTrackers, chaos, debug);
}