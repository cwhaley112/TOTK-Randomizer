#ifndef RANDOMIZE_H
#define RANDOMIZE_H

#include "NXFileIO.h"
#include "indexMap.h"
#include "yaml-cpp/yaml.h"
#include <iterator>
#include <oead/byml.h>
#include <random>
#include <unordered_map>
#include <vector>

typedef std::uniform_int_distribution<std::mt19937::result_type> UD;

void randomizeMap(const std::filesystem::path romfsDir, std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit, const GameObjTracker trackers[], const int nTrackers, EnemyClass &enemyTypes, WeaponClass &weaponTypes, WeaponLists &weaponLists, const bool chaos = false, const bool debug = false);

std::vector<unsigned int> createEditQueue(std::string matches, std::unordered_map<std::string, unsigned int> &getTrackerIx);

std::string randomizeEnemy(YAML::Node &actors, EnemyClass &enemyTypes, const int i, std::vector<std::vector<std::string>> &sampleData, UD &uniform, std::mt19937 &mt, const int enemyIx, const bool chaos);

void randomizeWeapon(YAML::Node &actors, const int i, std::vector<std::vector<std::string>> &sampleData, UD &uniform, std::mt19937 &mt, const unsigned int threshold, const int weaponIx, const bool chaos,
                     WeaponLists &weaponLists, WeaponClass &weaponTypes, const bool enemyWeapon, bool &canFuseWeapon, bool &canFuseShield, bool &canFuseArrow);

void giveEnemyWeapon(YAML::Node &actors, const int i, const std::string weapon, const bool isBow, const bool isShield);

std::vector<std::string> randomizeEnemyFusion(YAML::Node &actors, const int i, UD &uniform, std::mt19937 &mt, const unsigned int threshold, WeaponLists &weaponLists, WeaponClass &weaponTypes, bool canFuseWeapon,
                                              bool canFuseShield, bool canFuseArrow);

std::vector<std::string> randomizeStandaloneFusion(YAML::Node &actors, const int i, UD &uniform, std::mt19937 &mt, const unsigned int threshold, WeaponLists &weaponLists, WeaponClass &weaponTypes, bool canFuseWeapon,
                                                   bool canFuseShield);

void cullDynamic(YAML::Node &actors, const int i, const std::vector<std::string> toRemove);

std::vector<std::string> createSampleData(const GameObjTracker tracker, const bool chaos);

std::mt19937 initRNG(const bool debug);

UD createUniformDistribution(const int b, const int a = 0);

std::string getSample(std::vector<std::string> &sampleData, const int ix, const bool replacement);

std::string yamlToString(YAML::Node yaml);

#endif