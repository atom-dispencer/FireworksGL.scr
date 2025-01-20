﻿# 🎆 FireworksGL.scr 🎆

![gif](fireworksgl.gif)

*A reimplementation of my [FireworksGL.scr Screensaver](https://github.com/atom-dispencer/Fireworks.scr)
    in pure C and OpenGL (with [GLFW](https://github.com/glfw/glfw) and [GLAD](https://github.com/Dav1dde/glad) 
    for good measure).
My eternal graditude to [RenderDoc](https://renderdoc.org/) for making OpenGL easily debuggable, and to 
    [LearnOpenGL](https://learnopengl.com) for being an excellent teacher!*

A screensaver for Windows which makes pretty fireworks 😊🎆

This was somewhat inspired by a [Dave's Garage video](https://www.youtube.com/watch?v=-foAV_zU2as)
   about making a Matrix-style screensaver for the PDP-11/83.

## Rendering Pipeline

### Stage 1) Clear the screen black
[1_clear](pipeline_photos/1_clear.jpg)

Cover the land (geometry framebuffer) in darkness...

### Stage 2) Draw particle geometry (circles)
[2_circles](pipeline_photos/2_circles.jpg)

For each particle, draw a circle with its colour at its current location.
All particles share the same geometry, but the number, position and colour of 
    particles change, so the draw operation is instanced and uses an 
    Element Buffer Object to remove the need to resend particle geometry data
    to the GPU.

### Stage 3) Draw particle cores (points)
[3_points](pipeline_photos/3_points.jpg)

For each rocket, a white point is draw at its centre using the `GL_POINTS`
    drawing mode.

### Stage 4) Bloom

Bloom can be safely applied to the whole image (no lighting threshold required!)
    because the only bright things are the fireworks!

#### Stage 4a) Apply gaussian blur
[4a_blur](pipeline_photos/4a_blur.jpg)

In a seperate High Dynamic Range (HDR) framebuffer, a few passes of Gaussian 
    blur are applied to the whole image.
A fragment shader averages each pixel with its vertical, then horizontal,
    neighbours using a 5-wide weighting matrix (applied symmetrically
    around each texel, so effectively 11-wide).

#### Stage 4b) Merge geometry and blur buffers

The blurred texture is added to the geometry texture to create the famous
    "bloom" effect.
Tonemapping is applied at the same time...

#### Stage 4c) Tonemapping
[4b_bloom](pipeline_photos/4b_bloom.jpg)

The bloom effect's HDR framebuffer can have colour values outside the normal
    0-1 range, so it needs to be converted back to standard RGB or we'll get
    hotspots in the final image.
To do this, we apply a fixed exposure value, correct the gamma the standard
    2.2, and apply a Reinhard tonemapping function to bring our HDR image
    back to the Low Dynamic Range.

### Stage 5) Render to screen
[5_srgb](pipeline_photos/5_srgb.jpg)

🎆 *Ta-da!* 🎆 

## Usage

1) Grab `FireworksGL.scr` from the Releases page (or build it yourself).
2) Place `FireworksGL.scr` in your `System32` (e.g. `C:/Windows/System32/`).
3) Select `FireworksGL` in the Control Panel (Just search `screensaver` 
   or `change screen saver` or something like that in your search bar or 
   start menu).

I suggest turning the wait down as low as possible so you get to see the
   fireworks most often!

The screensaver automatically closes when you click any button, but *not*
   if you just move the mouse because I personally find that annoying.

## Options

**/s** - Run the screensaver full screen.

**/p** - Run the screensaver in preview/debug mode (windowed.)

*Not yet supported (but you don't need them anyway):*

**/?** - Show a help dialogue with these options.

**/c** - Show a configuration dialogue. No options currently supported.
