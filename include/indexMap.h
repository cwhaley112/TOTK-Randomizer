#ifndef INDEXMAP_H
#define INDEXMAP_H

#include <filesystem>
#include <string>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include "NXFileIO.h"
#include <iostream>
#include "strUtil.h"

#define MAX_PATH_LEN 512

typedef struct {
    std::string name;
    std::string category;
    std::unordered_map<std::string, unsigned int> *lookUpTable;
    std::regex filePattern;
} GameObjTracker;

typedef struct {
    std::unordered_set<std::string> oneHanded;
    std::unordered_set<std::string> twoHanded;
    std::unordered_set<std::string> bow;
    std::unordered_set<std::string> shield;
    std::unordered_set<std::string> weaponAttachments;
    std::unordered_set<std::string> bowAttachments;
    std::unordered_set<std::string> actorCandidates;
} WeaponClass;

typedef struct {
    std::vector<std::string> oneHanded;
    std::vector<std::string> shield;
    std::vector<std::string> weaponAttachments;
    std::vector<std::string> bowAttachments;
} WeaponLists;

typedef struct {
    std::string filePath; // path relative to romfs/Banc
    std::string matches;
} TrackedFile;

void readObjects(std::string file, std::unordered_map<std::string, unsigned int> &objectLookUp);
void readObjectsToSet(std::string file, std::unordered_set<std::string> &objectLookUp);

/*
 * parses each byml file in romfs/Banc and logs which files have a tracked game object + tallies game object counts
*/
void indexGameData(const std::filesystem::path romfsDir, std::vector<TrackedFile> &filesToEdit, GameObjTracker trackers[], const int nTrackers, const bool debug = false);

/*
* writes 2+ files to disk:
* 1) filesToEdit.txt: a list of files to edit, where the 1st column is the file path (relative to romfs/Banc) 
     and the second column is the object Tracker name(s) which are present in the file (comma delimited)
* 2) for each object Tracker, creates a file called {trackerName}Counts.txt
     where each line has the game object name and the number found by indexGameData (space delimited)
*/
bool writeGameData(const std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit, const GameObjTracker trackers[], const int nTrackers);

bool readGameFileData(const std::filesystem::path dataDir, std::vector<TrackedFile> &filesToEdit);
bool readGameObjectData(const std::filesystem::path dataDir, GameObjTracker trackers[], const int nTrackers);

#endif