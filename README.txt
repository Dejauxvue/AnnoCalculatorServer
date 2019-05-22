Requirements:
- Git-installation (e.g. https://git-scm.com/download/win)
- visual stdudio 2017 ()
Build instructions:
- git clone https://github.com/Dejauxvue/AnnoCalculatorServer.git
- git submodule update --init
- cd /AnnoCalculatorServer/cpp/vcpkg/
- ./bootstrap-vcpkg.bat
- vcpkg install tesseract:x64-windows
- vcpkg install cpprestsdk:x64-windows
- vcpkg install opencv:x64-windows
