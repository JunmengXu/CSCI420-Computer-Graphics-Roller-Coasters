AUTHOR
------
CSCI 420 Computer Graphics
Assignment 2: Roller Coasters
<Junmeng Xu>
<USC ID 8912001452>


CONTENTS OF THIS FILE
---------------------
 * Introduction
 * Environment
 * Basic functions
 * Program's features
 * Extra credit
 * Operation


INTRODUCTION
------------
In this project, I implemented Level 1 (spline), Level 2 (ground), Level 3 (sky), Level 4 (the ride), Level 5 (rail cross-section), and also implemented 6 tasks for extra credit and 2 other creative things


ENVIRONMENT
-----------
This program is developed in Mac OS X.
Please make sure there are below files:
* Pic.h : I reused the "pic" library in homework 1. As a reminder, the "pic" directory has to be located one level above the assignment 2 directory
* images/ : all images I used in the program
* track.txt : set junmengRide.sp as the input splines, I created the tracks that mimic real world roller coasters
* splines/junmengRide.sp : the tracks that mimic real world roller coasters
* Makefile : make assign2.cpp
* Extra_derive_equation.pdf : derive the steps that lead to the physically realistic equation of updating the u
* screenshots/ : empty folder for storing screenshots


BASIC FUNCTIONS
---------------
* Complete all levels.
* Properly render Catmull-Rom splines to represent my track.
* Render a texture-mapped ground and sky.
* Render a rail cross-section.
* Move the camera at a reasonable speed in a continuous path and orientation along the coaster. I made the "up" vector of my camera follow some continuous path of normals to the spline based on the method described in [Supplementary:Camera.html].
* Render the coaster in an interesting manner (good visibility, realism).
* Run at interactive frame rates (>15fps at 640x480)
* Be reasonably commented and written in an understandable manner
* Be submitted along with my animation video
* Be submitted along with a readme file documenting my program's features and describing the approaches


PROGRAM'S FEATURES
------------------
* Because of issues of keyboard mapping on MAC for CTRL or ALT keys for g_ControlState switch, the program used glutKeyboardFunc and use other keys (‘z’, ‘x’, ‘c’) to rebind the control of translation/scaling/rotation.
* Keys 'f' is used to control whether the camera follow the roller coaster. Only when camera not follows the roller coaster, you can make translation/scaling/rotation.
* Keys 'd' is to switch the background textures style, normal one is sunny day with stone ground, another is cosmic cloud.
* Used Pic.h to load images
* Used Sloan's method described in [Supplementary:Camera.html] to make the "up" vector of your camera follow some continuous path of normals to the spline, and set eyes, focus and up vectors in LookAt method.
* Keys 's' is used to automatically store 500. Saved all screenshots through 'sprintf(myFilenm, "screenshots/anim.%03d.jpg", i)' to name filenames.


EXTRA CREDIT
------------
* Render a T-shaped rail cross-section. 
  Use light and dark red to make the T-shaped rail cross-section.
* Render double rail (like in real railroad tracks).
* Draw additional scene elements: texture-mapped wooden crossbars, a support structure that looks realistic
* Create tracks that mimic real world roller coasters such as [Magic Mountain] (close to Los Angeles; check out the Green Lantern in the [video]), or [Gardaland] in Italy.
  Created a track with [junmengRide.sp]
* Modify the velocity with which my camera moves to make it physically realistic in terms of gravity.
* Derive the steps that lead to the physically realistic equation of updating the u (i.e. u_new = u_old + (dt)(sqrt(2gh)/mag(dp/du)), see [Supplementary:Camera_Volecity.pdf]).
  Please see [Extra_derive_equation.pdf]
* Use Pic.h to load images
* Camera can follow the roller coaster, or it can move free. Use 'f' to switch
* Use two style textures to render skybox, one is normal, another is cosmic cloud style


OPERATION
------------
* Mouse
  * LeftMouseButton: drag to rotate/translate/scale x and y dimensions
  * MiddleMouseButton: drag to rotate/translate/scale z dimensions
  * RightMouseButton: open menu and you can choose to quit the running program
* Keyboard
  * Z: switch the controlState to TRANSLATE, available when camera NOT FOLLOW the roller coaster
  * X: switch the controlState to SCALE, available when camera NOT FOLLOW the roller coaster
  * C: switch the controlState to ROTATE, available when camera NOT FOLLOW the roller coaster
  * D: switch the background textures style between NORMAL and COSMIC
  * F: switch the mode of camera between FOLLOW and NOT FOLLOW the roller coaster
  * S: begin to save 500 screenshots automatically
