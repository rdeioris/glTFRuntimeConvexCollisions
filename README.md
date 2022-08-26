# glTFRuntimeConvexCollisions

Convex Collisions support for glTFRuntime

This plugin makes use of the amazing V-HACD library (https://github.com/kmammou/v-hacd used by Unreal too) for generating Convex Collisions.

You can use this plugin as a base for adding more runtime features to your StaticMeshes or to understand the "CustomConfig" system used by glTFRuntime.

## Quickstart

Once the plugin is enabled you will get a Data Asset called 'glTFRuntimeConvexCollisionConfig'. Just create one in the Content Browser from the Miscellaneous/Data Asset menu:

This asset defines the Convex Collision generation policy (basically the same stuff you have in the standard Editor)


Now to enable Convex Collision generation just edit your StaticMeshConfig structure setting its outer to the StaticMeshComponent (something you generally do for Complex collisions) and adding the glTFRuntimeConvexCollisionConfig asset to the list of 'Custom Config Objects'
