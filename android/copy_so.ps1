# Copy built native .so to SDL android project jniLibs
param(
    [string]$buildDir = "build-android",
    [string]$abi = "arm64-v8a",
    [string]$dest = "android/android-project/app/src/main/jniLibs"
)

$src = Join-Path $buildDir "lib/$abi"
if (-not (Test-Path $src)) {
    Write-Error "Expected built libs in $src. Build them first with scripts/android_build.ps1"
    exit 1
}

$files = Get-ChildItem -Path $src -Filter "*.so"
if ($files.Count -eq 0) {
    Write-Error "No .so files found in $src"
    exit 1
}

$destAbi = Join-Path $dest $abi
if (-not (Test-Path $destAbi)) {
    New-Item -ItemType Directory -Path $destAbi -Force | Out-Null
}

foreach ($f in $files) {
    Copy-Item $f.FullName -Destination $destAbi -Force
    Write-Host "Copied $($f.Name) -> $destAbi"
}

Write-Host "Done. Open android/android-project in Android Studio and build the APK."
