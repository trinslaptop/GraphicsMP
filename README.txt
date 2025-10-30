Trin Wasinger (Idril)
twasinger at mines dot edu
Assignment #3 (2025-10-27)

# Description
This program renders a basic 3d world (art inspired by Minecraft) with a player that can be moved around and viewed from
several different cameras. Additionally, it also implements Phong Illumination.

# Usage
To run the program, first compile it (see below), then look for an `a3` or `a3.exe` in the build directory and execute it.
Make sure you are in this current directory containing the shaders and assets folder when running

This program has a variety of different inputs:
 + ESC - Exits the program
 + LEFT CTRL + C - Also exits the program just like on the command line
 + SHIFT + Left Drag Mouse - Changes arcball camera radius
 + Scroll - Changes arcball camera radius
 + Left Drag Mouse - Moves arcball camera
 + W - Moves player forward along heading
 + S - Moves player backward along heading
 + A - Rotates player right
 + D - Rotates player left
 + 1 through 3 - Selects secondary viewport camera out of [None, First Person, Sky Camera]
For debugging:
 + C - swaps between the player centered arcball camera and a free camera
 + V - prints player position

# Compiling
Compile like normal with cmake and make. If on Linux, you can just run `bash run.sh` to regenerate, compile,
and run the program.

# Bugs, Notes, Etc...
I based the graphics for this project on Minecraft; however, all the textures are my own or were made by a (non-Mines) friend that
I have explicit permission to use. The player texture itself is my custom Minecraft skin. The format is exactly the same, so you can
drop in a different valid Minecraft skin and update the texture path at the end of `mSetupBuffers()`, and it should work.

My world has several objects made out of "blocks". There are trees, mushrooms, a pyramid of amethyst, and a few others things
scattered around the world. The world is a 64x64(x64) level with the grid plane at y=0.

For *FFP-Free*, I implemented Phong Shading (instead of Gouraud Shading). This is still the full diffuse + specular + ambient
Phong Illumination but now in the fragment shader. Doing this interpolates the normal vectors across the fragments. I also used
textures for all the components of the illumination rather than flat material colors. Each face had two textures, a diffuse texture
and a specular texture. The diffuse texture could be rgb with cutout only alpha (empty pixels are fine, but no translucency) and was
the color applied for diffuse lighting and scaled down (darkened) for ambient lighting of the face. The RGB of the specular texture
controlled the highlight color while 32 times the alpha channel controlled the shininess. For simple applications, a full opaque
white texture creates a white glint everywhere on the face (angle permitting), see shiny.png. A fully transparent texture makes
a face completely dull. Some blocks in the world like wood are dull while others like leaves and amethyst are shiny. Additionally,
since I can use complex textures for the specular and not single values, I can make only parts of a face shiny. On the player model,
the metal decorations (belt, head, arm) and skin are shiny while the leather and cloth is not. Similarly, the cape is mostly dull
but has a few specular regions. See idril_specular.png and cape_specular.png.

Instead of a directional light for this assignment, I used a point light. It is located between the pyramid and center of the map
(look at the ground to tell where). It has a slightly warm tint to it.

I also implemented very basic collision. Solid blocks (wood, amethyst) but not plants (mushrooms, leaves) will block the player from passing
through. The player is two blocks tall, and collision is checked at each.

The player has a basic idle animation consisting of arm swing, minor head rotation, and cape blowing. While walking, there is also a leg
swing animation.

For *Doom iddt*, I also implemented a first person camera and sky camera. These render in a viewport at the top left and can be selected with
keys 2 and 3 respectively. Pressing 1 hides the additional viewport. (There's also a completely separate free camera accessible by C if you
want to look around the scene better).

Some helper classes were implemented as header-only files; while not ideal for compilation time, it had negligible impact and keeps code simple
(plus the CSCI441 library does it already).

One minor known bug/artifact is that moving too far from the player may cause the different layers (e.g. arm vs sleeve) to start z-fighting.

# Class Stuff
This assignment took around 43 hours.

The labs were helpful guidance, but I had to create my own starting point from scratch (7/10).

Debugging a gray screen is never fun, but otherwise, this was very a fun assignment (9/10).