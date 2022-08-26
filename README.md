# glTFRuntimeConvexCollisions

Convex Collisions support for glTFRuntime

This plugin makes use of the amazing V-HACD library (https://github.com/kmammou/v-hacd used by Unreal too) for generating Convex Collisions.

You can use this plugin as a base for adding more runtime features to your StaticMeshes or to understand the "CustomConfig" system used by glTFRuntime.

## Quickstart

Once the plugin is enabled you will get a Data Asset called 'glTFRuntimeConvexCollisionConfig'. Just create one in the Content Browser from the Miscellaneous/Data Asset menu:

![Screenshot0](Screenshot0.PNG?raw=true "Screenshot0")

This asset defines the Convex Collision generation policy (basically the same stuff you have in the standard Editor)

![Screenshot1](Screenshot1.PNG?raw=true "Screenshot1")

Now to enable Convex Collision generation just edit your StaticMeshConfig structure setting its outer to the StaticMeshComponent (something you generally do for Complex collisions) and adding the glTFRuntimeConvexCollisionConfig asset to the list of 'Custom Config Objects'

![Screenshot2](Screenshot2.PNG?raw=true "Screenshot2")

## Advanced usage (async mode)

Instead of creating an asset in your project, you can create your configuration at runtime. This will allows you to enable async mode too:

![Screenshot3](Screenshot3.PNG?raw=true "Screenshot3")

Notice that the async event returns the "Convex Elements" array too. You can reuse/cache/store this array in some way for reusing it (just assign it to the 'Convex Elements' field in the glTFRuntimeConvexCollisionConfig object and disable the 'Auto Convex Collision' generation)
