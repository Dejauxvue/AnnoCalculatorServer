# How to use
- run the Server.exe which should open a command window and might require administrator rights
- Anno1800Calculator from local file (if not done already)
- run Anno 1800
- in order to update population numbers in Anno1800Calculator hover over the population icon from your island or your global population

- if you change the screen resolution of your game, you have to restart Server.exe and maybe perform the steps in the following paragraph

# Limitations and known issues
- the ingame text language must be German or English
- sometimes the term for the population tier is not correctly detected and zero entered into the calculator - open an issue in that case (see below which information you should include)
- only 1920x1080 and 1680x1050 are tested and work out of the box
- in order to use another resolution, follow the follwing steps: 
  1. make a screenshot (ALT + PRINT) of your game in the desired resolution while island population is 			visible
  2. open paint and paste the screenshot (STRG + V)
  3. cut the popluation icon (see image)
  4. save the population icon as "population_symbol_with_bar.bmp" in the folder image_recon/"width"x"height"/
where "width"x"height" is the screen resolution of your game
important: use 24-bit bitmap format when saving the icon
  5. restart Server.exe


![Alt text](instructions_0.png?raw=true "instructions 0")
![Alt text](instructions_1.png?raw=true "instructions 1")

**If you encounter any bug, feel free to contact me and if possible send me the following information**
- your game's screen resolution
- the text from the Server.exe console when the bug occured
- if the bug is reproducable: try ServerD.exe instead of Server.exe and send me the images that are generated in the folder image_recon/

# Use prebuild binaries
- download the latest release from https://github.com/Dejauxvue/AnnoCalculatorServer/releases
- extract the archive to any location you desire

# Build it yourself 
## Requirements
- Git-installation (e.g. https://git-scm.com/download/win)
- visual studio 2017 or higher (https://visualstudio.microsoft.com/de/vs/)
	
## Build instructions
- git clone https://github.com/Dejauxvue/AnnoCalculatorServer.git
- cd AnnoCalculatorServer
- git submodule update --init
- cd cpp/vcpkg/
- bootstrap-vcpkg.bat
- vcpkg install tesseract:x64-windows (takes circa 17 minutes)
- vcpkg install cpprestsdk:x64-windows (takes circa 10 minutes)
- vcpkg install opencv:x64-windows (takes circa 7 minutes)
- vcpkg integrate project
- <build cpp/visual studio/CalculatorServer/CalculatorServer.sln>
			
## Troubleshooting 
- vcpkg error "Please install the English language pack. Could not locate a complete toolset."
-> go to visual studio installer -> visual studio communitiy -> change -> language package
-> select english -> click change (bottom right)
- if visual studio ist not installed on C:/Program Files x86 openGL (as a part of openCV) might fail to build
possible fix: https://github.com/Microsoft/vcpkg/issues/4377 (untested)	
or reinstall windows kit on C:/Program Files x86 (can be more tricky than you think)

			
	
