#ifndef NXOPEN_H
#define NXOPEN_H

#include <algorithm>
#include <fstream>
#include <iterator>
#include <oead/byml.h>
#include <oead/sarc.h>
#include <string>

oead::Sarc openSarc(std::string filePath, std::vector<unsigned char> &buffer);

oead::Byml openBymlFromSarc(oead::Sarc sarcFile, unsigned int fileIx);

oead::Byml openByml(std::string filePath, std::vector<unsigned char> &buffer);

bool saveByml(std::string filePath, oead::Byml byml);

#endif