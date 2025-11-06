Trin Wasinger (Idril, Player 1)
twasinger at mines dot edu

Samvat Dangol (Fuyuzetsu Toyota, Player 2)
sdangol1 at mines dot edu

Thuan Nguyen (Jon Snow, Player 3)
thuannguyen at mines dot edu

Guild: Colorado School of Mines of Moria

Midterm Project (2025-11-06)

# Description
This program renders a basic 3d world (art inspired by Minecraft) with three players that can be moved around and viewed from
several different cameras. Additionally, it also implements Phong Illumination and several types of lights.

# Usage
To run the program, first compile it (see below), then look for a `mp` or `mp.exe` in the build directory and execute it.
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
 + 1 through 3 - Selects different players
 + C - Switches between arcball and freecam in primary view
 + V - Switches between first person, skycam, and no camera in secondary view
 + Space - Takes screenshot
 + P - Plays md5camera movie (See notes below)
For debugging:
 + Z - prints player position

# Compiling
Compile like normal with cmake and make. If on Linux, you can just run `./run.sh` to regenerate, compile,
and run the program.

# Bugs, Notes, Etc...
Idril based the graphics for this project on Minecraft; however, all the textures are their own or were made by a (non-Mines) friend that
Idril has explicit permission to use. The player textures are our custom Minecraft skins. The format is exactly the same, so you can
drop in a different valid Minecraft skin and update the texture path at the end of `mSetupBuffers()`, and it should work.

The player has a basic idle animation consisting of arm swing, minor head rotation, and cape blowing. While walking, there is also a leg
swing animation.

The world has several objects made out of "blocks". There are trees, mushrooms, grass, a pyramid of amethyst, and a few others things
scattered around the world. The grass and leaves wave in the wind, and the mushrooms have animated textures (A lot of the animations are
pretty subtle so as to not be garish). There's also indicators of where light sources are. The torch atop the pyramid is a slightly orange
point light, the three spotlights have shells at their source, and the directional light comes from approximately the sun's angle. All applicable
lights are attenuated. If you stand under the spotlights, you can see the different RGB colors mix depending on cone overlap.

The world is a 64x64(x64) level. It has a hidden grid at y=0 carried over from A3, but it also has a tessellated patch surface with a grass texture.
Players move along and are oriented with this terrain. Instead of using bezier curves, it is based off of a sine and cosine equation that
was designed to give pretty results.

The skybox uses a cubemap to efficiently render the textures around the player. This was based on https://learnopengl.com/Advanced-OpenGL/Cubemaps,
and used a free skybox texture from http://www.humus.name.

In addition to the required cameras, there's also a bonus skycamera that can be shown in the secondary viewport. You can also play md5camera tracks
(format described on the class website) by pressing P in-game and entering a file name into the console (Note: not prompted at startup so as to not
block normal usage). There's a sample track in the data folder. To generate camera tracks, you can use edit the md5camera.py script to easily add
rotations around a point, movements along a line, and more. 

Some additional but not required features:
 - Basic collision on solid blocks
 - Tinted textures (e.g. grass.png is white and is then multiplied by a variable green color, all spotlights use the same texture)
 - Animated textures from vertically stacked texture files
 - Using textures for specular color and shininess
 - Cutout textures

Some helper classes were implemented as header-only files; while not ideal for compilation time, it had negligible impact and keeps code simple
(plus the CSCI441 library does it already).

One minor known bug/artifact is that moving too far from the player may cause the different layers (e.g. arm vs sleeve) to start z-fighting.

# Class Stuff
Contributions:
 - Idril - This project was *heavily* built on Trin's A3 which provided a lot of needed features already, also did the skybox, terrain,
   and a bunch of other features for fun that aren't required but look cool (animated textures, waving plants, tinted textures)
 - Toyota - XCode bug fixes, added additional players
 - Jon Snow - Spotlights, controls, added character

This assignment took around 16 hours in addition to the work carried over from Idril's A3.

The labs were helpful guidance for A3 and a starting point for some features, but weren't used much here (7/10).

This was very a fun assignment, and I think it looks awesome (10/10)!