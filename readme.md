# Whaley's Tears of the Kingdom Randomizer V1

### What's randomized
- Enemies
- Weapons (excludes those found in chests)
	- melee weapon attachments
	- enemy arrow attachments

### Randomization Options
- **Normal mode:** total numbers of enemies/weapons/etc. are unchanged. For example, there is still only 1 Ganondorf, but his location could be anywhere.
- **Chaos mode:** all enemies/weapons are equally likely. There will be ~equal numbers of Bokoblins, Lynels, and Ganondorfs :)

### Building (requires cmake)
1. create a build directory (`mkdir build`)
2. In the build directory, initialize the cmake build: `cd build && cmake ..`
3. compile: `cmake --build .`

### Usage
In the build directory, run `./randomizer <path-to-romfs-dump> [--chaos] [--reindex]`
- `--chaos`: chaos mode, all enemies/weapons are equally likely
- `--reindex`: re-calculates the number of enemies/weapons in the vanilla map files. You will probably need to do this if playing on a version > 1.0.0 without chaos mode. Takes a very long time.

Generates a new romfs directory which you should place in `atmosphere/contents/0100F2C0115B6000`

### Notes
- see data/unique/readme.txt for list of non-randomized enemies
