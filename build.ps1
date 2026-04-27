[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [string]$Arg1 = $null,

    [Parameter(Position = 1)]
    [string]$Arg2 = $null
)

$plugin = $null
$config = 'Release'

function Set-ConfigOrPlugin($value) {
    $lower = $value.ToLowerInvariant()
    if ($lower -eq 'debug' -or $lower -eq 'release') {
        $script:config = $value
    } else {
        if ($script:plugin) {
            Write-Error "Unexpected argument: $value. Usage: build.ps1 [plugin-dir-or-target] [debug|release]"
            exit 1
        }
        $script:plugin = $value
    }
}

if ($Arg1) { Set-ConfigOrPlugin $Arg1 }
if ($Arg2) { Set-ConfigOrPlugin $Arg2 }

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
        [string]$PluginTarget,
        [string]$BuildConfig
    )

    $sourceDir = Join-Path $scriptDir "src\$PluginDir"
    $configLower = $BuildConfig.ToLowerInvariant()
    $buildDir = Join-Path $buildRoot "$PluginDir-$configLower"

    Write-Host "Building $PluginDir (target: $PluginTarget, config: $BuildConfig)"

    if (Test-Path $buildDir) { Remove-Item $buildDir -Recurse -Force }

    cmake -S $sourceDir -B $buildDir -G Ninja `
        -DCMAKE_BUILD_TYPE=$BuildConfig `
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON `
        -DSMTG_CXX_STANDARD=23
    if ($LASTEXITCODE -ne 0) { throw "CMake $BuildConfig configure failed for $PluginDir" }

    cmake --build $buildDir --target $PluginTarget
    if ($LASTEXITCODE -ne 0) { throw "$BuildConfig build failed for $PluginDir" }

    $compileCommandsInputs.Add((Join-Path $buildDir 'compile_commands.json'))
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

if ($plugin) {
    $pluginDir = Resolve-PluginDir $plugin
    if (-not $pluginDir) {
        Write-Error "Usage: ./build.ps1 [plugin-dir-or-target] [debug|release]`nExample: ./build.ps1 gain-plugin`nExample: ./build.ps1 gain-plugin debug`nIf no plugin is provided, all plugins in src/*-plugin are built."
        exit 1
    }
    $pluginTarget = Resolve-PluginTarget $plugin
    Build-Plugin -PluginDir $pluginDir -PluginTarget $pluginTarget -BuildConfig $config
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
    Build-Plugin -PluginDir $pluginDir -PluginTarget $pluginTarget -BuildConfig $config
}

Write-CompileCommands
