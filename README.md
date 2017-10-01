# SAMP-MapDrawer2D
Draws a 2D Map of the World using ColAndreas

Load as Filterscript to begin drawing a BMP File.

You can configure resolution, World Bounds, Colors and more in the script.

To draw really big Maps, it's recommended to use a Memory Plugin for buffering the Bitmap.
See functions *Bitmap_Set* and *Bitmap_Get*.

If your Script does not already use ColAndreas for your maps, set the *GRAB_OBJECTS* define to *true* to temporarily create Streamer Objects in the specified virtual world as Collision Objects.

# OC.inc

This Include contains an Array for all Model Categories (including SAMP Objects).
The FS uses it to draw colors based on them.
