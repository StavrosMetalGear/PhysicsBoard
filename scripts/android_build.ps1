# PowerShell helper to configure and build Android target for PhysicsBoard
# Usage: set $env:ANDROID_NDK to your NDK path, then run this script from repo root

param(
    [string]$ndk = $env:ANDROID_NDK,
    [string]$buildDir = "build-android",
    [string]$abi = "arm64-v8a",
    [string]$platform = "android-24"
)

if (-not $ndk) {
    Write-Error "ANDROID_NDK not set. Provide -ndk or set the ANDROID_NDK environment variable."
    exit 1
}

$toolchain = Join-Path $ndk "build/cmake/android.toolchain.cmake"
if (-not (Test-Path $toolchain)) {
    Write-Error "Toolchain file not found at $toolchain"
    exit 1
}

cmake -G Ninja -B $buildDir -S . `
  -DCMAKE_TOOLCHAIN_FILE=$toolchain `
  -DANDROID_ABI=$abi `
  -DANDROID_PLATFORM=$platform `
  -DANDROID_STL=c++_static

cmake --build $buildDir
