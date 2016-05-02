# DIP Homework Assignment #4
# May 3, 2016
# Name: 蔡維新 Thomas Tsai
# ID #: D04922009
# email: d04922009@ntu.edu.tw, thomas@life100.cc
# Name: 朱煚
# ID #: T04902122
# email: t04902122@ntu.edu.tw
# Name: 戴晏寧
# ID #: B01901013
# email: b01901013@ntu.edu.tw
# Prepare a **POWERED USB3.0 port or POWERED USB3.0 hub** for Intel RealSense #

# README.md markdown #
<https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet>

# How to install opencv 3.1 in visual studio 2015: #
<https://www.youtube.com/watch?v=l4372qtZ4dc>

##setup environment variables in win10:##
1. OPENCV_BUILD=d:\opencv-3.1.0\build\x64\vc14\    
2. OPENCV_DIR=d:\opencv-3.1.0\  

##setup include path in vs2015##
VS2015 include: $(OPENCV_DIR)\build\include  
VS2015 lib path :$(OPENCV_BUILD)\lib\  
input lib: opencv_world310d.lib  

#intel realsense #
env variables : $(RSSDK_DIR)  
vs2015 c/c++  code generation: /MTD  
vs2015 include : $(RSSDK_DIR)\include\;$(RSSDK_DIR)\sample\common\include\  
vs2015 lib path : $(RSSDK_DIR)\lib\x64;$(RSSDK_DIR)\sample\common\lib\x64\v140\  
input lib : libpxcmd_d.lib libpxc_d.lib libpxcutils_d.lib libpxcutilsmd_d.lib

# How to run?#
0. **setup intel realsense cameras, its sdk, and opencv 3.1.0.**
1. double click "realsense\realsense\realsense.sln" to launch the realsense solution in vs2015.
2. set up active project to "realsense_test"
3. rebuild
4. run
5. left click mouse button to setup upper left corner of ROI.
6. right click mouse button to setup lower right corner of ROI.
7. double click left mouse button ro middle mouse button to start the hw4.
8. Wait till 300 frames

# ================================================ #
# Appendix: source code lists #
# ================================================ #
1. hw4.cpp : the program to solve homework#4
2. gui.cpp, gui.h : gui related codes
3. camera_viewer.cpp:main program to capture intel realsense cameras.
4.realsense2cvmat.cpp, realsense2cvmat.h: intel realsense camera image converter to opencv matrix.
# ================================================ #
# some useful command lines #
-listio
-nframes: total frames to capture
-sdname
-csize : %dx%dx%d => color image :width x height x fps
-dsize : %dx%dx%d => depth image :width x height x fps
-isize : %dx%dx%d => irda image :width x height x fps
-lsize : %dx%dx%d => left image :width x height x fps
-rsize : %dx%dx%d => right image :width x height x fps
-file
-record
-noRender
-mirror: mirror the display, so capture is like a morror

detail : $(RSSDK_DIR)\sample\common\src\util_cmdline.cpp
