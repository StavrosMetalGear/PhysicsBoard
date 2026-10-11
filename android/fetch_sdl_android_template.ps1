param(
    [string]$sdlRepo = 'https://github.com/libsdl-org/SDL.git',
    [string]$checkout = 'release-2.28.6',
    [string]$tmpDir = "$env:TEMP\SDL_tmp",
    [string]$dest = "android/android-project"
)

if (Test-Path $dest -PathType Leaf -or Test-Path $dest -PathType Container) {
    Write-Host "Destination $dest already exists. Existing files will be preserved; new files may be overwritten."
}

if (Test-Path $tmpDir) { Remove-Item -Recurse -Force $tmpDir }

Write-Host "Cloning SDL into $tmpDir (this may take a while)..."
git clone --depth 1 --branch $checkout $sdlRepo $tmpDir
if ($LASTEXITCODE -ne 0) { Write-Error "git clone failed"; exit 1 }

$src = Join-Path $tmpDir 'android-project'
if (-not (Test-Path $src)) { Write-Error "SDL android-project not found in cloned repo"; exit 1 }

Write-Host "Copying SDL android-project files to $dest"
if (-not (Test-Path $dest)) { New-Item -ItemType Directory -Path $dest | Out-Null }

# Copy recursively
robocopy $src $dest /MIR /XD .git .github .github* /NFL /NDL /NJH /NJS /nc /ns /np | Out-Null
if ($LASTEXITCODE -ge 8) { Write-Warning "robocopy reported an error code: $LASTEXITCODE" }

Write-Host "SDL android-project copied to $dest. You can now open it in Android Studio."

# Cleanup
Remove-Item -Recurse -Force $tmpDir
Write-Host "Temporary clone removed."