# landmark-annotator-desktop

A landmark annotator desktop application written in C++, built on top of openCV APIs and Azure Kinect SDK for a radiation-free spine deformity prediction tool.

# Example on how to link OpenCV4.5.2 and Qt5 to VS2019

## 1. Link OpenCV to Visual Studio 2019
1. Project -> DesktopApp Property  
Whether the `Configuration` is `Release` or `Debug` has to match with your installation. For instance, I only installed OpenCV `Release`. Therefore, I must always use `Release`.
2. VC++ Directories:  
`Include Directories` -> edit -> `D:\opencv-4.5.2\build\install\include`  
`Library Directories` -> edit -> `D:\opencv-4.5.2\build\install\x64\vc16\lib`
3. Linker -> Input  
`Additional Dependencies` -> edit -> `D:\opencv-4.5.2\build\install\x64\vc16\lib\*.lib`

- There is only 1 lib if `BUILD_opencv_world` was used when compiling from source.

## 2. Link Qt5 to Visual Studio 2019
1. Extensions -> Manage Extensions  
Install `Qt Visual Studio Tools`.
2. Extensions -> Qt VS Tools -> Qt Versions  
Add an entry and set as default:  
Version: `5.12.11`  
Path: `D:\Qt\Qt5.12.11\5.12.11\msvc2017_64\bin\qmake.exe`
3. Project -> DesktopApp Property -> Qt Project Settings  
Qt Installation = `5.12.11`, which was set in step 2.  
Whether the `Configuration` is `Release` or `Debug` has to match with your installation. For instance, I only installed OpenCV `Release`. Therefore, I must always use `Release`.