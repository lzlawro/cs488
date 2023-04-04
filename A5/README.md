# CS488 Winter 2023 Assignment 5

## Compilation
No changes were made to the default `premake4/make` combination.

No lab machine was used to test the code.

The code was tested on Thinkpad T14 Gen 2 with Windows 10 running the VM.

## How to run program
After the `premake4/make` combination, run `./A5 Assets/sample.lua` in the `A5` directory.

Operations and their desgnated hotkeys are shown on the GUI menu, except for pressing “M” to show/hide GUI.

## List of objectives
1. Modelling the scene: A scene is properly modelled and rendered with a visible surface water plane

2. User Interface: The user can perform some operations to manipulate the scene. For example, dragging or clicking on the surface plane produces ripples.

3. Texture mapping: texture applied on water plane itself, or on other surfaces of the rectangular water body (they can be walls of a pool of water), or on other additional visual elements.

4. Physics engine. At least a sphere should be introduced to the scene, and the sphere should be affected by both gravity and water buoyancy.

5. Static collision detection. Ripples are produced when the sphere touches the water surface (or some other ways to indicate the sphere touching the water plane).

6. Bump mapping. The sphere is given some bumpy appearance.

7. When the water surface is interacted with (clicked by user or touched by sphere), 
sound effect will be played.

8. Transparency using alpha value. If transparency is implemented, the water body will be transparent just like real water,
i.e. the bottom of the pool can be seen by looking at the water surface.

9. Shadow using shadow MVP matrix. At least the sphere needs to have shadow projected to some solid surface, or to the water surface.

10. Particle system. When the water produces ripples, particles are also emitted from the center of the ripple.
