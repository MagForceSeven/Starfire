# Starfire Persistence

## General Description

This plugin contains two parts, Persistence and Data Actors.

Persistence provides a way (or an example way) that actors or subsystem can be identified (components or an interface respectively) to be included in a save game. It also provides an archiver that can serialize a collection of objects (provided or found using those methods). A manager is also provided for tracking those persistent objects. That manager also tracks destroyed actors so that information can be included in the archive so that actors get destroyed when loading from save data.

This saving and loading has only been used in basic level setups. It has not been used with level instances or world partition. However, there should be enough to be the basis for something that does work in those cases.

Data Actors is a scheme for creating easily persistent data. These actors are created to act as the gameplay data during runtime. These are actor that aren't meant to have visual elements, so there is a way to associate a Data Actor with another actor as a Visualizer that acts as its visual representation (if it needs one). These Visualizers can optionally be persistent as well by adding the component.
The Data Store Actors are provided as one possible example use of a Persistence Component and how it interacts with the custom `FArchiver` classes.

Future work for this plugin can be found [here](https://exilecafe.codecks.io/decks/47-starfire-persistence), but there is no timeline for when this work may be addressed.
Pull requests and feature requests are happily accepted.

## Dependencies

In addition to the dependencies on plugins from the Engine, Starfire Persistence is also dependent on the Starfire Utilities plugin found in this repository. These dependencies are actually fairly minor: _TypeUtiliesSF.h/hpp/cpp_. These files could be copied from Starfire Utilities instead of using the plugin entirely.

## Detailed Usage

`UPersistenceComponent` and `IPersistentSubsystem` are two methods provided for including objects in the archiving process.
Those objects are used as the initial set and `FReferenceFinder` is used to expand the collection of archived objects by gathering owned subobjects.

`UPersistenceManager` is a World Subsystem that maintains a collection of the actors with the Persistence Component.
It also keeps track of Actors that have been identified as 'Destroyed' so that the actor will be destroyed when the data is restored from an archive.

There are two `FArchiver` types provided, one that writes objects into a byte array and one that constructs objects from a byte array.
It can also leverage the data from the Persistence Manager to destroy actors from the level that were destroyed in a previous session.
The writing archiver has members that can be configured with functions that can prevent specific objects or components from being included in the resulting byte data.

`ADataStoreActor` is a base for actors that are meant to be pure data and persist across sessions. They have a built in Persistence Component to work with the archiver in the Persistence module.
`ADataStoreSingleton` is a specialized base for Data Store actors that should only have 1 instance created at a time. Lifetime must still be managed manually, but errors will be raised when creating multiples. There are also simplified accessor functions through the Persistent Data Store given the uniqueness of the actor instance (since they can be accessible without a GUID).

`UPersistentDataStore` is a World Subsystem that is provided to manage the collection of Data Store Actors and act as the factory for Data Store Singletons.

Visualizers are Actors with the `UDataStoreVisualizer` component. Data Store Actor types can be configured to use specific Actor classes as their visualizer, but this is not required. When configured though, child Data Store Actors must be configured with the same class as its Super type or a child of the Super's Visualizer Type. A Validator is provided that tries to prevent configuration mistakes.

## Components

### Runtime

#### Starfire Persistence

_PersistenceComponent.h/cpp_

The component that identifies an actor as persistent. Tracks a GUID and whether or not the Actor was created as part of level loading or dynamically spawned.

_PersistentSubsystemInterface.h/cpp_

Interface that identifies subsystems that should be included in the Persistent Actor Archiver process.

_PersistenceManager.h/cpp_

A World Subsystem that tracks actors with the persistence component. Also tracks the IDs of persistent actors from levels that have been marked as destroyed (so that they can be destroyed again when the level is reloaded).

_PersistentActorArchiver.h/cpp_

Implementations of FArchiver for serializing collections of persistent objects to and from a byte array.
Saves property data marked up with the SaveGame meta (or the flag set on the property in Blueprint).
Preserves owned subobjects and references to other actors in the same collection.

_StarfirePersistenceSettings.h/cpp_

Developer settings that are used to configure some of the FArchiver behavior.
Contains an permission list of the types of actor components that should be saved (since not all components need to save runtime data).

_PersistenceShared.h/cpp_

Some common data structures used by the persistence API's.

_ArchiveUtilities.h/cpp_

Some private utilities for performing the archiving process.

#### Starfire Data Actors

_DataStoreActor.h/cpp_

A Base class for actors that are to be treated as Data Store Actors.
Comes with a built-in Persistence Component from the Persistence module.

_DataStoreSingleton.h/cpp_

Specialized Data Store Actor type that are only allowed to have a single instance.
Similar to subsystems, except lifetimes are not automatically managed. Gameplay still has to manually decide to create them at the correct time.

_DataStoreVisualizer.h/cpp_

A component used to link Actors to Data Store Actors when that actor is used as a Visualizer.

_PersistentDataStore.h/cpp_

A world subsystem that manages lifetimes and tracking of Data Store Actors.

_DataStoreUtilities.h/cpp_

Various utilities for interacting with Data Store actors and visualizers from Blueprint.

_DataStoreKismetUtilities.h/cpp_

Functionality that is meant to be hidden behind custom blueprint nodes.

_VisualizerValidator.h/cpp_

A validator for making sure that visualizer types are configured in a way that doesn't break assumptions.

### Developer

There is currently nothing in this module, it is a placeholder for future features that need this module type.

### Editor

There is currently nothing in this module, it is a placeholder for future features that need this module type.