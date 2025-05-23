# gradius-c
 Recreation of gradius in C and OpenGL, with no other external libraries or engines. OpenGL framework and input code provided, but rest all custom. 

# How to run:
Either download and compile through visual studio, or download, extract and run the compiled game via releases.

# How to play:
Arrow keys to move, z to shoot, x to activate your current powerup.

# Features:
## Collision System:
AABB collision detection system. Collisions call object-specific callback functions, that work with an object tag system to define behaviors for things like projectiles colliding with enemies, enemies colliding with the player, and so on.

## Level Definition:
Level can be modified directly through a .csv file, allowing for rapid iteration with needing to compile. Players progress through a level and lose some of that progress upon death.

## Object-Oriented Structue:
Used structs and vtables to mimic object-oriented logic within C.

## Content
Power-up system leads to 4 distinct shot types that can be combined, as well as stat upgrades and a follower AI. Six different types of enemies and a boss were implemented, all with their own AI. 

## Sprite Animations
While a graphics framework was provided, everything other than displaying an image was custom. Animations support through spritesheets. 
