# Starfire Game

A miscellaneous collection of gameplay tools that very generic but not big enough to warrant maintaining a full plugin.

## General Description

Starfire Game Core are some shared classes I use in all my projects to share some basic functionality (or provide a place for that in the future).

Game Feature Subsystems is a bit of an experiment to combine Subsystems with Game Features in a more intuitive way.

The rest are modules I rewrote based on what we had built at Midsummer while I was there. Nothing groundbreaking, but very useful tools when used to organize gameplay logic.

Future work for this plugin can be found [here](https://open.codecks.io/starfire/decks/48-starfire-game), but there is no timeline for when this work may be addressed.
Pull requests and feature requests are happily accepted.

## Detailed Module Descriptions

### Actor Collections

Actor Collections are custom Actor classes that can manage a collection of Actors. Which sounds a little simple and the basic collection is that simple, but it can enable other things that are more useful.

Other elements of the module
* Actor Collections have delegates that are broadcast when there is a change in its membership.
* A component that tracks the collections that an actor is a member of.
* A Filtered Actor Collection which will observe another collection and will add to its own membership if a new member of the other collection meets certain criteria (defined by the Filter Collection).
* Specialized singleton collections which enforce a single instance being created.
* A utility component which can be used to auto-add instances of Actors to a singleton collection.
* A function library of utilities for working with collection instances.

Actor collections can act as intermediary between the data (like a collection of currently selected units) and the system that is handling the input and management of the collection.

### Game Facts

Game Facts are an extension (of sorts) to Gameplay Tags, with facts being a subset of all tags.
The actual difference between the two is flexible (because it is entirely defined by the project) but conceptually facts represent state of some kind and not all tags represent state.
For example, the 'UI.Layer.' tags using with the CommonGame UI layout stacks aren't really state.
A project could define an explicit 'GameFacts' scope, but the module doesn't assume anything like that.

Gameplay Tags provide a lot of great utility but can have a couple of difficulties.
First, with assigning tags to objects you can run into problems if you're not ref-counting them or the source of the tags.
Next, save/load can become complicated because the tags applied by sources may change during development and if they are assigned to the object there can be mismatches between actual and expected when loading saves that straddle those sorts of data changes.
Finally, while there is an interface for accessing tags it is somewhat limited (not blueprint implementable) and intended for all tags (which tags are not).

Game Facts has two parts that are the fundamental basis for working with this subset of tags.
The most important is the interface, `IGameFactsProvider`, which is similar to the `IGameplayTagAssetInterface` supplied by the Engine but with blueprint support.
The other is a function library that has the public interface that should used to interact with the interface. These utilities function like blueprint interface messages and handle objects that may or may not implement the interface. It also handles a special case for `AActors` and `UActorComponents`.

Part of how these are meant to solve the issues tags has for tracking state is by inverting the relationship.
Instead of gameplay modifying the object and adding tags to a Tag Container, when an object is asked for its Game Facts it gather those from attached gameplay.
For example, if you were to get Facts from a Character it might return the tags built into the Character through blueprint configuration but the Character might delegate to a component that tracks stats which also provides a tag based on the amount of health the Character currently has.
This can make Facts incredibly dynamic with little effort and not having nearly the save/load problems because gathering facts would always return the latest version that an asset might reference.
In fact the utilities functions for getting the tags of an object even have this built-in to dealing with Actors. So the Character wouldn't even have to delegate anything. Calling the utility would already query all the attached components for any facts they wish to contribute.
This also makes it easy for Actors to participate in this interface without directly needing to directly implement it.

Of course this doesn't stop gameplay from assigning tags directly to an object. For some gameplay, like facts that are only added, this might still be the best way to associate the fact.

The Game Facts module provides a few other tools for managing these special tags.
* `FGameFactsCriteria` - a utility structure that allows you to define a way to check the tags of an object against some sort of condition. The basics are similar 'has all', 'has any', 'has none' (blocking) that Epic's GAS system leverages. It also has the option to use an `FGameplayTagQuery` for more complicated checks.
* A function library that leverages an `FGameFactsCriteria` to filter collections and remove objects that don't match the criteria.
* A simple component that wraps a `FGameplayTagContainer` and implements Game Facts Provider interface. Combined with the utilities, this makes it very convenient to add facts to instances of Actor types provided by the Engine (like Player Starts or Static Mesh Actors).
* A Game Fact Provider Volume which can add tags to an Actor for as long as they are in the volume. This is where the dynamic lookup can be very flexible both for runtime issues (overlapping volumes providing the same tag) as well as save/load (the tags being changed over the course of development).

This change from pushing to pulling is a trade-off. Getting the tags for an object is likely to be more complicated than a simple return.
However we found this trade off to be worth it for much of the development tasks that relied on facts.
And again it's worth noting that while the primary method of dealing with the tags is to pull them from various locations, nothing stops a project from still pushing some or all tags to a centralized location.

One very powerful tool involves combining the Actor Collection with Game Facts to create a collection that adds tags to its members. This would function very similar to the Fact Provider Volume.
This class is not provided so that, within this plugin, there is no dependency between these two modules. Left as an exercise to the reader.

### Game Feature Subsystems

Subsystems and Game Feature Plugins have been two incredible additions to the Engine since 4.26 & 4.27. Unfortunately, out of the box they don't play together very well.
Since subsystems only exist in C++ (as of 5.7) and C++ code is always available at runtime, you can run into some weird behavior. When a subsystem is declared in a Game Feature Plugin, it gets instantiated regardless of the state of the plugin.

One potential solution for this is to not include code in general in feature plugins. I can appreciate the simplicity of that approach, but I'm a C++ programmer and can't really abide that. The enforced separation of concerns is too helpful for preventing overly intertwined code.

Another is to only limit subsystems from feature plugins, but that feels awkward.

So instead I've tried to take an approach that provides the best of both worlds with custom subsystems that function in a way that is intuitive (at least to me) to the way these systems overlap.
The module creates new subsystem types that you would derive from to create a subsystem in a Game Feature Plugin module. I would like to find a way that doesn't require this, but suspect that is not possible without Engine changes.

The simple case where a Feature is enabled prior to a subsystem collection being created should result in the subsystem being found and created exactly the same as if it were not in a Feature.
For example, if you were to enable a Feature in your Main Menu and then load a level, the World Subsystems from the Feature would be created at exactly the same time as all of those not in the Feature.

The slightly more complicated case activating or deactivating a Feature while in a world. The basics aren't that complicated. When a Feature is activated the subsystems from that Feature will be created and on deactivation, they are destroyed.
The subsystems will be created as normal (except for the timing) with the proper outers and all the normal virtual function calls.

But you do have to be a little careful about what the subsystems expect in relation to other 'On Activation' changes the Feature may perform.
You _can_ be sure that any assets the Feature should load will have been loaded prior to ::Initialize.
You _can_ also be sure that any `UGameFeatureAction::OnGameFeatureActivating` calls will be made after `::Initialize`. This is just how the Game Feature Subsystem manager works.
You should assume that any interaction with an implementor of `IGameFeatureStateChangeObserver` is in an unknown relative order, basically random, and avoid dependencies between Observer state. Similar to the difficulties with the order of two Actor's `::BeginPlay` executions.
Again, I would like to provide some better guarantees but that would require Engine changes or being able to override behavior from the `UGameStateFeatureSubsystem` which is not currently supported.

Currently Editor and Engine subsystems are not supported and function as "normal" for subsystems & Features.
Editor subsystems seem like they should work in the Editor regardless of the state of the feature.
Similarly, Engine subsystems are such low level functionality, that they should be created as normal and themselves handle their source feature being enabled or not. However a Engine subsystem may be a reasonable addition in the future.

### Level Metadata

Worlds are a fundamental component to games in Unreal but they tend to be the largest assets of the entire project. This generally means references through soft references which can make them a little difficult to hook into gameplay.

While worlds are primary assets which allow for a lot of utility for understanding the collection of assets that are available, it doesn't help understand anything about the worlds and what they may represent.
It's not as helpful for understanding anything about the contents of the world or the context for how the world fits into the overall gameplay.

Having some kind of metadata about the worlds is incredibly helpful to gameplay and commonly involves developer settings, or tables or secondary assets with that information.

Level Metadata is a primary data asset that is designed to help make that a little easier by acting as the base for the metadata needed for by a project.

The first part is the creation of the asset which is configurable so that the asset type decides if, when saving, if a world needs a certain type of Level Metadata.
There's no reason why the level for the main menu should try to be producing the same meta data as actual gameplay maps. Same with test maps that aren't part of the gameplay loops.
The created asset is named based on three elements: the name of the World asset, a suffix and prefix that are configurable through virtual function overrides.

The second part is a hook that supports exporting data from the map into the metadata asset. This allows the world data to remain authoritative, but also make the data available while the world is not loaded.
Those properties can be visible in the editor or not depending on the property, but they shouldn't be editable. The Level Metadata has a built-in reference to the world it is associated with that is populated by the base class.
All of these properties are reset to the values from the CDO before the asset does any exporting so that there's no difference between new and updating Metadata assets.

The last part is that the project still has full control over the asset which allows adding more properties to the asset that are relevant as metadata but may not need to be things configured as part of the World.
These properties are _not_ reset during the export process so that any non-exported data isn't lost the next time the World is resaved. The Level Metadata leverages existing property markup to distinguish the properties that should and should not be reset.
Any properties that are default editable are assumed to be those that should not be reset.

### Player Modes

Player modes are like little, temporary, game modes. They exist on a stack for each player and can optionally spawn their own player pawn to create individual control schemes unique to each mode.

For example, if you where making something like XCom you could have a mode for the default looking around and planning. Then push a new mode that can modify the inputs and do any other special behaviors during targeting. Different abilities or weapons could push different modes to effect different types of targeting.

Player modes are custom Actor classes that are kept on a stack managed by a Local Player Subsystem.
As modes are pushed onto the stack, they have the opportunity to load temporary resources before having a custom activation function being triggered.
The mode also has an API that allows it to react to another mode being push on top of it.

### Starfire Game Core

Game core provides common custom implementations of the core engine classes like Game Instance, Game Mode, Game State, etc.
Most of these classes are empty, but have been made in advance to be have the class available and not have to try and insert one later into multiple projects.

The primary exception is Starfire Game Instance, which implements the common behavior I build in using the Data Definition Library (custom AssetManager from Starfire Assets) and Developer Settings Preloader interface (from Starfire Utilities) to preload certain assets when the game instance is initialized.

Starfire Game Mode does some minor setup to update the associated classes like `GameStateClass` to the Starfire versions. It also initializes the Redscreens debug system from Starfire Utilities.

## Dependencies

In addition to the dependencies on plugins from the Engine, Starfire Game is also dependent on the Starfire Assets, Utilities & UI plugins found in this repository.
However they are all marked as optional so only the dependencies for module you build should actually be relevant.

Starfire Assets can be modified out of the LevelMetadata module.
Starfire UI is a dependency that can be easily removed from the StarfireGameCore module.
Most of the module dependencies on Starfire Utilities can't be broken by duplicating the functionality found in TypeUtilitiesSF header of that plugin.
PlayerModesDeveloper has dependencies on Starfire Utilities Developer that can be removed by duplicating the functionality used from StarfireK2Utilities.h.
There are additional Starfire Utilities from StarfireGameCore, but those are more core. Again it shouldn't matter much if you don't want to use that module.

StarfireGameCore also has dependencies on CommonGame and ModularGameActors which can be found in Epic's Lyra game project. These dependencies are possible to remove with some minor edits to the plugin to change class declarations.
But again, these dependencies have been marked as optional and should be unnecessary to have if you're not using that particular module.