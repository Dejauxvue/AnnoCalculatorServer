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
[![Tutorial](https://i9.ytimg.com/vi/pPx0_A10G2Q/mqdefault.jpg?time=1586900122438&sqp=CITZ2PQF&rs=AOn4CLCMjXfm6ODoJJ7R-2vagN8pHoHMUA)](https://youtu.be/pPx0_A10G2Q)

- [one-time setup] download, install (and reboot your computer afterwards): [vc_redist](https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads)
- Run UXEnhancer.exe
- Edit the counters to specify how many times you want to buy that item.
- Open the trade menu of a trader that sells one of the items you want. The bot will start buying / rerolling automatically
- Hit 'Esc' to pause the bot and close the window
- [Tutorial](https://youtu.be/yOkjKXnUFAw)
- Thanks to [Veraatversus](https://github.com/Veraatversus) for adopting the AssetViewer

## Editing UXEnhancerConfig.json
Make sure the program is not running.

You can add a maximum reroll cost in the settings (e.g. set it to 1 million):

```
{
    "settings": {
    	"maxRerollCosts": 1000000
        "language": "english"
    },
    ...
}
```

# Troubleshooting
In case **vcruntime140_1.dll** is missing, download the vc_redist from [https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads](https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads)

In case nothing happens, make sure:
- the ingame text language and the language of the calculator/bot config are identical
- the values to be read are not covered by something else (overlay, external program, etc.; basically things you would see when running a screen capturing program)
- (Statistics Extractor) the correct island is selected in the statistics screen
- (Statistics Extractor) the island has a long (> 8 letters) name composed from standard letters (A-Za-z) and with other islands as few characters in common as possible
  - NEGATIVE example: "Múa-1" and "Múa-2": Too short, non standard letter ú and both names only differ in one letter
- (Statistics Extractor) in the center of the statistics menu the selected entry is fully visible
- the scaling in windows is set to 100%
- (UXEnhancer) Some items have identical prices and icons. In such cases wrong items might be bought or several counters decremented although only one item was bought.


**If you encounter any bug, feel free to contact me and if possible send me the following information**
- your game's screen resolution
- the text from the Server.exe (or UXEnhancer.exe) console when the bug occured


# Use prebuild binaries
- download the latest release from https://github.com/NiHoel/Anno1800UXEnhancer/releases/latest/
- extract the archive to any location you desire

# Build it yourself 
## Requirements
- Git-installation (e.g. https://git-scm.com/download/win)
- visual studio 2017 or higher (https://visualstudio.microsoft.com/de/vs/)
	
## Build instructions
- git clone https://github.com/NiHoel/Anno1800UXEnhancer.git
- cd Anno1800UXEnhancer
- SETUP.bat
- vcpkg install boost-property-tree:x64-windows (takes circa 2 minutes)
- vcpkg install tesseract:x64-windows (takes circa 17 minutes)
- vcpkg install cpprestsdk[core]:x64-windows (takes circa 10 minutes)
- vcpkg install opencv4[png]:x64-windows opencv4[jpeg]:x64-windows (takes circa 7 minutes)
- <build cpp/visual studio/UXEnhancer.sln>
			
## Troubleshooting 
- copy, move, rename errors during installation: make sure that vcpkg resides on a short path (e.g. accass the folder via a drive letter)
- vcpkg error "Please install the English language pack. Could not locate a complete toolset."
-> go to visual studio installer -> visual studio communitiy -> change -> language package
-> select english -> click change (bottom right)
- if visual studio ist not installed on C:/Program Files x86 openGL (as a part of openCV) might fail to build
possible fix: https://github.com/Microsoft/vcpkg/issues/4377 (untested)	
or reinstall windows kit on C:/Program Files x86 (can be more tricky than you think)

- To update the ui_texts.json place the contents from `Anno 1800/maindata/data2.rda//data/config/gui/` in `cpp/visual studio/CalculatorServer/x64/Release/texts` and delete ui_texts.json. Running the server in release will recreate ui_texts.json from the source files.		
	
