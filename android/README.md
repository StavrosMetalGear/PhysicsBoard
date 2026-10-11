This directory contains helper files to build an Android APK using the SDL2 Android project template.

Workflow (high level):
1. Build native libraries (.so) using scripts/android_build.ps1. This will populate build-android/lib/<abi>/libPhysicsBoard.so (path may differ depending on CMake configurations).
2. Copy the built .so into the SDL Android template's app/src/main/jniLibs/<abi>/ directory (arm64-v8a recommended).
3. Open android/android-project directory in Android Studio and build/run on device.

I will add the SDL android-project template files next (AndroidManifest, Gradle files and minimal Java Activity that uses SDL Activity).