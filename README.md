# Starfire
A collection of UE5 plugins that I've developed over the course of my hobby development. Some of which has also been used professionally in some version.

Current Engine version supported: 5.3.2 (though this isn't quite accurate as I've backported a change that fixes UHT so that the UFUNCTION and UE_NODISCARD macro work properly on the same function)

Minimum Engine version required: 5.3.2

These plugins are generally intended to be used directly by placing them in the Project or Engine Plugins folder.

I'm happy to take pull requests and any bug reports or feature requests.

## High Level Plugin Descriptions
More specific details can be found in ReadMe's found within each plugin.

### Starfire Utilities
This is a collection of code that is quite a hodgepodge. A lot of various utilities that involve one or two source files that would require too much management to place each "system" within its own plugin.
Many bits from this plugin could easily be included into projects directly instead of taking the entire plugin.

Some identified work can be found [here](https://open.codecks.io/starfire). There is no timeline for when any work tracked there might be completed.

Highlights:
* Typesafe array pointer and soft pointer casting utilities (I think everyone's got a version of this somewhere)
* A ForEach Blueprint node for TMaps
* An Editor Subsystem that can fixup Default Events when new defaults are added after child blueprints have already been made
* An interface that can be used for dealing with soft references held by Developer Settings
* Typesafe casting of structures (native only, somewhat less useful with Instanced Structs but somewhat more convienent than those)
* A subsystem that can manage "splatting" tasks across multiple frames with various perf controls
* A system of "redscreens" as an additional error reporting mechanism that promote the visibility of specific failures to something in-game instead of information that must be dug out of a log

### Starfire Assets
Introduces a new primary asset type that is designed to be "always loaded" for the duration of a game or while a game feature is active.
Provides a custom implementation of a number of asset management types as well as additional support for Game Features.

Highlights:
* A new primary asset type to act as the basis for the "always loaded" behavior
* A new Asset Manager type that can manage this "always loaded" behavior for the project and Game Features
* A new subsystem for helping to manage information about and control states of Game Features
* A derived GameFeatureData type that includes support for more data used by the new subsystem
* New mechanisms to help with the validation of assets and their subobjects

### Starfire Save Data
Creates a parallel mechanism of save data. No utilities are provided for what is included in the save, only for the interaction with the files.

Highlights:
* A new object that can be filled with data to be used to restore a game session
* A new object that can be filled with data that acts as custom header information that can be loaded independently of the whole save data
* Sync and Async hooks for writing to disk, but also for filling out the save data itself
* Controls for versioning, compression, file corruption checks and Game Feature information
* Reusable such that you could use it for multiple user-file save data reasons: campaign saves, meta-progression, character pools, etc

## Dependencies
* Starfire Utilities is only dependent on modules available from the UE5 engine
* Starfire Assets is dependent on Starfire Utilities and Epic's Game Features plugin
* Starfire Save Data is dependent on both Starfire Utilities and Starfire Assets
  * The Starfire Assets dependency is removable if desired. See the Starfire Assets ReadMe for more details  
