# Usage

## Statistics Extractor
[![Tutorial](https://i9.ytimg.com/vi/fxzYSh-92Ng/mq2.jpg?sqp=COjngvQF&rs=AOn4CLAMwEkElAJLIJ3LHL1foNFahOau2Q)](https://www.youtube.com/watch?v=fxzYSh-92Ng)

- [one-time setup] download, install (and reboot your computer afterwards): [vc_redist](https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads)
- run the Server.exe from the [zip archive](https://github.com/NiHoel/Anno1800UXEnhancer/releases/latest/) which should open a command window and might require administrator rights
- run [Anno1800Calculator](https://github.com/NiHoel/Anno1800Calculator/releases/latest/) from local file (if not done already)
- run Anno 1800
- open the statistic menu (population to update number of houses, finance to update number of factories, production to update productivity)
- alternatively, one can update the population from the overlay of the HUD (not recommended)
- [Tutorial](https://www.youtube.com/watch?v=fxzYSh-92Ng)


## Rerollbot
[![Tutorial](https://i9.ytimg.com/vi/yOkjKXnUFAw/mq2.jpg?sqp=CJvggvQF&rs=AOn4CLDpiTtUMyNLkD_tm4A_70CzAqrWYA)](https://youtu.be/yOkjKXnUFAw)

- [one-time setup] download, install (and reboot your computer afterwards): [vc_redist](https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads)
- Edit RerollbotConfig.json according to your needs, see below. To find out the GUIDs of the items, download the [AssetViewer](https://github.com/Miraak7000/AssetViewer)
- Run Rerollbot.exe
- Open the trade menu of a trader that sells one of the items you want. The bot will start buying / rerolling automatically
- Hit 'Esc' to pause the bot and close the window
- [Tutorial](https://youtu.be/yOkjKXnUFAw)

## Editing RerollbotConfig.json
Make sure that language matches your in-game text language
Possible values are:

"brazilian",
"chinese",
"english",
"french",
"german",
"italian",
"japanese",
"korean",
"polish",
"portuguese",
"russian",
"spanish",
"taiwanese"

Below the settings is the list of items. "\[" and "]" mark the beginning and end of this list.
Each item is encapsulated in curly braces ("{", "}") and has the following attributes:

* name: The item name to make the file more readable, the bot ignores this attributes
* guid: The unique idenfigier of this item. See the [AssetViewer](https://github.com/Miraak7000/AssetViewer/releases) to find the guid for an item (Hint: In the startup view in the right column: Right of the name of the item is its guid)
* count: how many times you want to buy this item; this value will be updated by the bot when it buys this item; if you leave out this property, the bot will buy this item whenever it can

To add an item, you can add the following code (replace <GUID> by the number):

{
  "guid": <GUID>
},


if you want to buy it only once:

{
  "guid": <GUID>,
  "count": 1
},

The name will be automatically added when you rung the bot. If you set count to 0, the bot won't buy this item but it will stay in the list, in case you need it later on.
Keep in mind that it's a list of items and each item is a list of properties, so don't forget the comma in between list elements!

# Troubleshooting
In case **vcruntime140_1.dll** is missing, download the vc_redist from [https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads](https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads)

In case no values are updated, make sure:
- the ingame text language and the language of the calculator/bot config are identical
- the values to be read are not covered by something else (overlay, external program, etc.)
- (Statistics Extractor) the correct island is selected in the statistics screen
- (Statistics Extractor) the island has a long (> 8 letters) name composed from standard letters (A-Za-z) and with other islands as few characters in common as possible
  - NEGATIVE example: "Múa-1" and "Múa-2": Too short, non standard letter ú and both names only differ in one letter
- (Statistics Extractor) in the center of the statistics menu the selected entry is fully visible
- the scaling in windows is set to 100%
- (Rerollbot) Make sure that RerollbotConfig.json is properly formatted 


**If you encounter any bug, feel free to contact me and if possible send me the following information**
- your game's screen resolution
- the text from the Server.exe (or Rerollbot.exe) console when the bug occured
- if the bug is reproducable: try ServerD.exe instead of Server.exe and send me the images that are generated in the folder image_recon/



# Use prebuild binaries
- download the latest release from https://github.com/NiHoel/Anno1800UXEnhancer/releases/latest/
- extract the archive to any location you desire

# Build it yourself 
## Requirements
- Git-installation (e.g. https://git-scm.com/download/win)
- visual studio 2017 or higher (https://visualstudio.microsoft.com/de/vs/)
	
## Build instructions
- git clone https://github.com/NiHoel/Anno1800UXEnhancer.git
- cd AnnoCalculatorServer
- git submodule update --init
- cd cpp/vcpkg/
- bootstrap-vcpkg.bat
- vcpkg install boost-property-tree:x64-windows (takes circa 2 minutes)
- vcpkg install tesseract:x64-windows (takes circa 17 minutes)
- vcpkg install cpprestsdk[core]:x64-windows (takes circa 10 minutes)
- vcpkg install opencv4[png]:x64-windows opencv4[jpeg]:x64-windows (takes circa 7 minutes)
- vcpkg integrate project
- <build cpp/visual studio/CalculatorServer/CalculatorServer.sln>
			
## Troubleshooting 
- copy, move, rename errors during installation: make sure that vcpkg resides on a short path (e.g. accass the folder via a drive letter)
- vcpkg error "Please install the English language pack. Could not locate a complete toolset."
-> go to visual studio installer -> visual studio communitiy -> change -> language package
-> select english -> click change (bottom right)
- if visual studio ist not installed on C:/Program Files x86 openGL (as a part of openCV) might fail to build
possible fix: https://github.com/Microsoft/vcpkg/issues/4377 (untested)	
or reinstall windows kit on C:/Program Files x86 (can be more tricky than you think)

- To update the ui_texts.json place the contents from `Anno 1800/maindata/data2.rda//data/config/gui/` in `cpp/visual studio/CalculatorServer/x64/Release/texts` and delete ui_texts.json. Running the server in release will recreate ui_texts.json from the source files.		
	
