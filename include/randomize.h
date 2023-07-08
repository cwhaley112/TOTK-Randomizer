#ifndef RANDOMIZE_H
#define RANDOMIZE_H

#include "indexMap.h"
#include "NXFileIO.h"
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
void randomize(const std::filesystem::path romfsDir, std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit, const GameObjTracker trackers[], const int nTrackers, const bool chaos = false, const bool debug = false);
std::vector<std::string> createSampleData(const GameObjTracker tracker, const bool chaos);
std::mt19937 initRNG(const bool debug);
UD createUniformDistribution(const int b, const int a=0);
std::string getSample(std::vector<std::string> &sampleData, const int ix, const bool replacement);

#endif