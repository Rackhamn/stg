# stg

TODO:
-----

`INPUT`
* remappable input to 'game' controller actions 
* keyboard input
* gamepad input
* joystick input
* storable / loadable mappings 

`MENU`
* main menu
* settings menu
* settings->input 
* settings->graphics
* in-game menu
* temporary copy of whatever editable data for entering settings

`GRAPHICS`
* creating and storing all graphics objects (player, fire, grass, snake_body, snake_head)
* basic materials for the graphic objects? (like simple phong shading with a single light source)
* building renderlists of the objects and building instance / groupings for rendering
* monospace texture with ASCII characters for general text rendering (its own render pass)
* framebuffer render targets (display resolution != not window size)
* render framebuffer using correct aspect ratio and always fit to window
* try use the SDL2 .ttf rendering for stuff as well? (or maybe just to build the glyph tables)

`AUDIO`
* audio playback
* FM oscillators and audio generators 
* FX like fadig using distance to player (option) [dep on player and camera orientation -> later]

`DEBUG`
* nicer debug compilation option (and runtime option)
* render models outline and volumes

`GAMEPLAY`
* get the main little gameplay loop in
* player (triangle), snake (circles), spear (also triangle), ground and background 

`GENERAL`
* streamable file IO
* endian independant file handling
* texture loading
* audio loading
* utf8 string file parsing
* fps & pause independant [runtime only] timers and events
* mat4, mat3, vec2, vec3, vec4 math library
* color picker
* model handler (vertex define)

`RUNTIME OPTIONS`
* pause on leave window
* pause on sleep / screen-lock / monitor off / low battery (laptop)

