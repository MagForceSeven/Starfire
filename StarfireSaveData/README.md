# Starfire Save Data

Some work identified for the future of this plugin can be found [here](https://open.codecks.io/starfire/decks/41-starfire-save-data), but there is no timeline for when this work may be addressed.

## General Description

This plugin is concerned with managing the save data format itself and accessing the files. It is not concerned with the contents of the same data itself, though example code is provided towards that end.

Starfire Save Data consist of multiple elements broken up between the project and a dedicated plugin. This system is independent of the `USaveGame` object that is provided by the Engine. It does use the Engine's `ISaveGameSystem` for interfacing with the filesystem.

The plugin consists of 4 modules, though only two of them currently do anything.

## Setup

* Branch or copy the files from the plugin's SaveGameExample module (public and private) to a similar directory in the project somewhere. The module files (_SaveDataExample.h/cpp_) and the module's .build.cs file can be ignored. In this step, you should also rename all the files replacing the GameSaveData naming scheme with something else appropriate to your project/use-case like "MySaveGame" or "WorldSaveData".
* Add the StarfireSaveData plugin to the dependencies of the plugin/project you've added the files to. Add the StarfireSaveData modules to the .build.cs dependencies for the module the files have been added to. If the plugin has an Editor and/or Developer module already, now is also a good time to add those dependencies even though those modules are presently unused.
* In the duplicated version of _GameSaveDataVersion.h_, you should update the `SF_BUILD_RTM` and `SF_SAVES_ALLOW_DEV` defines to have a project/use-case specific prefix instead of `SF_`. You should also add `ProjectDefinitions.Add("SF_BUILD_RTM=0");` to the _Target.cs_ and _TargetEditor.cs_ files for your project. If you have a branch specific to public releases, you should update the _Target.cs_ in that branch to "=1".
* Open all the files that you duplicated, and do a mass find and replace for "GameSaveData" to the same naming scheme you chose for the filenames.
* Also update Ex_GetSaveGameVersion to remove the Ex_ prefix
* Update the return value of your duplicated version of `UGameSaveHeader::GetFileTag`. This defaults to 'GAME' and should be unique per save data system. This value can be considered to be the file extension rather than the required .sav extension. This means it would have to match across projects if GameA was supposed to be able to load a data file from GameB for any reason.
* At this point you should be able to successfully compile and use the features of the save data.
* Evaluate the public API in the Utilities.h, BlueprintActions.h and ConsoleCmds.cpp to decide what is allowed to be done with the save data type that you are creating. You should also update the strings in ConsoleCmds.cpp to replace "Game.SaveData" with something more appropriate to your save data file. Consider any other elements of the public API that you wish to change the terminology of, ie "Checkpoint" → "InMemorySave".
* Start adding data to your SaveData and Header objects, using the functions `UGameSaveDataUtilities::CreateSaveData`, `UGameSaveDataUtilities::FillAsyncSaveGameData` & `UGameSaveDataUtilities::CreateSaveGameHeader` to populate them with data from the running game. There are comments in each function to help direct you to where you can fill it out.
* Add an implemenation to `UGameSaveData::ApplySaveData` to apply data from the save to the world after being loaded.
* At this point you should have a fully capable save system limited only by the game data you've included in the save. The console commands available by default in ConsoleCmds.cpp should be sufficient for testing your new save until you are able to hook actual game systems that should trigger the save or until UI becomes available for the player to interact with them.
* If the project supports a platform that doesn't have a matching _SaveDataUtilities_XXXXX.cpp_ supporting file, branch/duplicate _SaveDataUtilities_GenericPlatform.cpp_ to support your platform. Remove the #if/#endif.

## Dependencies

This plugin is dependent on both Starfire Assets and Starfire Utilities. There is a pending task to break the dependency on Starfire Assets, or at least make it optional.

## Detailed Description

### Save Data
An abstract `UObject` type defined by the plugin and a concrete type (or types) defined by the project.
This is the data that makes up what is written to disk as a "save game".
The plugin implementation contains some support for versioning and utilities for converting objects to/from arbitrary byte arrays.

The project's derived type is free to include whatever data it feels is necessary to save the state of game. All data that is meant to be saved must be a `UPROPERTY` since the underlying UE serialization used is property based. Since this object is dedicated save data, the "SaveGame" `UPROPERTY` markup is ignored and all properties are included when it is serialized.

The serialization of the save object does not use binary serialization, so adding and removing properties is safely backwards compatible. At least in general. How the project responds to default or missing data is left as an exercise for the project implementation. Changing the types of existing properties is a bit dicey and it's better to create a new property with a unique name and the desired type, unless you're also bumping the minimum version for the project save file to entirely prevent loading of old data.

It should be noted that the use of the term "Save Data" instead of "Save Game" is deliberate, at least when dealing with the plugin code. There is nothing inherently "Game" about this functionality and it could just as easily underpin the management of alternate persistent data or other type of meta data. But considering that the primary use case is "Save Games", these two terms are used somewhat interchangeably. There may also be some lingering use of "SaveGame" instead of "SaveData" in plugin code. In most cases this is because that code is private to the plugin and so shouldn't cause confusion, but it may be addressed in the future.

### Header Data
An abstract `UObject` type defined by the plugin and a concrete type (or types) defined by the project.
Acts as lightweight meta data about the save which can be loaded in isolation from the rest of the save, especially helpful if/when save size becomes an issue.

Primarily, this tends to be data that can drive UI elements involved with listing the saves in Save/Load related interfaces. This can also be information to filter which saves are important to a given operation.

The project's derived type is free to include whatever data it feels is necessary to save the state of game that is running. All data that is meant to be saved must be a `UPROPERTY` since the underlying UE serialization used is property based. Since this object is dedicated to save data, the SaveGame `UPROPERTY` markup is ignored and all properties are included when it is serialized.

The serialization of the header does not use binary serialization, so adding and removing properties is safely backwards compatible. At least in general. How the project responds to default or missing data is left as an exercise for the project implementation. Changing the types of existing properties is a bit dicey and it's better to create a new property with a unique name and the desired type, unless you're also bumping the minimum version for the project save file to entirely prevent the loading of old data.

### Public Utilities

The plugin contains a collection of utilities that are both public and protected. The protected utilities are available to the project's implementation of saves but not to unrelated systems. The primary reason for this is to limit the functions that the game is dealing with to those that use the project specific types and not the plugin types. This should prevent accidentally mixing any logic that means to interact with one system but uses another.

Another reason for the protected utilities is to force the project to have its own layer into which it can insert logic that is beyond the scope of the plugin. One possible insertion is the use of slot name prefixes to any save that matches the save type (like Manual, Dev or Auto) since this save type is not something that is meant to be dictated by the plugin. Another example is code related to cloud and other remote storage. The project specific API is able to bind together the two separate API's, one for cloud access and this one for local access, to get at the save file in the way most appropriate for the project. This both keeps the dependencies of this plugin light as well as preventing it from having to make arbitrary decisions for projects they may not like or be required to provide unnecessarily complicated logic in order for those decisions to be customized per-project.

### Private Utilities

The plugin also contains some utilities that aren't expected to be used directly by the project and are entirely project agnostic. These include a manager for asynchronous save data tasks, a cache of headers from known save data files and utilities for converting the header and save objects into data that will actually be written to the file. These utilities are discussed in more detail later.

## Starfire Save Data

This plugin contains the generic portions of working with saves. It is (in theory) project agnostic and provides the baseline support for reading/writing saves to disk, file operations (query, rename, delete, etc), performing save operations synchronously and asynchronously, versioning, file integrity checks, compression, a header cache, batch loading and multi-platform concerns. It also supports dealing with saves that are kept alive during the runtime (checkpoints) instead of written to disk and forgotten.

The plugin does not handle any form of cloud or other remote file system integration. It is expected that the project would add this sort of support as part of its public utilities prior to calling the protected utilities provided by the plugin.

The overall format for a save file consists of (in this order):

* File Description
  * `FSaveDataFileDescription`
* Header
  * `FSaveDataVersionInfo`
  * Serialized bytes from custom header type derived from `USaveDataHeader`
* Save Data
  * Serialized and compressed bytes from custom data type derived from `USaveData`

## Components
### Runtime Module
#### Save Data
_SaveData.h/cpp_

The base type for the data you wish to store in the file. It handles versioning and comes with a few utilities for working with your save data.

__PackageVersion__ & __LicenseeVersion__ deal with versions of the Unreal Engine and shouldn't need to be dealt with directly.

__GameVersion__ is the place for keeping the game specific version number for save data that you should be updating as you add data to your derived type. It is set when you call `FillCoreData` (which you should call first when populating the data of your save).

__ContentFeatures__ are the names of the Game Feature Plugins that were active at the time that the save was created. This is to facilitate error messages with which to inform users of the content they need to load the save. It is also set when calling `FillCoreData`.

__ConfigureArchiveVersions__ is a utility for setting all the version data on an archive before it's used. You should use this on any archiver that you create prior to using it to serialize anything. This will make sure that the correct versioning information is available to you during the serialization of objects.

__SerializeToBytes__ & __SerializeFromBytes__ are simple utilities for converting between a known object and a bytestream. This can be handy for very simple objects and wraps the process of creating an archiver (`FObjectAndNameAsStringProxyArchive`), setting the version information and then serializing the object using the provided byte array. As with other serialization, this archiver is flagged to not use the binary serialization which makes backwards compatibility easier at the cost of saves that might be a little larger.

__IsCompatible__ is a virtual that must be overridden by the derived type in order to check if a specific game version number can be loaded by the current game. This function is always called on the Class Default Object of the derived type so no actual save data can be used to make this determination, only the game version input and whatever values are available to determine min & max compatibility allowances.

#### Save Data Header
_SaveDataHeader.h/cpp_

The base type for the abbreviated data you want to make available for the file. This structure is meant to be filled with fairly simple types that can be serialized directly, and so does not have the range of versioning and serialization utilities that `USaveData` does.

It has two methods related to the file `GetFileTag` and `GetVersion` that must be overridden by the derived type. The file tag is just a simple integer (usually generated through a 4-character literal like 'GAME') and the version is the (most likely) the same value that was stored in the __GameVersion__ member of `SaveData`. The file tag is meant to quickly identify files that aren't the right type since we can't rely on the file extension (and even if we could, we wouldn't).

The header also has an `IsCompatible` function that must be overridden and works the same as the one in `USaveData` to check if the game can actually load that save. This function is always called on the Class Default Object of the derived type, so no actual header data will be available to compare against.

`GetCompressionType` is a virutal function that can be overridden to change the compression library used on the save data when converting to file data that will be written to disk.

__Timestamp__ is an encoding of the time that the save was made and should match the metadata from the filesystem in most cases.

__BuildChangeList__ and __BuildVersion__ are additional details about the build that created the save that can be used for diagnostics that aren't actually save/load related. They're more generally helpful when debugging to verify the state of the source build the save was sent from. Both values are filled in based on the values from the Build.version file that is updated by the packaging process (for cooked builds) or by UGS (for local/precompiled/developer builds). If you're working with a build that is not-cooked and not using UGS, you can update Build.version manually or with a batch file or fill in these values programmatically.

__DisplayName__ is a string that user friendly name. This could be sourced from the user, auto generated, or some combination. There is no default behavior and is a copy of the __InDisplayName__ parameter provided to `FillCoreData`.

__Language__ tracks the language setting of the build at the time that it was created. This is because __DisplayName__ is expected to be displayed to the user and may contain characters that the current language can't account for (English loading a Japanese save for example). __DisplayName__ may contain those characters because it allowed user input. If the project save system does not involve user input, __DisplayName__ and __Language__ could be ignored entirely in favor of `FText` members in the derived header type.

__ContentFeatures__ are the names of the Game Feature Plugins that were active at the time that the save was created. This is to facilitate error messages with which to inform users of the content they need to load the save.

`FillCoreData` is the utility that should be used when creating an instance of the derived type and filling in its data.

### Public Utilities
_SaveDataUtilities.h/cpp_

These are the basis for the main functions that are called either by the game or by the functions in the library the game derives from `USaveDataUtilities`. This document will not outline every single function, the code and comments are better for that, but instead will discuss the general groups of functions that are available.

The public functions are those that do not interact directly with the Save Data or Header types or are not reasonably expected to involve game specific logic in their function. This is why `GetTextForLoadingResult` is public, but `DoesSaveGameExist` is protected even though it does not interact with the Save Data or Header types. Similarly, functions in `USaveDataUtilities` are not generally blueprint accessible, because the expectation is that the project needs to create custom versions of these functions anyway to provide project specific information needed for that task.

The protected functions are the majority of functionality provided by `USaveDataUtilities`. The reason they are protected is to limit project code to the project's API for interfacing with saves so as to not accidentally bypass necessary or required inputs needed to make the project's saves function properly.

The private functions are just that, functions that should only need to be called by the `USaveDataUtilities` functions. Any information from those functions should be accessed through whatever protected version of that function is available.

_Platform Implementations_

Keeping with the multiplatform approach of the engine, this plugin is setup to be compatible with various platforms. Currently, `USaveDataUtilities::EnumerateSlotNames` is the only function that supports platform unique implementations. A few _SaveDataUtilities_XXXX.cpp_ are provided for platforms that don't require special access. _SaveDataUtilities_GenericPlatform.cpp_ is also provided as a quick implementation that can be duplicated and used as a reasonable starting point for any unsupported platforms.

_BatchHeaderLoader.h/cpp_

A utility for making and responding to multiple header loading requests more simply than requesting each one individually. While this is a public utility, it's only truly meant to be used by the derived version of `USaveDataUtilities` in the project. Since the BatchHeaderLoader is in the plugin, it only works with CoreSaveHeaders. But the project should be using its project specific header type. The project version of `USaveDataUtilities` is where this conversion/casting is meant to take place.

### Shared
_SaveDataCommon.h/cpp_

A collection of simple types used throughout the save system that would not be specific to any save system. The significant elements are the `ESaveDataLoadResult` enumeration, which lists all the error cases when trying to load a save data or header, and forward declarations for the plugin save data types.

_SaveDataVersion.h/cpp_

Contains the GUID used for the custom project versioning used when serializing the Header, Save Data and any other objects included in the save. As part of calling `USaveData::ConfigureArchiverVersions`, the custom version is assigned to the GUID version based on the __GameVersion__ member of the Save Data. During loads, the custom version can be checked using `GetSaveGameVersion` (from the project's version of _GameSaveDataVersion.h_) to modify the behavior to be compatible with the format from the older version.

### Private
#### Async Task Manager
_SaveDataAsyncManager.h/cpp_

A World Subsystem which manages `FSaveGameTask` (a structure derived from `FNonAbandonableTask`) instances to run and track save data related tasks asynchronously. This is the backbone of all the asynchronous operations supported by the plugin. (I am aware of general language support for coroutines and the UE5Coro plugin. This plugin's history predates both of those, but may be updated in the future.)

Save data functions can declare local structure types derived from `FSaveGameTask`, give it a function named `DoWork`, create an instance of the structure and call `StartAsyncSaveTask` (providing it the structure instance). `DoWork` will automatically be called, running on a separate thread to do whatever work is desired (normal rules for threaded work on a non-game, non-render thread apply). `FSaveGameTask` also provides two virtual functions, `Branch` and `Join`, which can optionally be overridden. `Branch` can be used for logic which should run when the task is successfully queued up and started. `Join` can be used for logic that should run when the task is complete but before completion is broadcast to anyone else. The reason for these separate functions is to allow a space where logic can run on the game thread before and after the async work is started or completed. There are lots of examples of creating these structures throughout `USaveDataUtilities` and the project's derived version of the library.

Another parameter to `StartAsyncSaveTask` is a callback for the async work completion. This callback is used for coordinating sequences of tasks or for communicating back "out" from the function through it's own type of callback. The primary difference between this callback and the `Join` function mentioned earlier is that `Join` should be limited to working with the internal data of the task, not reporting on it or trying to start additional async work.

As a private utility, other code isn't expected to interact with the subsystem directly. However there are a few ways to access it indirectly. The first is through the `StartAsyncSaveTask` function which is a protected method of `USaveDataUtilities` since only the save utilities from the plugin or from the project's derived implementation should be starting any tasks. There are two functions, `FlushAsyncSaveTasks` and `WaitOnPendingSaveTasks` that can be used to make sure there are no pending tasks remaining. `IsAccessingSaveData` and the two delegates `OnSaveDataAccessStarted` & `OnSaveDataAccessEnded` are available to check on or respond to async save operations happening. Most commonly this is showing a UI element to warn the player not to shutdown the game.

#### Memory Utilities
_SaveDataMemoryUtilities.h/cpp_

The functions collected here manage the low level conversions between the objects and files. Most of the functions support the conversions back and forth between `SaveDataHeader` & `SaveData` objects and file data (arrays of `uint8`s) which can be written to disk. There are also utilities for saving and loading this file data to save slots (using the `ISaveGameSystem` provided by the Engine) and utilities for loading only the `SaveDataHeader` from an `FArchive`.

There are also a couple of structures which are used at the start of a save file that generically deal with file type, corruption check hashes, versioning info and compression info.

Multiple compression types are supported, ZLib, GZip and Oodle (all using built in Engine support). The default is ZLib. If a project wants to change this, it can be done by overriding `USaveDataHeader::GetCompressionType`.

#### Header Cache
_SaveDataHeaderCache.h/cpp_

This is a behind the scenes cache of any of the headers that have been request to load. This back-end is shared by all save data systems. The header is primarily populated by creating/saving new saves, loading full saves or loading headers. There is a utility, `USaveDataUtilities::CacheSaveGameHeaders`, that can be used to forcibly populate the cache at a specific time if desired (like when first entering the Main Menu). The cache is not considered authoritative though, a newer file that is on disk than what is in the cache will result in the header being loaded from the file instead of using what's in the cache. When doing a full load of a save file, the header from the file will always be used regardless of the contents of the cache.

### Editor and Developer Modules
The StarfireSaveData plugin has two modules: StarfireSaveDataDeveloper & StarfireSaveDataEditor. Developer is an UncookedOnly module for any tool support required by the cook or by uncooked standalone builds. Editor is an Editor module for any future Editor tools. Currently, these are strictly placeholder modules for future code.

### Starfire Save Data Example
This module is meant to be an example, hence the name, of an implementation of a project specific save system. It is oriented around the assumption that a Save Game is desired and not some other type of local, persistent save data. For the most part, any deviation from the Save Game profile just involves deleting portions of the publicly available APIs, be that whole functions or individual function parameters.

For Engine reasons, all saves use the ".sav" file extension. This is something that should be kept in mind if there are multiple save systems running as you may want to consider how a naming scheme (especially if user entered strings are allowed to affect the filename) can guarantee there are no filename collisions that cause one system to overwrite saves from another save system.

#### Game Save Data
_GameSaveData.h/cpp_

This is the actual data that is going to be persisted from session to session. For the most part, adding new `UPROPERTY`'s is the simplest way to extend the save data. It's best to avoid adding any sort of references to other objects as direct pointers or sub-objects as the default archiver may or may not serialize that object so that it persists. It's a little easier to create `TArray< uint8 >` members which you fill in using the `SerializeToBytes` and `SerializeFromBytes` utilities provided by `USaveData`. Some exceptions may occur, especially if you want to reference assets or types that would be around external to the save data, but those will still be better to reference through `TSoftObjectPtr`s or `FSoftObjectPath`s (like the map is).

Example Data

__SaveType__ : An enumeration that identifies the circumstances in which the game was created, manually, an autosave, a quicksave, a bug report, etc. Used to match with the ESaveGameFilter to limit certain save game operations.

__Descriptor1__ & __Descriptor2__ : Project specific descriptors that can inform the player about the details of this save. Overly generic names provide a certain amount of flexibility across any game mode specific needs without having an explosion of text values (most of which wouldn't get used).

__MapPath__ : The map that was loaded at the time the save was created. `PostSaveGameLoad` in _GameSaveDataUtilities.cpp_ uses this as the parameter to ServerTravel when loading a save.

#### Game Save Data Header
_GameSaveDataHeader.h/cpp_

As mentioned previously, this data is mostly metadata about the save. It's been separated out from the full save so that the sub-object can be extracted and stay loaded without having to keep the entire save in memory. At least in theory. On PC, a file stream allows loading and reading only what is necessary but some platforms may still required loading the whole save before extracting the header. At the very least once read, the header doesn't require the entire save to stay resident in memory. The example header contains data that is copied from the example save data. The data that is copied from the save data is because it's generally easier to do that than to figure it out again. The save data is more likely to be hooked into all the gameplay data that would be required to be checked to set these values so it's simpler to do those checks there and then copy them when writing the save to disk (which is the only time a header is created). In addition, when dealing with checkpoints it's possible that between the time of creating the in-memory save and when it's written to disk, one or more of those values may have changed. The header should reflect the game state actually being saved and not the game state at the time it was written to disk, but the project can easily customize this through it's specific implementation of `UGameSaveDataUtilities::CreateSaveGameHeader`.

Example Data

__SaveType__ : Same as in Save Data, copied from there

__Descriptor1__ & __Descriptor2__ : Same as in Save Data, copied from there

#### Save Game Subsystem
_GameSaveDataSubsystem.h/cpp_

This is a game instance subsystem used by the save system to persist information across levels. Previously this data would have been stored directly in the game instance, but with subsystems this is a much easier way to keep that data, especially from the perspective of providing the SaveDataExample module that can be duplicated and "just work". The data included by default is a reference to a save data object (generally the one selected to be loaded which ended the previous session), the name of the last save slot that was loaded (which supports the 'Game.SaveData.ReloadSave' console command), and a string of additional parameters that can be appended to map loads when loading save data (to support automation tools, though none of these are included in the plugin or the Example at this time). Since the subsystem is directly part of the example module, there is no derivation required to implement project specific logic, a project should just modify their branched version directly. It also means that any functionality dependent on this subsystem is not accessible to any of the plugin logic.

This is also where a Developer Settings class for configuring various project data related to saves.

##### Shared
_GameSaveDataUtilities.h/cpp_

This is the library with the functions that are used by the rest of the project to interact with the project save. It serves as a way to translate the more general save data functionality into the types and terminology that are appropriate for the project save as well as providing a place for the project to layer extra logic (like cloud features) into the operations before or after using the lower level save data system. Most of the functions in this library come in pairs with one function being asynchronous and one not. The pattern is for a function that has "Async" appended to the name which is asynchronous and another function which is synchronous. Synchronous functions may or may not be exposed to blueprint by default, but it should almost always be okay to expose them if not done so by default. Asynchronous functions should be exposed indirectly as part of the Async Blueprint Utilities, as those will provide a much better blueprint user experience than directly exposing the asynchronous utility as a callable function.

The library is meant to expose lots of concepts externally, while internally redirecting down to a very few functions and ultimately resulting in calls to the plugin library. For example, the public API includes both an option for an AutoSave and QuickSave, but the implementations of both are just parameterized calls to `SaveToSlot`.

One thing that you may notice is that the `FEnumeratedSaveDataHeader` structure has been subtly duplicated here as `FEnumeratedHeader_Core`. The reason being that the `FEnumeratedHeader_Core` is enough for what the plugin needs, but not necessarily for the needs of the game. The first major difference being that the project version is blueprint accessible while the plugin structure isn't even a `USTRUCT`. The second difference is that the project structure stores the project specific object pointer instead of using the `USaveDataHeader` type.

_GameSaveDataVersion.h/cpp_

The project version of this header is where the history of save versions is kept, through values in an enumeration. This enumeration value is what is assigned to `USaveData::GameVersion` and what is returned by `USaveDataHeader::GetVersion`. It's also what the `IsCompatible` function should check against to do its work.

The versioning enum is made up a few different parts. The first section (Development to Current_Plus_One & Latest) is space for values to identify any/every significant (or perhaps even insignificant) changes to the data included in the save. The primary use here will be a version that needs to act as a new minimum version (preventing saves of a lower version from being loaded) or a minor change to some piece of the save that can be checked using the `FArchive` custom version to do different work for different versions. Next is a single value which acts as a flag to identify saves that were produced in a public release build of some kind. This allows builds to discriminate as it is often very helpful for release builds to not be able to load builds produced by an in-development build. Development_Minimum and RTM_Minimum allow you to set the enumeration value that should be the minimum allowed version that your project loads. You might update these after adding a new value to the first section. It's also reasonable for RTM to use a higher value as a minimum than Development, but the reverse is dangerous. In fact (by default at least) a static_asset is included to catch these being mismatched in this way. A compile switch is then used to bake a single enumeration value that is the minimum version allowed for the current build as well as baking a single enumeration value for the current version of the save for the current build.

Thanks to the use of non-binary serialization, many of the usual culprits for requiring versions are taken care of because new properties and property removal do not affect the serialization of the data in the same way that is so detrimental to binary serialization. It is also very important that each type of save data that implements save data have it's own versioning enumeration. Although they can share the FGUID for identifying the custom version within an archive, each type of save should have it own custom type and history of versioning.

If there have been a number of optional version updates and the minimum version is updated, it is strongly suggested that any custom versioning be removed. Bumping the minimum version should make it impossible to load those previous saves and keeping around that versioning code is unhelpful cruft (because you have source control, right?). Also any change that would require bumping the minimum version should be avoided once the game has shipped and their are saves in the wild.

#### Async Blueprint Utilities
_GameSaveDataBlueprintUtilities.h/cpp_

This is a collection of all the classes for supporting the asynchronous save data operations from blueprint. For cases where blueprint wants a synchronous version it should just use a blueprint callable function from the Utilities library. All of the functionality here should have corresponding functionality in the utility library somewhere. But the action to function mapping is not one-to-one. Many of the async actions intentionally support multiple async actions when those actions are all relatively similar. For example `USaveSaveData_AsyncAction` has a range of functions for manual saves, auto saves and quick saves. And because of the similarity in inputs, it also acts as the wrapper for deleting saves asynchronously.

There is one blueprint async action that is provided by the plugin, `WaitOnSaveGameAsyncActions`, since it is more directly tied to the Async Task Manager and does not require any project-specific logic.

#### Console Commands
_GameSaveData_ConsoleCmds.cpp_

This is the one stop shop for collecting all the console commands used to interact with save data of a particular type. There is no header for the file as it is not expected that anyone should ever interact with this code from other code. It self registers with the engine as an exec (console command) handler for use in the running game.
