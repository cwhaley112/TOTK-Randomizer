#include "randomize.h"

void randomize(const std::filesystem::path romfsDir, std::filesystem::path targetDir, const std::vector<TrackedFile> &filesToEdit, const GameObjTracker trackers[], const int nTrackers, 
               WeaponClass &weaponTypes, WeaponLists &weaponLists, const bool chaos, const bool debug)
{

    std::filesystem::path bancDir = romfsDir / "Banc";
    targetDir = targetDir / "romfs/Banc";

    std::filesystem::create_directories(targetDir);

    // vars
    int i, distributionRange;
    std::filesystem::path toEdit, toSave;
    std::string matches;
    std::vector<unsigned int> queue;
    std::vector<unsigned char> buffer;
    std::vector<std::string> allWeapons;
    std::vector<std::vector<std::string>> sampleData;
    std::unordered_map<std::string, unsigned int> getTrackerIx;
    oead::Byml byml;
    YAML::Node yaml;
    YAML::Emitter out;

    UD uniform;
    std::mt19937 mt = initRNG(debug);

    distributionRange = 0; // to get min range of uniform dist
    for (i = 0; i < nTrackers; i++)
    {
        sampleData.push_back(createSampleData(trackers[i], chaos));
        getTrackerIx[trackers[i].name] = i;
        distributionRange += sampleData[i].size();
    }

    uniform = createUniformDistribution(distributionRange);

    for (i = 0; i < filesToEdit.size(); i++)
    {
        toEdit = bancDir / filesToEdit[i].filePath;
        toSave = targetDir / filesToEdit[i].filePath;
        std::cout << toSave.string() << std::endl;

        // queue = createEditQueue(filesToEdit[i].matches, getTrackerIx);

        byml = openByml(toEdit.string(), buffer);
        yaml = YAML::Load(byml.ToText());
        randomizeEnemies(yaml, sampleData, getTrackerIx, trackers, weaponTypes, weaponLists, uniform, mt, chaos, distributionRange);

        out << yaml;
        byml = oead::Byml::FromText(std::string(out.c_str()));
        std::filesystem::create_directories(toSave.parent_path());
        saveByml(toSave.string(), byml);
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

void randomizeEnemies(YAML::Node &yaml, std::vector<std::vector<std::string>> &sampleData, std::unordered_map<std::string, unsigned int> &getTrackerIndex, const GameObjTracker trackers[], 
                             WeaponClass &weaponTypes, WeaponLists &weaponLists, UD &uniform, std::mt19937 &mt, const bool chaos, const int distributionRange)
{
    int i, j, sampleIx, threshold;
    unsigned int enemyIx, weaponIx, weaponAttachIx, bowAttachIx;
    std::string object, sample;
    YAML::Node actors;
    bool weaponsEnabled, canFuseWeapon, canFuseArrow, canFuseShield;
    std::vector<std::string> toRemove;

    actors = yaml["Actors"]; // sequence of maps

    threshold = (int)(distributionRange * 0.5); // 50% chance for weapon/arrow fusions + whether enemy has sword && shield (vs just 1)

    enemyIx = getTrackerIndex["enemy"];
    weaponIx = getTrackerIndex["weapon"];

    for (i=0; i<actors.size(); i++) {
        object = actors[i]["Gyaml"].as<std::string>();
        if ((*trackers[enemyIx].lookUpTable).find(object) == (*trackers[enemyIx].lookUpTable).end())
            continue;

        sampleIx = uniform(mt) % sampleData[enemyIx].size();
        sample = getSample(sampleData[enemyIx], sampleIx, chaos);
        actors[i]["Gyaml"] = sample;

        // remove rails (they cause crashes for certain enemies)
        // there's a railsenabled component in dynamic I might also need to remove
        if (actors[i]["Rails"])
            actors[i].remove("Rails");

        if (actors[i]["Dynamic"].IsNull()) {
            std::cout << "No dynamic map for " << sample << std::endl;
        }


        // randomize weapons;
        // TODO make this work with non chaos mode (need to refactor indexMap fxn to count number of times there is *not* a weapon/bow/shield)

        // this is so messy

        toRemove.clear();
        canFuseWeapon = false;
        canFuseShield = false;
        canFuseArrow = false;
        weaponsEnabled = weaponTypes.actorCandidates.find(sample) != weaponTypes.actorCandidates.end();

        if (weaponsEnabled) {
            sampleIx = uniform(mt) % sampleData[weaponIx].size();
            sample = getSample(sampleData[weaponIx], sampleIx, chaos);
        }

        if (weaponsEnabled && sample != "None") {
            if (weaponTypes.oneHanded.find(sample) != weaponTypes.oneHanded.end()) {
                actors[i]["Dynamic"]["EquipmentUser_Weapon"] = sample;
                canFuseWeapon = true;
                
                // try shield
                if (uniform(mt) >= threshold) {
                    sampleIx = uniform(mt) % weaponTypes.shield.size();
                    sample = getSample(weaponLists.shield, sampleIx, true);
                    actors[i]["Dynamic"]["EquipmentUser_Shield"] = sample;
                    canFuseShield = true;
                }
            }
            else if (weaponTypes.shield.find(sample) != weaponTypes.shield.end()) {
                actors[i]["Dynamic"]["EquipmentUser_Shield"] = sample;
                canFuseShield = true;
                
                // try weapon
                if (uniform(mt) >= threshold) {
                    sampleIx = uniform(mt) % weaponTypes.oneHanded.size();
                    sample = getSample(weaponLists.oneHanded, sampleIx, true);
                    actors[i]["Dynamic"]["EquipmentUser_Weapon"] = sample;
                    canFuseWeapon = true;
                }
            }
            else if (weaponTypes.twoHanded.find(sample) != weaponTypes.twoHanded.end()) {
                actors[i]["Dynamic"]["EquipmentUser_Weapon"] = sample;
                canFuseWeapon = true;
            }
            else if (weaponTypes.bow.find(sample) != weaponTypes.bow.end()) {
                actors[i]["Dynamic"]["EquipmentUser_Bow"] = sample;
                actors[i]["Dynamic"]["Drop__DropTable"] = "Arrow";
                canFuseArrow = true;
            }
            else 
                std::cout << "mistakes have been made" << std::endl;
        }

        if (canFuseWeapon && uniform(mt) >= threshold) {
            sampleIx = uniform(mt) % weaponTypes.weaponAttachments.size();
            sample = getSample(weaponLists.weaponAttachments, sampleIx, true);
            actors[i]["Dynamic"]["EquipmentUser_Attachment_Weapon"] = sample;
        }
        else if (canFuseWeapon)
            toRemove.push_back("EquipmentUser_Attachment_Weapon");
        else {
            toRemove.push_back("EquipmentUser_Attachment_Weapon");
            toRemove.push_back("EquipmentUser_Weapon");
        }

        if (canFuseShield && uniform(mt) >= threshold) {
            sampleIx = uniform(mt) % weaponTypes.weaponAttachments.size();
            sample = getSample(weaponLists.weaponAttachments, sampleIx, true);
            actors[i]["Dynamic"]["EquipmentUser_Attachment_Shield"] = sample;
        }
        else if (canFuseShield)
            toRemove.push_back("EquipmentUser_Attachment_Shield");
        else {
            toRemove.push_back("EquipmentUser_Attachment_Shield");
            toRemove.push_back("EquipmentUser_Shield");
        }
        
        if (canFuseArrow && uniform(mt) >= threshold) {
            sampleIx = uniform(mt) % weaponTypes.bowAttachments.size();
            sample = getSample(weaponLists.bowAttachments, sampleIx, true);
            actors[i]["Dynamic"]["EquipmentUser_Attachment_Arrow"] = sample;
        }
        else if (canFuseArrow) 
            toRemove.push_back("EquipmentUser_Attachment_Arrow");
        else {
            toRemove.push_back("EquipmentUser_Attachment_Arrow");
            toRemove.push_back("EquipmentUser_Bow");
        }

        for (j=0; j<toRemove.size(); j++) {
            if (actors[i]["Dynamic"][toRemove[j]])
                actors[i]["Dynamic"].remove(toRemove[j]);
        }        
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