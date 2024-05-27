# Starfire Assets

Some work identified for the future of this plugin can be found [here](https://open.codecks.io/starfire/decks/40-starfire-assets), but there is no timeline for when this work may be addressed.

## General Description

This is a plugin for managing two special subsets of primary data assets. The goal is to have a collection of assets that are easy to reason about and write generalized logic around. It does this by controlling the loading of the new primary asset types so that they match up with the state of their source game feature plugin. A custom asset manager provides a new/alternate set of functions to access the assets that are currently active (as opposed to those known to the asset registry or those currently loaded).

## Detailed Description

This is a plugin that lays out one specific way to handle primary data assets across a game and its feature plugins. It works by recognizing a specific subtype of primary data asset (those derived from `UDataDefinition`) as an asset that should be "always loaded" while the game (not Editor) is running. This means slightly different things for assets in a game feature and those in the game or a non-feature plugin. In the case of the non-feature assets, they will be loaded (if you follow the setup instructions) with the same lifetime as the GameInstance, which means the whole game lifetime. Assets from game features will only be loaded as long as the feature is loaded. This may seem crazy, but it's really not as crazy as it seems. The primary data assets managed this way should individually be fairly small with all the expensive content (meshes, textures, blueprints, etc) being referenced through soft pointers. The upside is that the game can then do work and logic based on these always loaded assets. This is similar to how primary data properties can be made searchable by the Asset Registry to determine what should be loaded without having the primary asset loaded. However, I've run into difficulties trying to work with the data in that way as it requires that the data can be reasoned about as a string and not as its native type.

In addition to the collection of primary assets that are "always loaded", two bundle names are treated special: AlwaysLoaded and DebugAlwaysLoaded. These are bundles that are applied as part of loading DataDefinitions for the game. These may sound like hard references, but the key difference is that they aren't loaded when the Editor loads the asset (or show up as part of the SizeMap report). The DebugAlwaysLoaded bundle is really helpful before you know what bundles you need or for local testing of new Definition types. It is expected that a project will/may ship with content in the AlwaysLoaded bundle, but that all cases of DebugAlwaysLoaded have been replaced with real, project specific, bundles prior to ship.

Also, while the assets from game features are loaded while the feature is loaded the DataDefinitionLibrary provides separate tracking from the base AssetManager to know what assets are not only loaded but also "active". Assets from non-game features are always active while assets from game features are only treated as active when the feature is active. This makes it very easy to query the DataDefinitionLibrary for the set of assets that are actually relevant for the state of the game with respect to game features!

The next important element of this plugin is the DataDefinitionExtension type. This is another primary data asset subtype. They aren't DataDefinitions but do follow the same "always loaded" rules. Instead these assets act as a way for a game feature to append additional information to DataDefinitions that are in the game or another game feature. This is very similar to the Game Feature Action which adds components to actors when a feature is activated or an Actor is spawned. To prevent the setup of extensions from having a bottleneck, they reference the definition(s) they extend directly instead of doing anything through a feature action. It also results in better experience when using the Reference Viewer in the Editor. _Note: I would like to write a custom editor that supports editing a definition and all it's extensions within a single Editor UI. The current workflow isn't terrible, but neither is it ideal._

Next is the custom support for Game Feature Plugins which consists of three parts: a new Game Feature Data asset type, a new management subsystem and developer settings. The asset adds some values to make features a little more useful to runtime of the game like display data as well as some for runtime decision making with some gameplay tags. The subsytem adds an extra layer of for controlling the activation of features through some utilities as well as and API that is somewhat more helpful (at least in my opinion after working with built in subsystem). It also adds a place that can track ownership of features that can come into play for DLC or expansions that are shipped as features. This ownership is not directly tied to anything (except for a few things supported through feature asset gameplay tags), so your project is free to mark features as owned or not in whatever way make sense for your game. Developer settings are available to make testing (in PIE or other builds) easier by allowing developers to control the set of features they want to have available when they're doing local testing.

Lastly, some interfaces are provided that can be used as part of checking the configuration of assets and asset subobjects. There is one interface for assets and one interface for subobjects (such as instanced objects). The subobject interface includes some options for interacting with the kismet (blueprint) compiler because I've been able to reuse subobjects within custom K2 nodes and wanted to be able to write validation code once and have it work for both assets as well as checking during the blueprint compilation process. During implementations of the interfaces, you call `AssetChecks::AC_Message` to report the error. If you're in the Editor, this error is directed to the Editor's Message Log, if it's a K2 Node, the message is sent to compiler context and if you're in-game (either standalone or PIE) it produces a Redscreen (see StarfireUtilities). The means that with one function the configuration is checked and reported as much as possible. The subobject interface also makes it easy to delegate from asset to instanced sub-objects to further instanced subobjects. This way you can delegate configuration checks all the way to the most specific place with the most information to know what is right or wrong.

## Setup

There are a lot of elements to the UE's management of primary assets that need to be configured once this plugin is installed

1) Update the project to use the DataDefinitionLibrary as the Asset Manager. This can be done any of three ways: a) change the value in DefaultEngine.ini, AssetManagerClassName in the /Script/Engine.Engine section to be "/Script/StarfireAssets.DataDefinitionLibrary", b) update the value found in Project Settings > Engine > General Settings > Default Classes (Advanced) > Asset Manager Class to DataDefinitionLibrary, or c) update the parent class of your existing custom Asset Manager to derive from DataDefinitionLibrary instead of directly from the Asset Manager.
2) Add entries for the DataDefinition and DataDefinitionExtension types to the Asset Manager's PrimaryAssetsToScan in Project Settings > Game > Asset Manager. If you already have a shared and standardized base class for all your primary data assets, you can either leave that alone or update it to derive from DataDefinition depending on the desires of the project and if they should be "always loaded" or not.
3) Add an entry for the GameFeatureData type to the AssetManager's PrimaryAssetsToScan. The directory to scan should be /Game/Unused and "Always Cook". If you don't want to do this manually, starting the Editor after adding a dependency on this plugin will result in a popup that can add this entry for you.
4) Update the project to use the Starfire_GameFeaturesProjectPolicy as the Game Feature Policies. This can be done an of three ways: a) change the value in DefaultGame.ini, GameFeaturesManagerClassName in the /Script/GameFeatures.GameFeaturesSubsystemSettings section to be "/Script/StarfireAssets.Starfire_GameFeaturesProjectPolicy", b) update the value found in Project Settings > Game > Game Features > Default Classes > Game Feature Project Policy Class to Starfire_GameFeaturesProjectPolicy, or update the parent class of your existing custom project policy to derive from Starfire_GameFeaturesProjectPolicy instead of directly from GameFeaturesProjectPolicies or DefaultGameFeaturesProjectPolicies.
5) If you use the Editor Wizard for creating feature plugins, you can update the asset type it creates for the GameFeatureData in Project Settings > Editor > Game Features Editor Settings > Plugin Templates > Default Game Feature Data Class to StarfireFeatureData. You can also do this by editing DefaultEditor.ini, but in this case doing it from the Editor is easier. Don't miss setting the value in _both_ array entries. One is for content only features and the other is for features with C++. If you've already got a custom Game Feature Data, you should update it to derive from StarfireFeatureData instead of from Game Feature Data directly.
6) If you already have any feature plugins (and weren't previously using a custom Feature Data type), you'll want to update them to be StarfireFeatureData. The easiest way to do this is to add a CoreRedirect to DefaultEngine.ini in the CoreRedirects section. Paste this redirector `+ClassRedirects=(OldName="/Script/GameFeatures.GameFeatureData",NewName="/Script/StarfireAssets.StarfireFeatureData")`. Open each of your feature data assets in the Editor and save them. You can now remove the redirector.
7) If you don't have a custom Game Instance, you'll want to set one up. Once you do you want to access the DataDefinitionLibrary (or your custom Asset Manager that derives from it) in an override of `GameInstance::Init` and `Shutdown` to call `UDataDefinitionLibrary::GameInstanceInit` and `GameInstanceShutdown`. Depending on your project's specific needs, you can wait for the `GameInstanceInit` stream handle to complete before returning, you can store it for checking later or you can ignore it (and hope for the best).
8) The initial state of all future GameFeatureData's should be set to Registered.

## Dependencies

In addition to the dependencies on plugins from the Engine, Starfire Assets is also dependent on the Starfire Utilities plugin found in this repository.

Currently this plugin requires the pull request found [here](https://github.com/EpicGames/UnrealEngine/pull/11604). I still need to make an option that gets around this, but really the engine change is the superior solution to the timing problem that is happening. Otherwise it is very difficult to ensure that all of the features can be known about and that we're tracking the proper state.

## Components

### Runtime

#### Data Definitions
_DataDefinitionLibrary.h/hpp/cpp_

A custom implementation of the UAssetManager class that adds functionality related to the DataDefinition subtype of primary data asset from this plugin. Primarily this extension is handling the special case "always loaded" behavior outlined in the detailed description.

Another useful thing the Library adds is support for "recursive" bundles. This allows the `ChangeBundleState` process to follow hard and soft references to other primary data assets and apply the same bundles to those assets. This process follow normal bundle rules, so if `ChangeBundleState` is called for "BundleA" and the asset property is marked up for "BundleB" then no "recusion" happens. This recusive process of bundles can be very helpful when setting up primary assets that reference other primary assets. Of course there's no requirement that this recursion is used at all, just like not all assets may be appropriate to load with bundles in the first place.

_DataDefinition.h/hpp/cpp_

The base class for any primary asset type that should function as a Data Definition and the custom loading behavior described in the detailed description.

_DefinitionExtension.h/cpp_

The base class for Data Definition Extension assets. (Currently there is no way to limit which extensions can be applied to which DataDefinition types, but that is planned as part of overall tooling support for these assets.)

_DataDefinitionSource.h/cpp_

An useful interface for use by non-DataDefinition UObjects that have a 1:1 relationship with a Data Definition. Not required to be used, but it is helpful when writing some types of generic code that wish to get the definition that would have been used to create the UObject instance.

_Exec_DataDefinition.h/cpp_

This is an extension to the `ExecSF_Params` namespace from Starfire Utilities. It provides support that allow data definitions to be retrieved from console command input strings directly as asset references like so:
```
UDataDefinition_Subtype *Asset = nullptr;
int NumParams = GetParams( Cmd, Asset );
```
instead of getting the asset name as an FString and doing an asset lookup. This `Get` is also type safe so that if there is an asset of with the name but it isn't the same type as the variable, the variable will be nullptr.

On some platforms (clang for sure), you will need to include this header _before_ including ExecSF.h

_LibraryGameFeatureWatcher.h/cpp_

A private engine subsystem that acts as an intermediary between the DefinitionLibrary and the GameFeaturesSubsystem. Primarily used to track the DataDefinition primary assets in order to know what assets should be affected when a feature is loaded or unloaded.

#### Game Features
_StarfireFeatureData.h/cpp_

A custom derived version of the GameFeatureData asset type provided by the Engine with additional data to work with the FeatureContentManager subsystem and additional debugging elements. That additional data includes a collection of gameplay tags and some text for helpful display information (at least in the case where a feature is directly player facing like DLC or other expansion packs).

_FeatureContentManager.h/cpp_

A GameInstance subsystem that acts as an intermediary between a game and the GameFeatureSubsystem. It adds the concept of tracking ownership/entitlement to features and provides additional ways to activate collections of features without neccessarily knowing names or needing references to the feature data.

These files also contain a developer settings type that can be used for local testing of feature configurations. It works with the FeatureContentManager subsystem and some console commands to make it easy to test various feature configurations or entitlement scenarios. The developer settings are not intended to control any project wide defaults (those should be controlled through the tags that have been added to the StarfireFeatureData).

_StarfireGameFeaturePolicy.h/cpp_

A default policy implementation that allows the Game Feature system to load the proper set of primary assets and applies the appropriate asset bundles to them.

#### Asset Validation
_VerifiableContent.h/hpp/cpp_

Contains the interface that can be used by assets and the interface for other objects that can be used to hook into a wholistic content verification process.

_AssetChecks.h/cpp_

Contains the namespace/functions that should be used to report any errors or warnings that occur from implementations of the interfaces from _VerifiableContent.h_.

### Editor

#### Thumbnail Renderer
_DataDefinitionThumbnailRenderer.h/cpp_

This provides simple support for any DataDefinition to display a texture as it's thumbnail image in the content browser. (There is a known issue where the thumbnail doesn't display properly for assets that aren't currently loaded.)

### Developer

There is currently nothing in this module, it is a placeholder for future features. This is a placeholder for future features that need this module type.
