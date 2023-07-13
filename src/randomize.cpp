#include "randomize.h"

void randomizeMapFile(std::filesystem::path toEdit, std::filesystem::path toSave, const GameObjTracker trackers[], std::vector<std::vector<std::string>> &sampleData, UD &uniform, std::mt19937 &mt,
                      WeaponClass &weaponTypes, WeaponLists &weaponLists, const int enemyIx, const int weaponIx, const bool chaos, int threshold)
{

    int i;
    std::string gyaml, newEnemy;
    std::vector<std::string> toRemove;
    bool gotMatch, isEnemy, weaponsEnabled, canFuseWeapon, canFuseArrow, canFuseShield;
    std::vector<unsigned char> buffer;
    oead::Byml byml;
    YAML::Node yaml, actors;

    // not using, might in v2:
    // std::string matches;
    // std::vector<unsigned int> queue;
    // queue = createEditQueue(filesToEdit[i].matches, getTrackerIx);

    byml = openByml(toEdit.string(), buffer);
    yaml = YAML::Load(byml.ToText());
    actors = yaml["Actors"]; // sequence of maps

    for (i = 0; i < actors.size(); i++)
    {

        gotMatch = false;
        isEnemy = false;
        weaponsEnabled = false;
        canFuseWeapon = false;
        canFuseShield = false;
        canFuseArrow = false;
        toRemove.clear();

        gyaml = actors[i]["Gyaml"].as<std::string>();

        // enemy
        if ((*trackers[enemyIx].lookUpTable).find(gyaml) != (*trackers[enemyIx].lookUpTable).end())
        {
            gotMatch |= true;
            isEnemy = true;
            newEnemy = randomizeEnemy(actors, i, sampleData, uniform, mt, enemyIx, chaos);
            weaponsEnabled = weaponTypes.actorCandidates.find(newEnemy) != weaponTypes.actorCandidates.end();
        }

        // weapon
        if (weaponsEnabled || (*trackers[weaponIx].lookUpTable).find(gyaml) != (*trackers[weaponIx].lookUpTable).end())
        {
            gotMatch |= true;
            randomizeWeapon(actors, i, sampleData, uniform, mt, threshold, weaponIx, chaos, weaponLists, weaponTypes, weaponsEnabled, canFuseWeapon, canFuseShield, canFuseArrow);
        }

        if (!gotMatch)
            continue;

        // fusion
        if (isEnemy)
            toRemove = randomizeEnemyFusion(actors, i, uniform, mt, threshold, weaponLists, weaponTypes, canFuseWeapon, canFuseShield, canFuseArrow);
        else
            toRemove = randomizeStandaloneFusion(actors, i, uniform, mt, threshold, weaponLists, weaponTypes, canFuseWeapon, canFuseShield);

        cullDynamic(actors, i, toRemove);
    }

    byml = oead::Byml::FromText(yamlToString(yaml));

    std::filesystem::create_directories(toSave.parent_path());
    saveByml(toSave.string(), byml);
}

void randomizeMap(const std::filesystem::path romfsDir, std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit, const GameObjTracker trackers[], const int nTrackers,
                  WeaponClass &weaponTypes, WeaponLists &weaponLists, const bool chaos, const bool debug)
{

    std::filesystem::path bancDir = romfsDir / "Banc";
    targetDir = targetDir / "romfs/Banc";

    std::filesystem::create_directories(targetDir);

    // vars
    int i, distributionRange, threshold;
    unsigned int enemyIx, weaponIx;
    std::filesystem::path toEdit, toSave;
    std::vector<std::vector<std::string>> sampleData;
    std::unordered_map<std::string, unsigned int> getTrackerIx;

    // RNG stuff:
    UD uniform;
    std::mt19937 mt = initRNG(debug);
    distributionRange = 0;
    for (i = 0; i < nTrackers; i++)
    {
        sampleData.push_back(createSampleData(trackers[i], chaos));
        getTrackerIx[trackers[i].name] = i;
        distributionRange += sampleData[i].size();
    }
    uniform = createUniformDistribution(distributionRange);
    threshold = (int)(distributionRange * 0.5); // 50% chance for weapon/arrow fusions + whether enemy has sword && shield (vs just 1)

    // main loop
    enemyIx = getTrackerIx["enemy"];
    weaponIx = getTrackerIx["weapon"];

    for (i = 0; i < filesToEdit.size(); i++)
    {
        toEdit = bancDir / filesToEdit[i].filePath;
        toSave = targetDir / filesToEdit[i].filePath;
        std::cout << toSave.string() << std::endl;

        randomizeMapFile(toEdit, toSave, trackers, sampleData, uniform, mt, weaponTypes, weaponLists, enemyIx, weaponIx, chaos, threshold);
    }
}

std::string randomizeEnemy(YAML::Node &actors, const int i, std::vector<std::vector<std::string>> &sampleData, UD &uniform, std::mt19937 &mt, const int enemyIx, const bool chaos)
{

    int sampleIx = uniform(mt) % sampleData[enemyIx].size();
    std::string sample = getSample(sampleData[enemyIx], sampleIx, chaos);
    actors[i]["Gyaml"] = sample;

    // remove rails (they cause crashes for certain enemies)
    // there's a railsEnabled component in dynamic I might also need to remove
    if (actors[i]["Rails"])
        actors[i].remove("Rails");

    return sample;
}

void randomizeWeapon(YAML::Node &actors, const int i, std::vector<std::vector<std::string>> &sampleData, UD &uniform, std::mt19937 &mt, const int threshold, const int weaponIx, const bool chaos,
                     WeaponLists &weaponLists, WeaponClass &weaponTypes, const bool enemyWeapon, bool &canFuseWeapon, bool &canFuseShield, bool &canFuseArrow)
{
    std::string extraWeapon = "";
    bool extraWeaponIsShield = false;

    int sampleIx = uniform(mt) % sampleData[weaponIx].size();

    std::string sample = getSample(sampleData[weaponIx], sampleIx, chaos);

    if (sample == "None")
        return; // if !enemyWeapon, field weapon will just be unfused instead of removed

    if (weaponTypes.oneHanded.find(sample) != weaponTypes.oneHanded.end())
    {
        canFuseWeapon = true;

        // try shield
        if (enemyWeapon && uniform(mt) >= threshold)
        {
            sampleIx = uniform(mt) % weaponTypes.shield.size();
            extraWeapon = getSample(weaponLists.shield, sampleIx, true);
            extraWeaponIsShield = true;
            canFuseShield = true;
        }
    }
    else if (weaponTypes.shield.find(sample) != weaponTypes.shield.end())
    {
        canFuseShield = true;

        // try weapon
        if (uniform(mt) >= threshold)
        {
            sampleIx = uniform(mt) % weaponTypes.oneHanded.size();
            extraWeapon = getSample(weaponLists.oneHanded, sampleIx, true);
            extraWeaponIsShield = false;
            canFuseWeapon = true;
        }
    }
    else if (weaponTypes.twoHanded.find(sample) != weaponTypes.twoHanded.end())
    {
        canFuseWeapon = true;
    }
    else if (weaponTypes.bow.find(sample) != weaponTypes.bow.end())
    {
        canFuseArrow = true;
    }
    else
        std::cout << "mistakes have been made" << std::endl;

    if (!enemyWeapon)
        actors[i]["Gyaml"] = sample;
    else
    {
        giveEnemyWeapon(actors, i, sample, canFuseArrow, canFuseShield);
        if (extraWeapon.length() > 0)
            giveEnemyWeapon(actors, i, extraWeapon, false, extraWeaponIsShield);
    }
}

std::vector<std::string> randomizeEnemyFusion(YAML::Node &actors, const int i, UD &uniform, std::mt19937 &mt, const int threshold, WeaponLists &weaponLists, WeaponClass &weaponTypes, bool canFuseWeapon,
                                              bool canFuseShield, bool canFuseArrow)
{
    int sampleIx;
    std::string sample;
    std::vector<std::string> toRemove;

    if (canFuseWeapon && uniform(mt) >= threshold)
    {
        sampleIx = uniform(mt) % weaponTypes.weaponAttachments.size();
        sample = getSample(weaponLists.weaponAttachments, sampleIx, true);
        actors[i]["Dynamic"]["EquipmentUser_Attachment_Weapon"] = sample;
    }
    else if (canFuseWeapon)
        toRemove.push_back("EquipmentUser_Attachment_Weapon");
    else
    {
        toRemove.push_back("EquipmentUser_Attachment_Weapon");
        toRemove.push_back("EquipmentUser_Weapon");
    }

    if (canFuseShield && uniform(mt) >= threshold)
    {
        sampleIx = uniform(mt) % weaponTypes.weaponAttachments.size();
        sample = getSample(weaponLists.weaponAttachments, sampleIx, true);
        actors[i]["Dynamic"]["EquipmentUser_Attachment_Shield"] = sample;
    }
    else if (canFuseShield)
        toRemove.push_back("EquipmentUser_Attachment_Shield");
    else
    {
        toRemove.push_back("EquipmentUser_Attachment_Shield");
        toRemove.push_back("EquipmentUser_Shield");
    }

    if (canFuseArrow && uniform(mt) >= threshold)
    {
        sampleIx = uniform(mt) % weaponTypes.bowAttachments.size();
        sample = getSample(weaponLists.bowAttachments, sampleIx, true);
        actors[i]["Dynamic"]["EquipmentUser_Attachment_Arrow"] = sample;
    }
    else if (canFuseArrow)
        toRemove.push_back("EquipmentUser_Attachment_Arrow");
    else
    {
        toRemove.push_back("EquipmentUser_Attachment_Arrow");
        toRemove.push_back("EquipmentUser_Bow");
    }

    return toRemove;
}

std::vector<std::string> randomizeStandaloneFusion(YAML::Node &actors, const int i, UD &uniform, std::mt19937 &mt, const int threshold, WeaponLists &weaponLists, WeaponClass &weaponTypes, bool canFuseWeapon,
                                                   bool canFuseShield)
{
    int sampleIx;
    std::string sample;
    std::vector<std::string> toRemove;

    if (actors[i]["Dynamic"].IsNull())
        actors[i]["Dynamic"] = YAML::Load("{}");

    if ((canFuseWeapon || canFuseShield) && uniform(mt) >= threshold)
    {
        sampleIx = uniform(mt) % weaponTypes.weaponAttachments.size();
        sample = getSample(weaponLists.weaponAttachments, sampleIx, true);
        actors[i]["Dynamic"]["Equipment_Attachment"] = sample;
    }
    else
    {
        toRemove.push_back("Equipment_Attachment");
    }

    return toRemove;
}

void cullDynamic(YAML::Node &actors, const int i, const std::vector<std::string> toRemove)
{
    if (actors[i]["Dynamic"].IsNull())
        return;

    for (int j = 0; j < toRemove.size(); j++)
    {
        if (actors[i]["Dynamic"][toRemove[j]])
            actors[i]["Dynamic"].remove(toRemove[j]);
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

void giveEnemyWeapon(YAML::Node &actors, const int i, const std::string weapon, const bool isBow, const bool isShield)
{
    if (actors[i]["Dynamic"].IsNull())
        actors[i]["Dynamic"] = YAML::Load("{}");

    if (isBow)
    {
        actors[i]["Dynamic"]["EquipmentUser_Bow"] = weapon;
        actors[i]["Dynamic"]["Drop__DropTable"] = "Arrow";
    }
    else if (isShield)
    {
        actors[i]["Dynamic"]["EquipmentUser_Shield"] = weapon;
    }
    else
    {
        actors[i]["Dynamic"]["EquipmentUser_Weapon"] = weapon;
    }
}

std::vector<std::string> createSampleData(const GameObjTracker tracker, const bool chaos)
{
    std::vector<std::string> sampleData;
    int i;

    if (chaos)
    {
        for (auto &[obj, count] : (*tracker.lookUpTable))
        {
            if (count > 0 || tracker.name == "weapon")
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

std::string yamlToString(YAML::Node yaml)
{
    YAML::Emitter out;
    out << yaml;
    return std::string(out.c_str());
}