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
