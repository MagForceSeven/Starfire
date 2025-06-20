# Starfire Utilities

A grab bag of random utilities useful to any sort of game being developed in UE5.

If you're not interested in this entire plugin, most of the code can be copied as individual source files. There are some dependencies across them, some of them can be removed and others require copying a little more code. For example all the custom K2 Nodes are dependent on StarfireK2Utilities.h/cpp.

The plugin contains 3 modules: Starfire Utilities (Runtime), Starfire Utilities Developer (Uncooked Only), & Starfire Utilities Editor (Editor)

Planned/possible future work can be found [here](https://open.codecks.io/starfire/decks/38-starfire-utilities). There is no timeline for when any work tracked there might be completed.

## Runtime

### Redscreens
_Redscreen.h/cpp_, _RedscreenManager.h/cpp_, _RedscreenScreen.h/cpp_, _RedScreen_W.uasset_, _Redscreens.ini_

This is an error reporting tool that acts sort of like an in-game ensure. When a redscreen happens in code/blueprint, a full screen widget is popped up that lasts for a few seconds. Just long enough to get a jist of the message and connect it temporally to what was going on in the game. The message is also mirrored to the log, so it's possible to go back and get the full text of the message as well. A basic widget is provided by this plugin which places a thick red rectangle around the screen. This makes it highly visible, but doesn't overly impeed play in action games. That widget can easily be swapped out for a more project specific widget through an ini setting.

If you want these redscreens showing up in non-shipping, packaged builds you'll want to add this plugin's content directory to the additional directories to cook in your project settings.

Redscreens are best for occasional errors or content configuration errors that are difficult to determine through some form of asset validation (such as when dealing with combinatorics of Asset A + Asset B). They should not be used for anything per-frame or general on-screen messaging. Redscreens should be considered an error with _something_ that can be addressed mostly through the information provided in the redscreen. There are two macros, Redscreenf and RedscreenOncef, the latter will only show messages once even if tripped again, but it still shouldn't be used in cases where the error is happening every frame.

### Blueprint Async Action
_BlueprintAsyncActionsSF.h/cpp_

A layer on top of the the BlueprintAsyncActionBase that adds a slightly more convienent way to do a blueprint async action that has a ticking component instead of resorting to an FLatentActionInfo.

### Blueprint Utilities
_BlueprintUtilitiesSF.h/hpp/cpp_

A collection of standardized enumerations that work really, really well with the ExpandEnumAsExec meta. No reason to redeclare these all the time.

A collection of array casting utilities to get around issues with arrays of const pointers because Blueprint assumes they're always arrays of non-const pointers. This generates compilation issues in the generated code for blueprint callable functions. Generally used in conjuction with blueprint only functions or CustomThunks.

_BlueprintContainerExtensions.h/cpp_

A couple blueprint internal-use-only functions for TSet and TMap containers with custom functions that support the custom ForEach nodes written for each container type.

This code is also pending as pull requests [here](https://github.com/EpicGames/UnrealEngine/pull/10355) and [here](https://github.com/EpicGames/UnrealEngine/pull/11622).

_BlueprintDevUtilities.h/cpp_

I found it amazing that there weren't any ways to check the build configuration from blueprint so I created a few functions for that.

I also found it useful to be able to trigger ensures and checks directly from blueprint. Your mileage on that may very depending on who's on your team.

### Invoked Scope
_InvokedScope.h/cpp_

A clever little macro that makes immediately invoked lambdas have a slightly nicer form within the flow of a function. Borrowed from https://github.com/VoxelPlugin/VoxelCore.

```
const int  = INVOKED_SCOPE
{
  if (LocalFunctionCondition1)
    return Value1;
  if (LocalFunctionConditions2)
    return Value2;

  return Value3;
};
```

### On Scope Exit
_OnScopeExit.h_

Another macro borrowed from https://github.com/VoxelPlugin/VoxelCore that provides an alternate form of Epic's ON_SCOPE_EXIT macro to allow the scope to be given a name in addition to the autogenerated name the macro generates to create a unique variable.

### Developer Settings Preloader
_DevSettingsPreloader.h/hpp/cpp_

A utility interface to make soft pointers in Developer Settings objects more convenient and preloadable instead of having to hit LoadSynchronous calls when you're not sure if that content will be loaded by something else.

Once you add the interface to the Developer Setting class and implement the PreloadAll function, you just need to call the two static functions at some point during your startup. I suggest GameInstance init and shutdown, but it doesn't have to be.

Currently no blueprint support, but Developer Settings aren't supported as Blueprint subtypes anyway (yet)

### Self Registering Exec Helper
_ExecSF.h/hpp/cpp_

A utility that leverages the self registering execs to provide another (sometimes easier) way to do console commands than having to mess around with the cheat manager or reflection limitations.

It has the downside that the input params aren't directly visible when triggering the exec and that the input is a string, but the code includes utilities to make that less painful and sufficiently typesafe. The conversion from string to value is also extensible through template "trickery" for any project/plugin specific types. See _Exec_DataDefinition.h/cpp_ in Starfire Assets for an example of such an extension.

### Type Utilities
_TypeUtilities.h/hpp/cpp_

A collection of type related helpers: concepts for interfaces, uobjects and actors, template accessors to StaticClass's and implementations of a few standard library concepts for platforms that don't properly support the ones that are desired for writing other concepts. The PS5 I know needs this so it enables it for that platform. Other platforms can be added directly or by adding "STARFIRE_PLATFORM_MISSING_CONCEPTS=1" as a project definition to your Target.cs.

_ArrayTypeUtilities.h/hpp/cpp_

Typesafe casting options for up and down casting arrays of pointers and soft pointers. More user friendly than the TArrayCaster or direct reinterpret casts (though they are implemented through reinterpret casts). In the case of downcasts, the caller is still responsible for guaranteeing that all the elements of the array are of the expected type.

Also provides some helper utilities for casting from arrays of uobject pointers to interface pointers. These will only work for interfaces that aren't blueprint implementatble. I want to figure out a way to check that out at compile time with a concept or something, but it's a bit of a trap right now. But at least it's only as much of a trap as any other cast-to-interface.

### Math Utilities
_StarfireMath.h/cpp_

A place for math utilities that aren't provided by Epic. Just a couple of utilities for rounding values that can be more easily selected at runtime using an enumeration to select rounding preference.

### Splat Task Manager
_SplatTaskManager.h/hpp/cpp_

This was a utility that I wrote to amoratize large amounts of work across multiple frames (but still on the game thread), sometimes known as "splatting". It has two main modes of operation: 1) a simple functor that is called every frame until it reports completion, or 2) a data collection and a functor to operate on an individual element of that collection. The manager then applies the functor to each element in the collection until a preset count or time budget is reached. The remaining elements are processed on the next frame.

And before anyone jumps in, _yes_ I'm aware of coroutines and _yes_ I'm aware of the UE5Coro plugin. This can naively look like a poor-man's coroutine manager and there's definitely some truth in that. I definitely need to learn more about them, but I suspect that if I wanted something that handled both coroutines and the throttling I'll still need something similar to this.

### Latent Action Utilities
_LatentAction_Simple.h/hpp/cpp_, _LatentAction_Polling.h/cpp_

More utilities for managing tasks over multiple frames, but these ones are based on the FPendingLatentAction class. The simple version ticks every frame until it returns done while the polling version makes codifies the case where logic is broken up into 'update' and 'check done' behaviors/calbacks.

Again, this could probably be replaced by coroutines.

### Container Randomizing Utilties
_ContainerRandUtilities.h/hpp/cpp_

A few utilities for doing random things with various Unreal container types like selecting a random element or selecting and removing a random element.

### Native Gameplay Tags
_NativeGameplayTags_SF.h_

Another variant of the Native Gameplay Tag declaration macro that allows them to be declared in a `struct` or `class` scope.

This is also pending as an Engine [pull request](https://github.com/EpicGames/UnrealEngine/pull/10365).

### Array Caster
_SFArrayCaster.h_

A duplicate of the `TArrayCaster` from Epic's _GeneratedCodeHelpers.h_ because that header doesn't compile without an engine change.

The ArrayTypeUtilities function used to be written using the ArrayCaster instead of direct reinterpret casts, but I still have other code that isn't an up/down/interface cast and I find using this to be slightly better syntax than the reinterpret cast for general code.

## Developer

### Starfire K2 Utilities
A collection of functions I've developed over time to make it easier to develop custom K2 Nodes. Highlights include functions for creating and connecting output execs based on multicast delegates, creating additional input pins based on a UFUNCTION, a utility for formatting pin tooltips, a utility for updating existing connections when the pin type is changing and a few utilities for dealing with a few node UI elements.

### Array For Each
_K2Node_NativeForEach.h/cpp_, Starfire K2 Utilities

A reimplementation of the For Each macro for iterating a TArray. Since Epic implemented this as a macro, it's available for use by custom K2 nodes like TMap For Each. UI overrides have been removed to prevent any confusion with two For Each nodes available when working in Blueprint.

### TMap For Each
My [pull request](https://github.com/EpicGames/UnrealEngine/pull/10355) was merged and included in the 5.6 release of the Engine. The custom node for this has been removed from this plugin and redirectors included that should make updates seamless. Any existing For Each (Map) nodes should use the Engine version on load or compile.

### TSet For Each
The [pull request](https://github.com/EpicGames/UnrealEngine/pull/10355) was merged and included in the 5.6 release of the Engine. The custom node for this has been removed from this plugin and redirectors included that should make updates seamless. Any existing For Each (Map) nodes should use the Engine version on load or compile.

### Delegate Param Function
![image](https://github.com/MagForceSeven/Starfire/assets/30977279/ed8e1255-0db9-4dd0-99d9-c1ab458f9652)

_K2Node_DelegateParamFunction.h/cpp_, _K2Interface_SelectDelegate.h_, _SGraphNode_K2SelectDelegate.h/cpp_, Starfire K2 Utilities

A base class K2 node to aid in creating nodes for functions that have a single delegate as one of it's inputs. Simplifies the regular combination of autogenerated function nodes and a Create Event node.

### Dependent Output Type Node

_K2Node_DependentOutputTypeNode_, Starfire K2 Utilities

An abstract base class K2 node to aid in creating nodes that have an output pin whose type is compile-time deterministic based on the input pin type.

### Bind Delegate
![image](https://github.com/MagForceSeven/Starfire/assets/30977279/3455c0a1-ed6a-4c0b-ba9c-da5a7120f649)

_K2Node_BindDelegate.h/cpp_, _K2Interface_SelectDelegate.h_, _SGraphNode_K2SelectDelegate.h/cpp_, Starfire K2 Utilities

I personally got tired of the workflow for delegate binding caused by the fact that individual blueprint nodes are created for binding to delegates. This node solves that problem by making the delegate to bind to a dropdown selection based on the Target input pin.

The second dropdown is similar to the Create Event node and allows selecting the callback function from the Listener input. Also just like Create Event, it has options for creating a function or event that matches the required delegate signature.

Double clicking on the node will jump the view to the function that will be called when the delegate is executed or broadcast.

### Is Valid Object
![image](https://github.com/MagForceSeven/Starfire/assets/30977279/22eac25d-9c01-4132-b360-425e6ad663c5)

_K2Node_IsValidObject.h/cpp_, Starfire K2 Utilities

A reimplementation of the Is Valid Object macro for branching execution based on an object instance. Since Epic implemented this as a macro, it's unavailable for use by custom K2 nodes which is unfortunate.

One major difference (and the reason it is available to the user) is the pass through pin. The input pin is connected to the output pin to aid in making overlaps and bathtubs (reroute lines below the Is Valid node check) less prevelent. It should also be able to help with validating results from pure functions that are meant to be attached to multiple input pins as they could be attached to the output of this Is Valid node instead of attaching the output of the pure node to multiple input pins (a potential blueprint hazard).

## Editor

### Default Events Manager
_DefaultEventsManager.h/cpp_

This is an Editor Subsystem to help with the default nodes that appear on a blueprint's event graph. At a minimum `FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent` can be useed to setup these default nodes. However if you've already created a blueprint asset and modify the set of function that should show by default you're out of luck.

This subsystem hooks into a few Editor delegates so that when opening blueprints for edit, it can reconcile the set of functions that should be there with those that are. Currently it will only add missing functions, it will not remove unused functions that are no longer defaults. All the functions added will be in the disabled state just like the default functions when creating a new blueprint.
