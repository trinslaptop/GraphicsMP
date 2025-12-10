Trin Wasinger (Idril)
twasinger at mines dot edu

Guild: Colorado School of Mines of Moria

A4 (2025-12-12)

# Description
This program renders a 3d cube world (art inspired by Minecraft) and implements AABB collision. The player can run around collecting 
as many diamonds as possible while avoiding zombies!

# Usage
To run the program, first compile it (see below), then look for a `a4` or `a4.exe` in the build directory and execute it.
Make sure you are in this current directory containing the shaders and assets folder when running!

This program has a variety of different inputs:
 + ESC - Exits the program
 + LEFT CTRL + C - Also exits the program just like on the command line
 + SHIFT + Left Drag Mouse - Changes arcball camera radius
 + Scroll - Also changes arcball camera radius
 + Left Drag Mouse - Moves arcball camera
 + W - Moves player forward along heading
 + S - Moves player backward along heading
 + A - Rotates player right
 + D - Rotates player left
 + SPACE - Jump
 + F5 - Switches between arcball and freecam in primary view
 + F4 - Switches between first person, skycam, and no camera in secondary view
 + F2 - Take PNG screenshot
 + F3 - Toggle debug HUD (Position, region, etc...)
 + Secret cheat codes (try classic ones)?

This program also supports console commands for debugging on stdin:
(These aren't intended for normal use and may have unexpected effects!)
 + `exit` - Exits the program
 + `tp x y z` - Moves player to a given position (insert numbers for x, y, z)
 + `setblock x y z name` - Places a block in the world, valid blocks are `air`, `planks`, `leaves`, `log`, `glass`, 
   `amethyst`, `mushroom`, and `tallgrass` (insert integers for x, y, z) 
 + `summon zombie` - Adds an additional zombie to the game
 + `md5play path/to/file.txt` - Plays an MD5 camera track file
 + `unstuck` - Resets player and diamond positions
 + Secret cheat codes?

# Compiling
Compile like normal with cmake and make. If on Linux, you can just run `./run.sh` to regenerate, compile, and run the program.

If you're on Linux and have HTTPS capable libcurl, you can set `-DUSE_CURL=True` for cmake. Doing so and providing a real Minecraft username
as the first argument on the command will fetch and use that player's Minecraft skin (and cape if applicable)! This requires a network connection!

E.g. `./run.sh -DUSE_CURL=True -- Technoblade`

# Bugs, Notes, Etc...
I based the graphics for this project on Minecraft; however, all the textures are my own or were made by a (non-Mines) friend that
I have explicit permission to use. Like last time, the player skin format is exactly the same as official Minecraft ones. There's my skin builtin,
but see the notes above about using libcurl to fetch any player's skin.

The goal is to collect as many diamonds as possible without dying. When you pick up one, another is placed randomly. The ground and most blocks have
collision, but leaves don't so that you can intentionally move through them. You can see your health, diamonds, and deaths on the screen. When you
pick up a diamond, there is a chance another zombie will spawn. If you or zombies fall off the world, they will die. When zombies hit the player or
each other, they have a chance to deal damage. After dying, the player respawns and the death counter goes up. When the player or zombies take damage,
they animate going red and recoiling from the hit.

Press F3 to enable debug view and see the collision checks. I think it's really cool to watch! Green cubes are the areas it checks, white is the hitbox.

The zombies slowly track the player but can get stuck behind blocks since they have no path finding.

The player and zombie have a basic idle animation consisting of arm swing, minor head rotation, and cape blowing. While walking, there is also a leg
swing animation.

The world has several objects made out of "blocks". There are trees, mushrooms, grass, a pyramid of amethyst, and a few others things
scattered around the world. The grass and leaves wave in the wind, and the mushrooms have animated textures (A lot of the animations are
pretty subtle so as to not be garish). There's also indicators of where light sources are. The torch atop the pyramid is a slightly orange
point light, and the directional light comes from approximately the sun's angle. All applicable lights are attenuated. 

The world is a 64x64 block level. It has a hidden grid at y=0 carried over from A3, but it also has a tessellated bezier patch surface with a grass
texture to create nice hills and elevation. These are procedurally generated with Perlin-like noise. Players and zombies move with the terrain.
In the MP, the player was oriented with the terrain too; however, I removed this feature because I personally didn't like how it looked, and it overly
complicated AABB collision and jumping.

The skybox uses a cubemap to efficiently render the textures around the player. This was based on https://learnopengl.com/Advanced-OpenGL/Cubemaps,
and used a free skybox texture from http://www.humus.name. There is also a animated cloud plane. Look for easter eggs in the clouds... there's a heart
and my name.

In addition to the arcball camera and free camera, there's also a bonus skycamera and first person view that can be shown in the secondary viewport.
You can also play md5camera tracks (format described on the class website) via the console. There's a sample track in the data folder. To generate
camera tracks, you can use edit the md5camera.py script to easily add rotations around a point, movements along a line, and more. 

Some additional but not required features:
 - Tinted textures (e.g. grass.png is white and is then multiplied by a variable green color, all spotlights use the same texture)
 - Animated textures from vertically stacked texture files
 - Using textures for specular color and shininess
 - Cutout textures

Some helper classes were implemented as header-only files; while not ideal for compilation time, it had negligible impact and keeps code simple
(plus the CSCI441 library does it already).

One minor known bug/artifact is that moving too far from the player may cause the different layers (e.g. arm vs sleeve) to start z-fighting.

Gravity isn't physically accurate, but it still looks and plays fine. It is occasionally possible to glitch through blocks.

I implemented a bunch of other utilities like an `#include` processor and `json` parser for this project!

# Class Stuff
This was based on my A3 and midterm project.

This assignment took an insane amount of time. I have no idea exactly how much but many full days were spent on it.

The billboarding lab was helpful, but I did the collision from scratch (5/10).

This was very a fun assignment, and I think it looks awesome (10/10)!