#include "randomize.h"

void randomize(const std::filesystem::path romfsDir, std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit, const GameObjTracker trackers[], const int nTrackers, const bool chaos, const bool debug)
{

    std::filesystem::path bancDir = romfsDir / "Banc";
    targetDir = targetDir / "romfs/Banc";

    std::filesystem::create_directories(targetDir);

    std::vector<std::vector<std::string>> sampleData;

    // vars
    int i, j;
    std::filesystem::path toEdit, toSave;
    std::string matches, bymlText;
    std::vector<unsigned int> queue;
    std::vector<unsigned char> buffer;
    std::unordered_map<std::string, unsigned int> getTrackerIx;
    oead::Byml byml, newByml;

    UD uniform;
    std::mt19937 mt = initRNG(debug);

    j = 0; // to get min range of uniform dist
    for (i = 0; i < nTrackers; i++)
    {
        sampleData.push_back(createSampleData(trackers[i], chaos));
        getTrackerIx[trackers[i].name] = i;
        j += sampleData[i].size();
    }

    uniform = createUniformDistribution(j);

    for (i = 0; i < filesToEdit.size(); i++)
    {
        toEdit = bancDir / filesToEdit[i].filePath;
        toSave = targetDir / filesToEdit[i].filePath;
        std::cout << toSave.string() << std::endl;

        queue = createEditQueue(filesToEdit[i].matches, getTrackerIx);

        byml = openByml(toEdit.string(), buffer);
        bymlText = randomizeMap(byml.ToText(), sampleData, queue, trackers, uniform, mt, chaos);

        newByml = oead::Byml::FromText(bymlText);
        std::filesystem::create_directories(toSave.parent_path());
        saveByml(toSave.string(), newByml);
    }
}

std::vector<unsigned int> createEditQueue(std::string matches, std::unordered_map<std::string, unsigned int> &getTrackerIx)
{
    std::vector<unsigned int> queue;
    int strPos, matchPos;

    strPos = 0;
    while (true)
    {
        matchPos = matches.find(",", strPos);
        queue.push_back(getTrackerIx[matches.substr(strPos, matchPos - strPos)]);
        if (matchPos == std::string::npos)
            break;
        strPos = matchPos + 1;
    }

    return queue;
}

std::string randomizeMap(std::string bymlText, std::vector<std::vector<std::string>> &sampleData, std::vector<unsigned int> &queue, const GameObjTracker trackers[], UD uniform, std::mt19937 mt, const bool chaos)
{
    int trackerIx, j, cumulativeOffset, sampleIx;
    GameObjTracker tracker;
    std::string object, parsed, sample;
    std::smatch m;

    for (j = 0; j < queue.size(); j++)
    {
        trackerIx = queue[j];
        tracker = trackers[trackerIx];

        parsed = bymlText;
        cumulativeOffset = 0;
        while (std::regex_search(parsed, m, tracker.filePattern))
        {
            object = parsed.substr(m.position(), m.length());

            if ((*tracker.lookUpTable).find(object) != (*tracker.lookUpTable).end())
            {

                sampleIx = uniform(mt) % sampleData[trackerIx].size();
                sample = getSample(sampleData[trackerIx], sampleIx, chaos);

                // replace in string
                // std::cout << "object: " << object << std::endl << "replacing: " << bymlText.substr(m.position() + cumulativeOffset, object.size()) << std::endl << "with: " << sample << std::endl;
                bymlText.replace(m.position() + cumulativeOffset, object.size(), sample);
                cumulativeOffset += m.position() + sample.length();
            }
            else 
                cumulativeOffset += m.position() + m.length();

            parsed = m.suffix().str();
        }
    }

    return bymlText;
}

std::vector<std::string> createSampleData(const GameObjTracker tracker, const bool chaos)
{
    std::vector<std::string> sampleData;
    int i;

    if (chaos)
    {
        for (auto &[obj, count] : (*tracker.lookUpTable))
        {
            if (count > 0)
                sampleData.push_back(obj);
        }
    }
    else
    {
        for (auto &[obj, count] : (*tracker.lookUpTable))
        {
            for (i = 0; i < count; i++)
                sampleData.push_back(obj);
        }
    }

    return sampleData;
}

std::mt19937 initRNG(const bool debug)
{
    std::mt19937 mt;
    std::random_device dev;
    // mersenne twister? i hardly know her
    if (debug)
        mt = std::mt19937(0);
    else
        mt = std::mt19937(dev());

    return mt;
}

UD createUniformDistribution(const int b, const int a)
{
    return UD(a, b);
}

std::string getSample(std::vector<std::string> &sampleData, const int ix, const bool replacement)
{

    std::string ret;
    std::string tmp;
    ret = sampleData[ix];

    if (!replacement)
    {
        tmp = sampleData[sampleData.size() - 1];
        sampleData[ix] = tmp;
        sampleData.pop_back();
    }

    return ret;
}