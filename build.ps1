[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [string]$Plugin
)

$ErrorActionPreference = 'Stop'

$scriptDir = $PSScriptRoot
$buildRoot = Join-Path $scriptDir 'build'
$compileCommandsInputs = [System.Collections.Generic.List[string]]::new()
$script:VsEnvLoaded = $false

function Enter-VsEnv {
    if ($script:VsEnvLoaded) { return }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exe not found at $vswhere. Install Visual Studio 2019+ or Build Tools with the 'Desktop development with C++' workload."
    }

    $vsRoot = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsRoot) {
        throw "No Visual Studio installation with the C++ toolchain was found. Install the 'Desktop development with C++' workload."
    }

    $devShell = Join-Path $vsRoot 'Common7\Tools\Launch-VsDevShell.ps1'
    if (-not (Test-Path $devShell)) {
        throw "Launch-VsDevShell.ps1 not found at $devShell"
    }

    Write-Host "Entering VS dev shell: $vsRoot"
    & $devShell -Arch amd64 -HostArch amd64 -SkipAutomaticLocation | Out-Null
    $script:VsEnvLoaded = $true
}

function Resolve-PluginDir {
    param([string]$Request)

    $normalized = $Request -replace '^src[\\/]', ''
    if (Test-Path (Join-Path $scriptDir "src\$normalized\CMakeLists.txt")) {
        return $normalized
    }

    if ($normalized -like '*-vst3') {
        $mapped = $normalized -replace '-vst3$', '-plugin'
        if (Test-Path (Join-Path $scriptDir "src\$mapped\CMakeLists.txt")) {
            return $mapped
        }
    }

    return $null
}

function Resolve-PluginTarget {
    param([string]$Request)

    if ($Request -like '*-vst3') { return $Request }

    $normalized = $Request -replace '^src[\\/]', ''
    $cmakeFile = Join-Path $scriptDir "src\$normalized\CMakeLists.txt"
    if (Test-Path $cmakeFile) {
        $match = Select-String -Path $cmakeFile -Pattern '^\s*project\s*\(\s*([A-Za-z0-9_.+-]+)' | Select-Object -First 1
        if ($match) { return $match.Matches[0].Groups[1].Value }
    }

    if ($normalized -like '*-plugin') {
        return ($normalized -replace '-plugin$', '-vst3')
    }
    return "$normalized-vst3"
}

function Build-Plugin {
    param(
        [string]$PluginDir,
        [string]$PluginTarget
    )

    $sourceDir = Join-Path $scriptDir "src\$PluginDir"
    $debugDir = Join-Path $buildRoot "$PluginDir-dbg"
    $releaseDir = Join-Path $buildRoot "$PluginDir-rel"

    Write-Host "Building $PluginDir (target: $PluginTarget)"

    if (Test-Path $debugDir) { Remove-Item $debugDir -Recurse -Force }
    if (Test-Path $releaseDir) { Remove-Item $releaseDir -Recurse -Force }

    cmake -S $sourceDir -B $debugDir -G Ninja `
        -DCMAKE_BUILD_TYPE=Debug `
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON `
        -DSMTG_CXX_STANDARD=23
    if ($LASTEXITCODE -ne 0) { throw "CMake Debug configure failed for $PluginDir" }

    cmake --build $debugDir --target $PluginTarget
    if ($LASTEXITCODE -ne 0) { throw "Debug build failed for $PluginDir" }

    cmake -S $sourceDir -B $releaseDir -G Ninja `
        -DCMAKE_BUILD_TYPE=Release `
        -DSMTG_CXX_STANDARD=23
    if ($LASTEXITCODE -ne 0) { throw "CMake Release configure failed for $PluginDir" }

    cmake --build $releaseDir --target $PluginTarget
    if ($LASTEXITCODE -ne 0) { throw "Release build failed for $PluginDir" }

    $compileCommandsInputs.Add((Join-Path $debugDir 'compile_commands.json'))
}

function Write-CompileCommands {
    if ($compileCommandsInputs.Count -eq 0) { return }

    $output = Join-Path $scriptDir 'compile_commands.json'
    if (Test-Path $output) { Remove-Item $output -Force }

    if ($compileCommandsInputs.Count -eq 1) {
        Copy-Item $compileCommandsInputs[0] $output
        return
    }

    $merged = [System.Collections.Generic.List[object]]::new()
    $seen = [System.Collections.Generic.HashSet[string]]::new()
    foreach ($path in $compileCommandsInputs) {
        if (-not (Test-Path $path)) { continue }
        $entries = Get-Content $path -Raw | ConvertFrom-Json
        foreach ($entry in $entries) {
            $key = "$($entry.directory)::$($entry.file)"
            if ($seen.Add($key)) { $merged.Add($entry) }
        }
    }
    $merged | ConvertTo-Json -Depth 10 | Set-Content -Path $output -Encoding UTF8
}

Enter-VsEnv

if ($Plugin) {
    $pluginDir = Resolve-PluginDir $Plugin
    if (-not $pluginDir) {
        Write-Error "Usage: ./build.ps1 [plugin-dir-or-target]`nExample: ./build.ps1 gain-plugin`nIf no plugin is provided, all plugins in src/*-plugin are built."
        exit 1
    }
    $pluginTarget = Resolve-PluginTarget $Plugin
    Build-Plugin -PluginDir $pluginDir -PluginTarget $pluginTarget
    Write-CompileCommands
    exit 0
}

$pluginCmakeFiles = Get-ChildItem (Join-Path $scriptDir 'src') -Directory -Filter '*-plugin' |
    ForEach-Object { Join-Path $_.FullName 'CMakeLists.txt' } |
    Where-Object { Test-Path $_ }

if (-not $pluginCmakeFiles) {
    Write-Error "No plugin directories found in src\*-plugin"
    exit 1
}

foreach ($cmakeFile in $pluginCmakeFiles) {
    $pluginDir = Split-Path -Leaf (Split-Path -Parent $cmakeFile)
    $pluginTarget = Resolve-PluginTarget $pluginDir
    Build-Plugin -PluginDir $pluginDir -PluginTarget $pluginTarget
}

Write-CompileCommands
