# landmark-annotator-desktop

A landmark annotator desktop application written in C++, built on top of openCV APIs and Azure Kinect SDK for a radiation-free spine deformity prediction tool.

# 1. Link OpenCV to Visual Studio 2019
1. Project -> DesktopApp Property  
Whether the `Configuration` is `Release` or `Debug` has to match with your installation. For instance, I only installed OpenCV `Release`. Therefore, I must always use `Release`.
2. VC++ Directories:  
`Include Directories` -> edit -> `D:\opencv-4.5.2\build\install\include`  
`Library Directories` -> edit -> `D:\opencv-4.5.2\build\install\x64\vc16\lib`
3. Linker -> Input  
`Additional Dependencies` -> edit -> `D:\opencv-4.5.2\build\install\x64\vc16\lib\*.lib`

- There is only 1 lib if `BUILD_opencv_world` was used when compiling from source.

# 2. Link Qt5 to Visual Studio 2019
1. Extensions -> Manage Extensions  
Install `Qt Visual Studio Tools`.
2. Extensions -> Qt VS Tools -> Qt Versions  
Add an entry and set as default:  
Version: `5.12.11`  
Path: `D:\Qt\Qt5.12.11\5.12.11\msvc2017_64\bin\qmake.exe`
3. Project -> DesktopApp Property -> Qt Project Settings  
Qt Installation = `5.12.11`, which was set in step 2.  
Whether the `Configuration` is `Release` or `Debug` has to match with your installation. For instance, I only installed OpenCV `Release`. Therefore, I must always use `Release`.

# 3. Link Qt Network modules to Visual Studio 2019
1. Project -> DesktopApp Property -> C/C++ -> General: 
`Additional Include Directories` -> edit -> `C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\include\QtNetwork`.
2. Set for both `Debug` and `Release`:
Under Linker -> Input: prepend `C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\lib\Qt5Networkd.lib` for Debug mode and `C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\lib\Qt5Network.lib` for Release mode.

## Step 3 old method
Copy the `C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\bin\Qt5Networkd.dll` and the `C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\bin\Qt5Network.dll` to `x64/Debug` and `x64/Release` respectively directly under the project root directory.

## Step 3 new method
Run the below command to update the `x64\Debug` directory such as (automatically) adding dll files there.
```
C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\bin\windeployqt.exe C:\Users\Edward\Desktop\Michael_Fong\DesktopApp\x64\Debug\DesktopApp.exe
```

Similarly, for `x64\Release` directory.
```
C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\bin\windeployqt.exe C:\Users\Edward\Desktop\Michael_Fong\DesktopApp\x64\Release\DesktopApp.exe
```

# 4. Manually add SSL libraries so that HTTPS requests can be sent
Copy `libcrypto-1_1-x64.dll`, `libeay32.dll`, `libssl-1_1-x64.dll` and `ssleay32.dll` to `C:\Users\Edward\Desktop\Michael_Fong\DesktopApp\x64\Debug` and `C:\Users\Edward\Desktop\Michael_Fong\DesktopApp\x64\Release`.

# 5. Import Intel RealSense SDK 2.0 (C++) to Visual Studio 2019 (Windows)
1. Download SDK for Windows from https://github.com/IntelRealSense/librealsense/releases/tag/v2.51.1

2. Find the installed SDK at `C:\Program Files (x86)\Intel RealSense SDK 2.0\`

3. Create folder `C:\Users\Edward\Desktop\Michael_Fong\DesktopApp\packages\Intel RealSense SDK 2.0\`

4. Copy `C:\Program Files (x86)\Intel RealSense SDK 2.0\include\` to `C:\Users\Edward\Desktop\Michael_Fong\DesktopApp\packages\Intel RealSense SDK 2.0\`

5. Copy `C:\Program Files (x86)\Intel RealSense SDK 2.0\lib\` to `C:\Users\Edward\Desktop\Michael_Fong\DesktopApp\packages\Intel RealSense SDK 2.0\`

6. Copy `C:\Program Files (x86)\Intel RealSense SDK 2.0\bin\x64\realsense2.dll` to both `C:\Users\Edward\Desktop\Michael_Fong\DesktopApp\x64\Debug\` and `C:\Users\Edward\Desktop\Michael_Fong\DesktopApp\x64\Release\`

7. Project -> DesktopApp Property -> C/C++ -> General:

* Remember to set for both `Debug` and `Release`.

VC++ Directories:  
- `Include Directories` -> edit -> add `$(SolutionDir)packages\Intel RealSense SDK 2.0\include`  
- `Library Directories` -> edit -> add `$(SolutionDir)packages\Intel RealSense SDK 2.0\lib`
- Linker -> Input
`Additional Dependencies` -> edit -> add `$(SolutionDir)packages\Intel RealSense SDK 2.0\lib\x64\realsense2.lib`

# Visual Studio Creating Qt Translation Files
- https://doc-snapshots.qt.io/vstools-dev/qtvstools-translation-files.html

## Create translation file
1. Select Project > Add New Item > Installed > Visual C++ > Qt > Qt Translation File.
2. In Select a Language, you can choose a language from the list of supported languages. You can use Search to filter for a specific language.
3. In the Save as field, enter a filename for the translation file.
4. Select Finish to create the file and have it listed in Translation Files in Visual Studio's Solution Explorer.
5. Right-click a translation file to open a context menu with options for running `lupdate` and `lrelease`. `lupdate` needs to be run to initialize the translation file (e.g. automatically find the contexts and the strings that will be translated).


## Install translation file
In order to find the generate and find the .qm file (for each translation file):
1. Select either Debug or Release in Visual Studio.
2. Right-click each translation file (.ts file) and select lrelease. Now the .qm file will be generated in either x64/Debug or x64/Release.

