Trin Wasinger (Idril)
twasinger at mines dot edu

Samvat Dangol (Fuyuzetsu Toyota)
sdangol1 at mines dot edu

Thuan Nguyen (Jon Snow)
thuannguyen at mines dot edu

Guild: Colorado School of Mines of Moria

FP (2025-12-17)

=========================================================================

# Description

This program renders a 3d cube world game (inspired by Minecraft). It's got collision, procedural generation, fancy lighting, shadows, 
a daylight cycle, various blocks, particles, sound, and more! The goal is to pick up as many diamonds as possible.

=========================================================================

# Usage

To run the program, first compile it (see below), then look for a `mc` or `mc.exe` in the build directory and execute it.
Make sure you are in this current directory containing the shaders, assets, and data folder when running!

This program has a lot of different inputs:
 + ESC - Exits the program
 + LEFT CTRL + C - Also exits the program just like on the command line
 + ALT + Left Drag Mouse - Changes arcball camera radius
 + Scroll - Also changes arcball camera radius
 + Left Drag Mouse - Moves arcball camera
 + W - Moves player forward along heading
 + S - Moves player backward along heading
 + A - Rotates player right
 + D - Rotates player left
 + SPACE - Jump
 + LEFT SHIFT - Crouch/Sneak
 + F5 - Switches between arcball and freecam in primary view
 + F4 - Switches between first person, skycam, and no camera in secondary view
 + F3 - Toggle debug HUD (Position, region, etc...)
 + F2 - Take PNG screenshot
 + Secret cheat codes (try classic ones)?

This program also supports console commands for debugging on stdin:
(These aren't intended for normal use and may have unexpected effects!)
 + `exit` - Exits the program
 + `tp x y z` - Moves player to a given position (insert numbers for x, y, z)
 + `setblock x y z name` - Places a block in the world, valid blocks are `air`, `planks`, `leaves`, `log`, `glass`, 
   `amethyst`, `mushroom`, and `tallgrass` (insert integers for x, y, z) 
 + `summon zombie` - Adds an additional zombie to the game
 + `summon wolf` - Adds an additional wolf to the game
 + `md5play path/to/file.txt` - Plays an MD5 camera track file
 + `playsound path/to/file.wav` - Plays an wav audio file, see notes below about OpenAL
 + `stopsound` - Stops playing audio, see notes below about OpenAL
 + `unstuck` - Resets player and diamond positions
 + Secret cheat codes?

=========================================================================

# Compiling

Compile like normal with cmake and make. If on Linux, you can just run `./run.sh` to regenerate, compile, and run the program.

If you're on Linux and have HTTPS capable libcurl, you can set `-DUSE_CURL=True` for cmake. Doing so and providing a real Minecraft username
as the first argument on the command will fetch and use that player's Minecraft skin (and cape if applicable)! This requires a network connection!

If you're on Linux and have OpenAL and ALUT, you can set `-DUSE_OPENAL=True` for cmake. Doing so enables sound playback. While nothing makes sound
by itself yet, you can test out the `playsound assets/sounds/crystal_forest.wav` for some nice Minecraft-like background music (Made by me!).

Also for linux users, you can set `-DOPTIMIZE=True` if you need to improve performance.

E.g. `./run.sh -DUSE_CURL=True -DUSE_OPENAL=True -DOPTIMIZE=True -- Technoblade`

(You could probably get these options working for Mac or Windows too, but I'm not able to test that...)

=========================================================================

# Bugs, Notes, Etc...

Trin based the graphics for this project on Minecraft; however, all the textures are their own or were made by a (non-Mines) friend that
they have explicit permission to use. Like last time, the player skin format is exactly the same as official Minecraft ones. There's Trin's skin builtin,
but see the notes above about using libcurl to fetch any player's skin.

+ The name "Minceraft" is an intentional misspelling of Minecraft
+ The goal is to collect as many diamonds as possible.
+ When you pick up one, another is placed randomly, and there is a chance for another zombie to spawn.
+ You can see your health, diamonds, and deaths on the screen.
+ If the player or other mobs fall off the world, they will die. 
+ When zombies hit the player or each other, they have a chance to deal damage.
+ After dying, the player respawns and the death counter goes up. Wolves can also respawn.
+ When the player or zombies take damage, they animate going red and recoiling from the hit.
+ The zombies slowly track the player but can get stuck behind blocks since they have no path finding.
+ Wolves are friends. They're sooo cute!!!
+ The player and mobs have a basic idle animation consisting of arm swing, minor head rotation, and cape blowing. While walking, there is also a leg
  swing animation (it even stops when you do!)
+ The player can jump and sneak, sneaking gradually puts you into a different pose and changes your hitbox!

+ The ground and most blocks have collision, but leaves don't so that you can intentionally move through them.
+ Press F3 to enable debug view:
  + Debug mode shows collision checks. I think it's really cool to watch! Green cubes are the areas it checks, white is the hitbox.
  + It also shows the firefly bezier curve and particles

+ The world has several objects made out of "blocks".
+ The world is a 64x64 block level.
+ Trees, mushrooms, and grass are procedurally generated with Fractal Brownian Motion (Perlin-like) noise.
+ There's a hidden grid at y=0 carried over from A3, but it also has a procedurally generated then tessellated bezier patch surface with a grass
  texture to create nice hills and elevation. Players and creatures move with the terrain. In the MP, the player was oriented with the terrain too;
  however, Trin removed this feature because they personally didn't like how it looked, and it overly complicated AABB collision and jumping.
+ There's also a pyramid of amethyst and a few others things scattered around the world.
+ The grass and leaves wave in the wind, and the mushrooms have animated textures (A lot of the animations are pretty subtle so as to not be garish).
+ The torches are slightly orange point lights and are attenuated. 
+ Torches give of smoke and ember particles.
+ Walking gives of dust particles.

+ The skybox uses cubemaps to efficiently render the textures around the player. This was based on https://learnopengl.com/Advanced-OpenGL/Cubemaps.
+ There is also a animated cloud plane. Look for easter eggs in the clouds.
+ The world has a daylight cycle:
  + The sun and moon orbit the world
  + Light intensity and direction depend on the sun position
  + The moon has full, waning, new, and waxing phases that go in order with the day
  + Clouds are darker at night
  + Right at sunset
  + Sunlight is normally bluish white but is golden right around sunrise and sunset.
  + Clouds turn a pretty marbled pink and orange at sunrise and sunset
  + Starts twinkle at night
  + There's shadows! (Only for the sun, not torches)
  + The sun and moon are generated COMPLETELY via shaders, no VBO. It pulls the same direction the lighting uses then projects the textures onto the sky.

+ Fireflies spawn in a swarm at night (not visible during day):
  + They float around and change color.
  + While each individual firefly has it's own path, the swarm follows a bezier curve in a close loop around the world. It's got arc length
    parameterization so that it's smooth and was designed to be C(1) continuous. See notes below about `fireflies.json`.

+ There's a ton of assorted shaders, and Trin shimmed ShaderProgram to support `#includes`.
+ To aid in using lots of shaders with shared uniforms, the program uses UBOs.

+ In addition to the arcball camera and free camera, there's also a bonus skycamera and first person view that can be shown in the secondary viewport.
+ You can also play md5camera tracks (format described on the class website) via the console. There's a sample track in the data folder. To generate
  camera tracks, you can use edit the md5camera.py script to easily add rotations around a point, movements along a line, and more. 

Some additional but not required features:
+ Tinted textures (e.g. grass.png is white and is then multiplied by a variable green color, all spotlights use the same texture)
+ Animated textures from vertically stacked texture files
+ Using textures for specular color and shininess
+ Cutout textures

+ Some helper classes were implemented as header-only files; while not ideal for compilation time, it had negligible impact and keeps code simple
  (plus the CSCI441 library does it already).
+ One minor known bug/artifact is that moving too far from the player may cause the different layers (e.g. arm vs sleeve) to start z-fighting. Also,
  lag-spikes may momentarily break collision. Gravity isn't physically accurate, but it still looks and plays fine.

=========================================================================

# Files and Configuration

Player skins expect the exact same format as Minecraft. If compiled with support, you can pass a player name on the command line to
fetch that skin from the official APIs (Also fetches cape if present). Alternatively, add your own asset file and tweak the paths in `Player.hpp`.

The `md5play` command expects a movie track in the same format as previous assignments. (Like before, you can use `md5camera.py` to generate tracks.)

All other configuration is done using JSON files. (Note, there's a few very minor differences in this program's JSON parser; see `include/json.hpp` for
details, but you shouldn't have any issues)

The file `data/config.json` can be used to control general program properties. Make an object with any of the following keys:
+ `"day_length"` - How many seconds make up a single in-game day (defaults to 1440 aka 24 minutes), affects daylight cycle
+ `"width"` - How wide to make the window (defaults to 720)
+ `"height"` - How high to make the window (defaults to 720)

The file `data/world/fireflies.json` controls the path of the fireflies in-game. It should be an object with:
+ `"type": "fireflies"` - Fixed string used for identifying file
+ `"speed"` - Set to a float to control the speed the firefly swarm moves around the world at (default 1.0)
+ `"points"` - Required, defines the control points for the bezier curve the firefly swarm move along, should be a list of float `[x, y, z]` lists
(`[[1,2,3], [4,5,6], ...]`). Make sure to use a valid number of control points. In-game, press F3 to enter debug mode and see the points. Numbered
points are the intersections between sub-curves. If green instead of red, the intersection is C(1) continuous (all intersections are C(0) by definition).
Note that the very start and end points won't show as green even if you position them to make a closed loop. That's on the user to verify. `fireflies.py`
can be used to generate C(1) config files.

All `*.json` files in `data/blocks/` are read in to create blocks. You can tweak existing ones or add entirely new ones (accessible via the `setblock`
command). The game expects at least `torch`, `leaves`, `log`, `amethyst`, `mushroom`, and `tall_grass` to exist for it to work properly. `air` is a
reserved name. Each block file has the following layout:

```json
{
    "name": string,
    "collision": bool,
    "model": {
        ...
    }
}
```

+ `"name"` - Required, specifies the name of the block, should be `[a-z_]`
+ `"collision"` - If true, the block has a full cube AABB; otherwise, it can be moved through (defaults to true)
+ `"model"` - Specifies how to render the block, must be one of types described below

```json
{
    "type": "cube",
    "pos": [0.0, 0.0, 0.0],
    "size": [1.0, 1.0, 1.0],
    "textures": []
}
```

+ Creates a basic cube, `"pos"` and `"size"` are optional. `"textures"` should hold either 2 string textures (diffuse and specular) to apply to all
faces, or 12 textures (diffuse then specular per face).

```json
{
    "type": "cross",
    "pos": [0.0, 0.0, 0.0],
    "size": 1.0,
    "textures": []
}
```

+ Creates a basic cross model, `"pos"` and `"size"` are optional. `"textures"` should  2 string textures (diffuse and specular).

```json
{
    "type": "group",
    "pos": [0.0, 0.0, 0.0],
    "rotation": [0.0, 0.0, 0.0],
    "scale": [1.0, 1.0, 1.0],
    "hidden": false,
    "children": []
}
```

+ Creates a group of the nested drawables in `"children"` with some optional transforms.

```json
{
    "type": "oscillate",
    "value": 1.0,
    "child": ...
}
```

+ Causes the model in `"child"` to oscillate like leaves or grass in-game, magnitude is controlled by `"value"`

```json
{
    "type": "tint",
    "color": [r, g, b],
    "child": ...
}
```

+ Tints all diffuse textures in `"child"` (up to another `"type": "tint"` drawable) by the given RGB color (specify in 0.0-1.0 decimals).

```json
{
    "type": "ignore_light",
    "child": ...
}
```

+ Disables shading when rendering `"child"`, good to make fake light sources

```json
{
    "type": "animtex",
    "frames": 1,
    "time": 1.0,
    "child": ...
}
```

+ Treats all diffuse textures in `"child"` as frame strips with `"frames"` number of frames stacked vertically. Each frame lasts for `"time"` seconds
and is lerped with the next as time goes on.

```json
{
    "type": "cullface",
    "face": "back",
    "child": ...
}
```

+ Culls the given `"face"`, should be either `"back"` (default if `"face"` is missing) or `"front"`.

=========================================================================

# Contributions

+ Samvat Dangol - Mac compatibility and bug fixes
+ Thuan Nguyen - Windows compatibility, bug fixes, and leg swing improvements
+ Trin Wasinger - Everything else

=========================================================================

# Class Stuff

This was based on Trin's A4 and midterm project.

This assignment took an insane amount of time. I have no idea exactly how much but ***MANY*** full days were spent on it.

Which lab!? (5/10)

This was very a fun assignment, and I think it looks awesome (10/10)!

=========================================================================

# Easer Eggs

  (Scroll Down for Hints)
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v

+ Up in the sky, look! (x2)
+ We do what we must because we can
+ Star based pseudoscience
+ Gradius
+ Other minor stuff hidden in textures and files (probably can't be found in game)

  (Scroll Down for Spoilers)
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v
              v

+ Up in the sky, look! (x2) - Clouds contain "TRIN" and a heart (Note that since the clouds move, these may not always be visible)
+ We do what we must because we can - There's a companion cube under the map near (64,*,64)
+ Star based pseudoscience - During the night, my sun sign, ♎ (Libra), is visible directly overhead (Look for the noticeably larger stars)
+ Gradius - Use the Konami Code for god mode
+ Other minor stuff hidden in textures and files (probably can't be found in game) - One of many examples, sprites.png has a bunch of unused
  characters, particles, and emojis
