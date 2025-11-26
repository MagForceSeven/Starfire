# Starfire
A collection of UE5 plugins that I've developed over the course of my hobby development. Some of which has also been used professionally in some version.

Current Engine version supported: 5.7.0

Platform Support:
* Windows, Linux. (May work on other platforms, but untested)
* (Linux compile is only tested using the cross-compile toolchains, but apparently this doesn't find 100% of compilation issues)

There are branches for version of the plugins compatible with earlier versions. However they are archives and not updated with any of the improvements that can be found in 'main'. The 'main' version may or may not work seamlessly in earlier versions of the engine.

These plugins are intended to be used directly by placing them in the Project or Engine Plugins folder.

Future improvement work that has been identified can be found [here](https://open.codecks.io/starfire). There is no timeline for when any of that future work may be addressed.

I'm happy to take pull requests and any bug reports or feature requests.

## High Level Plugin Descriptions
More specific details can be found in ReadMe's found within each plugin.

### Starfire Utilities
This is a collection of code that is quite a hodgepodge. A lot of various utilities that involve one or two source files that would require too much management to place each "system" within its own plugin.
Many bits from this plugin could easily be included into projects directly instead of taking the entire plugin.

Highlights:
* Typesafe array pointer and soft pointer casting utilities (I think everyone's got a version of this somewhere)
* An Editor Subsystem that can fixup Default Events when new defaults are added after child blueprints have already been made
* An interface that can be used for dealing with soft references held by Developer Settings
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

### Starfire Messenger
Provides a World Subsystem that acts as a message bus and provides highly decoupled method of broadcasting messages across a project.

Highlights:
* Messages are simple structures that don't require any additional allocation
* The message structure is used as both the payload data and the type used by listeners to register for messages
* A special type of message that is remembered and passed to listeners that register later
* A significant suite of functions that provide lots of flexibility in how listeners register, all with overlords & concepts to minimize the API clients actually need to care about
* Support for blueprint structures within the Project Settings
* Blueprint support for all messaging operations including two options for how blueprint listens for messages
* Support for hierarchical messages and message listening (listening for a type also recieves any messages of a child type) (only for native message types thoughs)

### Starfire Persistence
Provides example code for selecting and saving certain objects to a byte array using custom FArchiver implementations.
Also provides a module for Data Store Actors as a method of creating a runtime collection of Actors which easily save/load.

Highlights:
* A component for identifying persistent Actors which should have save data included by the FArchiver
* This component also manages a GUID ID system and recognizing if an Actor was spawned or part of a World that was loaded
* An interface for use by Subsystems for identifying which of those should have save data included by the FArchiver
* FArchiver for converting the collection of objects into a `TArray< uint8 >` which can easily be included in a save game object
  * Collection starts with Actors with the component and Subsystems with the interface
  * Collection is expanded to include owned subobjects (including components)
  * Custom archiver includes callbacks for filtering objects and components out of the saving process
* FArchiver for converting the `TArray< uint8 >` back to a collection of objects.
  * Will spawn objects which where spawned by the previous session
  * Will update objects which were loaded by the level
  * Will recreate actor components which were added to Actors at runtime
  * Will update actor components which are default sub objects
  * Will update Engine spawned actors like GameMode, GameState, Player Controllers (assuming the game didn't change any of those types in between the save and load)
* FArchivers are designed to handle hard references seamlessly
  * References to assets (or asset subobjects) are saved as paths
  * References to objects also being saved are saved with an index that is restored as a pointer to the spawned or existing object

## Dependencies
* Starfire Utilities is only dependent on modules available from the UE5 engine
* Starfire Assets is dependent on Starfire Utilities and Epic's Game Features plugin
* Starfire Save Data is dependent on both Starfire Utilities and Starfire Assets
  * The Starfire Assets dependency is removable if desired. See the Starfire Assets ReadMe for details
* Starfire Messenger is dependent on Starfire Utilities
  * The Starfire Utilities dependency is removable if desired. See the Starfire Messenger ReadMe for details
* Starfire Persistence is dependent on Starfire Utilities
  * The Starfire Utilities dependency is removable if desired. See the Starfire Persistence ReadMe for details
