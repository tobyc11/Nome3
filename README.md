# NOME3
NOME is a CAD tool that allows users to procedurally generate complex shapes and interactively modify them in the GUI. 

We have introduced multiple unique language constructs, such as "group" and "mesh", that allow users to piece together shapes and reuse them easily. We also have implemented advanced shape generators (e.g. torus knot) that may make certain modelling tasks easier. Please view our full language reference here: https://docs.google.com/document/d/15c1VIXpGUxIL3daA3pI-iYW4K5ebd3snveDFWXRpa70/edit?ts=5fcb20ad#

# Compilation Preparation
This section will walk you through the steps for setting up a build environment for Nome 3 on Windows (see below for MacOS). Here is a list of the required dependencies:

- Visual Studio 2019 (https://visualstudio.microsoft.com/)
- CMake
- Qt 5
- OpenMesh
- ANTLR parser generator (https://www.antlr.org/download/antlr-4.8-complete.jar)
- Java 


To make your life easier, there are a few package management software on Windows that I would personally recommend:
- Scoop (We need this for cmake) 
- Vcpkg (We need this for OpenMesh and Qt5)

Please install __Visual Studio__ first, as it is necessary for any development work on Windows. Then you can install __scoop__ and __vcpkg__; Scoop installation instructions are here: https://scoop.sh/, and vcpkg instructions are here: https://github.com/microsoft/vcpkg (just run git clone https://github.com/microsoft/vcpkg and .\vcpkg\bootstrap-vcpkg.bat first. You'll install the needed packages below). It should be straightforward. Go ahead and spend a few minutes playing with each package management tool. Please read this: https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=msvc-160. They will save you tons of time trying to pull together different C++ libraries, potentially in the future. Now you might be wondering why we need two different package managers. So I will explain: __scoop__ deals with applications, such as CMake and Python; __vcpkg__ is a C++ dependency manager from Microsoft that goes together with Visual Studio, and it concerns with libraries that we can `#include` from our own code. 

Now hopefully you are familiar with those package managers. Run the following commands (potentially in different directories depending on where you installed each package manager) to install what we need:

```
scoop install cmake
vcpkg --triplet x64-windows install openmesh 
vcpkg --triplet x64-windows install pugixml instructions to install Qt on Windows)
git clone --recursive https://github.com/randyfan/NOME3
```
~~vcpkg --triplet x64-windows install qt5-base qt5-3d~~ (Update 12/21/2020: the most recent qt5 package, 5.15 as of 12/21, is no longer compatible with NOME3. Please read Qt installation below. 

# Qt Installation for NOME
Unfortunately, the more recent qt5 packages are not compatible with NOME3 (Qt 5.15 is confirmed to not work, and I assume the newer ones, if any, may not work also). Vcpkg essentially forces users to install the most recent version of any package (there is a way to bypass this, but it is a bit of a headache), so we can't use vcpkg to install qt5. Thus, as a temporary solution, let's download Qt directly from the Qt website.

To fix this, please download Qt directly:
1. Go to https://www.qt.io/download
2. Click on "Go open source" , which leads to: https://www.qt.io/download-open-source?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5
3. Scroll down and click "Download the Qt Online Installer"
4. Download the installer and run it 
5. Follow the setup instructions,  which will require you to create a Qt account if I recall correctly. When you to "Select Components" page, please select the following. We choose to install Qt 5.12.5 since it works w/ NOME3 (__this is a very important step__):
![](https://github.com/randyfan/NOME3/blob/master/Docs/Qtsetup.png)
6. Click next and complete installation. Qt should be ~40gb, so make sure you have enough storage space. 
7. Done, move on to compilation 

# Compilation
Open up the CMake GUI, and set it up like this:
![](https://github.com/cyj0912/Nome3/blob/master/Docs/snip1.jpg)

Then click on "Add Entry" to add the following variables so that CMake can find all those libraries:
```
CMAKE_TOOLCHAIN_FILE = [Vcpkg dir]/scripts/buildsystems/vcpkg.cmake
CMAKE_INSTALL_PREFIX =  In my case it is C:/Qt/5.12.5/msvc2017_64/lib/cmake to give you an example. This environment variable CMAKE_PREFIX_PATH is needed so that CMake knows where to look for Qt.
```

Now you can click on "Configure" and "Generate" to generate a Visual Studio solution file and open it with Visual Studio. Then, click Build Solution, and a NOME3.exe should appear in the Build directory.

If you get an error involving ANTLR, first make sure the file path is correct. Then, copy paste this file https://github.com/randyfan/NOME3/blob/master/CMake/Modules/ExternalAntlr4Cpp.cmake into your ExternalAntlr4Cpp.cmake.

If you run into an issue with windeployqt.exe, make sure CMake was able to find it. Try providing the path manually. 

Upon your first run of CMake, it may complain about Foundation does not contain a CMakeLists.txt file. This is because when you clone the repository, git does not by default clone all the submodules, so the Foundation subfolder is empty by default. You can either reclone the repo with git clone --recursive or run git submodule update --init --recursive. If you have further questions, read the top portion of this short tutorial on git submodules - https://www.vogella.com/tutorials/GitSubmodules/article.html.

# Preparation and Compilation for Mac Users 
This is a quick setup for new students working on Nome3 on Mac. If the below instructions do not work, please also look at the mini guide Toby Chen recently wrote: https://github.com/cyj0912/Nome3/wiki (which also contains Fedora, Ubuntu, and macOS homebrew commands), or ask an experienced macOS NOME member. 

Go to your desired repository in your terminal and run the following command to add all the Nome3 files and required submodules.
```
git clone --recursive https://github.com/randyfan/NOME3
```

Nome3 requires the most recent version of its dependencies so run:
```
brew install qt5
brew install open-mesh
brew install pugixml
brew install cmake
```
You'll also need to download: ANTLR parser generator (https://www.antlr.org/download/antlr-4.8-complete.jar). Zachary informed me you can just do brew install antlr, and that should work. Make sure to install Java as well.

Finally, run the following commands:
```
cd NOME3
mkdir build && cd build
cmake .. && make
```
Most likely, CMake is going to complain about `Could NOT find ANTLR (missing: ANTLR_EXECUTABLE)`. If this happens, you just need to manually set `ANTLR_EXECUTABLE=<path to antlr-4.8-complete.jar>`. You can either manually edit the `CMakeCache.txt` file under your build directory, or pass it as a command line argument to CMake. I think the command line argument is just cmake NOME3 -D ANTLR_EXECUTABLE= `path to antlr`.

If you still get an error related to ANTLR, first make sure the file path is correct. Then, copy paste this file https://github.com/randyfan/NOME3/blob/master/CMake/Modules/ExternalAntlr4Cpp.cmake into your ExternalAntlr4Cpp.cmake.


The above commands will create a build folder, and compile and make Nome3. This may take a few minutes so wait until the make has completed. Once it has completed, navigate to `build/Application/Binaries`. In the Binaries folder there should be a Nome3 executable. Run this program and it will open up an untitled.nom preview. From the menu bar at the top of your screen run `File/Open` and select the .nom file you would like to run. You will now be able to see your .nom instance in a 3-D display!

Upon your first run of CMake, it may complain about `Foundation does not contain a CMakeLists.txt file.` This is because when you clone the repository, git does not by default clone all the submodules, so the Foundation subfolder is empty by default. You can either reclone the repo with `git clone --recursive` or run `git submodule update --init --recursive`. If you have further questions, read the top portion of this short tutorial on git submodules - https://www.vogella.com/tutorials/GitSubmodules/article.html.


This an example of running a simple cube .nom instance after following the steps above:
![](https://github.com/cyj0912/Nome3/blob/master/Docs/hellocube.png)

# Running
TBD

# Useful stackoverflow posts
https://stackoverflow.com/questions/65264658/unable-to-find-renderer-plugin-for-opengl
https://stackoverflow.com/questions/53314957/trouble-deploying-qt-exe-for-windows

