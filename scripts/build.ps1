$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot

$vsDevCmdCandidates = @(
    "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat",
    "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
)

$cmakeBinCandidates = @(
    "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin",
    "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
)

$ninjaBinCandidates = @(
    "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja",
    "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
)

$vsDevCmd = $vsDevCmdCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
$cmakeBin = $cmakeBinCandidates | Where-Object { Test-Path (Join-Path $_ "cmake.exe") } | Select-Object -First 1
$ninjaBin = $ninjaBinCandidates | Where-Object { Test-Path (Join-Path $_ "ninja.exe") } | Select-Object -First 1

if (-not $vsDevCmd) {
    throw "Could not find VsDevCmd.bat. Install Visual Studio with the Desktop development with C++ workload."
}

if (-not $cmakeBin) {
    throw "Could not find cmake.exe. Install CMake or add it to PATH."
}

if (-not $ninjaBin) {
    throw "Could not find ninja.exe. Install Ninja or add it to PATH."
}

Push-Location $repoRoot
try {
    $command = "set `"PATH=$cmakeBin;$ninjaBin;%PATH%`" && call `"$vsDevCmd`" -arch=x64 && cmake --preset ninja-debug && cmake --build --preset ninja-debug"
    cmd.exe /d /c $command

    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}
