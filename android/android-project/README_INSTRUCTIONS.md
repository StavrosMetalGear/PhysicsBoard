Instructions after fetching SDL android-project template

1. From PowerShell run (from repo root):
   .\android\fetch_sdl_android_template.ps1
   This will clone SDL and copy its android-project template into android/android-project.

2. Build native libraries for Android using our CMake helper:
   $env:ANDROID_NDK = 'C:\path\to\android-ndk'
   .\scripts\android_build.ps1

3. Copy the generated .so into the SDL project jniLibs folder:
   .\android\copy_so.ps1 -buildDir build-android -abi arm64-v8a

4. Open android/android-project in Android Studio. Sync and run.

Notes
- The SDL project includes an Activity that loads SDL and calls SDL_main. We renamed our main entry in main_android.cpp to SDL_main so SDL's Java activity can call it.
- Android Studio will build the Java wrapper and package the native .so into the APK.

If you want I can also configure the Android Gradle files to invoke the CMake helper automatically. Let me know.