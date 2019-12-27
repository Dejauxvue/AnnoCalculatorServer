[![Tutorial](https://img.youtube.com/vi/fxzYSh-92Ng/0.jpg)](https://www.youtube.com/watch?v=fxzYSh-92Ng)

# How to use
- [one-time setup] download, install (and reboot your computer afterwards): [vc_redist](https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads)
- run the Server.exe from the [zip archive](https://github.com/Dejauxvue/AnnoCalculatorServer/releases/latest/) which should open a command window and might require administrator rights
- run [Anno1800Calculator](https://github.com/NiHoel/Anno1800Calculator/releases/latest/) from local file (if not done already)
- run Anno 1800
- open the statistic menu (population to update number of houses, finance to update number of factories, production to update productivity)
- alternatively, one can update the population from the overlay of the HUD (not recommended)
- [Tutorial](https://www.youtube.com/watch?v=fxzYSh-92Ng)

# Troubleshooting
In case **vcruntime140_1.dll** is missing, download the vc_redist from [https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads](https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads)

In case no values are updated, make sure:
- the ingame text language and the language of the calculator are identical
- the values to be read are not covered by something else (overlay, external program, etc.)
- the correct island is selected in the statistics screen
- the island has a long (> 8 letters) name composed from standard letters (A-Za-z) and with other islands as few characters in common as possible
  - NEGATIVE example: "Múa-1" and "Múa-2": Too short, non standard letter ú and both names only differ in one letter
- in the center of the statistics menu the selected entry is fully visible


# More resultions
- in order to use another resolution (only for the population pop-ups in the HUD), follow the follwing steps: 
  1. make a screenshot (ALT + PRINT) of your game in the desired resolution while island population is 			visible
  2. open paint and paste the screenshot (STRG + V)
  3. cut the popluation icon (see image)
  4. save the population icon as "population_symbol_with_bar.bmp" in the folder image_recon/"width"x"height"/
where "width"x"height" is the screen resolution of your game
important: use 24-bit bitmap format when saving the icon


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
- vcpkg install boost-property-tree:x64-windows (takes circa 2 minutes)
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

- To update the ui_texts.json place the contents from `Anno 1800/maindata/data2.rda//data/config/gui/` in `cpp/visual studio/CalculatorServer/x64/Release/texts` and delete ui_texts.json. Running the server in release will recreate ui_texts.json from the source files.		
	
