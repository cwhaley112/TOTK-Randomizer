#ifndef RANDOMIZE_H
#define RANDOMIZE_H

#include "NXFileIO.h"
#include "indexMap.h"
#include "yaml-cpp/yaml.h"
#include <iterator>
#include <oead/byml.h>
#include <random>
#include <regex>
#include <unordered_map>
#include <vector>

typedef std::uniform_int_distribution<std::mt19937::result_type> UD;

/*
 * randomizes all objects tracked within trackers
 * chaos = equal weighting for all unique game objects
 */
void randomize(const std::filesystem::path romfsDir, std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit, const GameObjTracker trackers[], const int nTrackers, WeaponClass &weaponTypes, WeaponLists &weaponLists, const bool chaos = false, const bool debug = false);
std::vector<unsigned int> createEditQueue(std::string matches, std::unordered_map<std::string, unsigned int> &getTrackerIx);
void randomizeEnemies(YAML::Node &yaml, std::vector<std::vector<std::string>> &sampleData, std::unordered_map<std::string, unsigned int> &getTrackerIndex, const GameObjTracker trackers[], 
                             WeaponClass &weaponTypes, WeaponLists &weaponLists, UD &uniform, std::mt19937 &mt, const bool chaos, const int distributionRange);
std::vector<std::string> createSampleData(const GameObjTracker tracker, const bool chaos);
std::mt19937 initRNG(const bool debug);
UD createUniformDistribution(const int b, const int a = 0);
std::string getSample(std::vector<std::string> &sampleData, const int ix, const bool replacement);

#endif