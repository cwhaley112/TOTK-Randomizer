#include "NXFileIO.h"

oead::Sarc openSarc(std::string filePath, std::vector<unsigned char> &buffer)
{
    std::ifstream input(filePath, std::ios::binary);
    buffer = std::vector<unsigned char>(std::istreambuf_iterator<char>(input), {}); // could run buffer.assign(args) instead I think?

    return oead::Sarc(buffer); // sarc stores buffer's address so cannot free buf
}

oead::Byml openBymlFromSarc(oead::Sarc sarcFile, unsigned int fileIx)
{
    oead::Sarc::File file = sarcFile.GetFile(fileIx);
    return oead::Byml::FromBinary(file.data);
}

oead::Byml openByml(std::string filePath, std::vector<unsigned char> &buffer)
{
    std::ifstream input(filePath, std::ios::binary);
    buffer = std::vector<unsigned char>(std::istreambuf_iterator<char>(input), {});

    return oead::Byml::FromBinary(buffer);
}

bool saveByml(std::string filePath, oead::Byml byml) {
    std::ofstream out(filePath, std::ios::binary);
    std::vector<unsigned char> buffer = byml.ToBinary(false, 7);
    std::copy(buffer.cbegin(), buffer.cend(), std::ostream_iterator<unsigned char>(out));

    return true; // TODO catch exception
}