Use prebuild binaries:
	- download the latest release from https://github.com/Dejauxvue/AnnoCalculatorServer/releases
	- extract the archive to any location you desire

Build it yourself: 
	Requirements:
	- Git-installation (e.g. https://git-scm.com/download/win)
	- visual studio 2017 or higher (https://visualstudio.microsoft.com/de/vs/)

	Build instructions:
	- git clone https://github.com/Dejauxvue/AnnoCalculatorServer.git
	- cd AnnoCalculatorServer
	- git submodule update --init
	- cd cpp/vcpkg/
	- bootstrap-vcpkg.bat
	- vcpkg install tesseract:x64-windows (takes circa 17 minutes)
	- vcpkg install cpprestsdk:x64-windows (takes circa 10 minutes)
	- vcpkg install opencv:x64-windows (takes circa 7 minutes)
		
			
			
	troubleshooting: 
		- vcpkg error "Please install the English language pack. Could not locate a complete toolset."
			-> go to visual studio installer -> visual studio communitiy -> change -> language package
			-> select english -> click change (bottom right)
		- if visual studio ist not installed on C:/Program Files x86 openGL (as a part of openCV) might fail to build
			possible fix: https://github.com/Microsoft/vcpkg/issues/4377 (untested)	
			or reinstall windows kit on C:/Program Files x86 (can be more tricky than you think)
			

How to use:
	- run the Server.exe which should open a command window and might require administrator rights
	- open Anno1800Calculator/index.html
	- run Anno 1800
	- in order to update population numbers in Anno1800Calculator hover over the population icon from your island
	
Limitations and known issues:
	- works only for 1920x1080 resolution
	- only for english language
	- if you're using multiple monitors, Anno 1800 must run in the top left most position
	
