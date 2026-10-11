Android port notes — PhysicsBoard
================================

Overview
--------
This document describes how to prepare Visual Studio and the project to build PhysicsBoard as a native Android app using the Android NDK and CMake. The repo currently targets desktop (GLFW + OpenGL core). Porting requires switching the platform backend (recommended: SDL2) and targeting OpenGL ES.

Prerequisites (on the development machine)
- Visual Studio Community 2026 with workloads:
  - "Desktop development with C++"
  - "Mobile development with C++" (includes Android SDK/NDK support)
- Android SDK & NDK (can be installed via Visual Studio Installer or Android Studio)
- Java JDK (for packaging / Gradle, if using Android Studio)
- CMake (already used by project) and Ninja (generator)

High-level steps
----------------
1. Install Visual Studio workloads above and ensure Android SDK/NDK are present.
2. Replace GLFW desktop backend with SDL2 for Android compatibility (SDL2 provides window, input and lifecycle on Android).
3. Use ImGui backends: imgui_impl_sdl.cpp + imgui_impl_opengl3.cpp configured for OpenGL ES ("#version 300 es" when targeting GLES3).
4. Update main loop to use SDL2 initialization on Android. Map touch events to ImGui mouse input and provide an on-screen keyboard via Android IME.
5. Update CMake to add an Android target that uses the NDK toolchain file. Example cmake flags are in scripts/android_build.ps1.
6. Add AndroidManifest.xml and packaging metadata if you plan to build an APK (Visual Studio or Android Studio handles packaging step).

Quick CMake command (PowerShell example)
---------------------------------------
Set the ANDROID_NDK environment variable to your NDK path, then from repo root:

    $env:ANDROID_NDK = 'C:\\path\\to\\android-ndk'
    cmake -G Ninja -B build-android -S . `
      -DCMAKE_TOOLCHAIN_FILE=$env:ANDROID_NDK\\build\\cmake\\android.toolchain.cmake `
      -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24 -DANDROID_STL=c++_static
    cmake --build build-android

Notes
-----
- This repo will still build as a desktop app in Visual Studio unchanged. The Android build is an additional CMake target.
- You will need to add or compile SDL2 for Android; prefer using SDL2's Android project or fetch it in CMake via FetchContent configured for Android.
- ImPlot is header-only; ensure any GL calls are compatible with GLES.

Deployment to device
--------------------
1. Enable Developer Options and USB debugging on the Pixel device.
2. Connect via USB and verify with adb devices.
3. Use Visual Studio deployment (if configured) or adb install the generated APK.

Next steps I can take now
-------------------------
- Add a starter CMake Android target (CMakeLists additions) and a PowerShell script to run the Android cmake invocation.
- Add a minimal SDL2-based main_android.cpp wrapper and ImGui platform initialization notes.

Tell me which of the above you want me to add to the repository now (CMake target + scripts, or Android wrapper + SDL2 integration).
