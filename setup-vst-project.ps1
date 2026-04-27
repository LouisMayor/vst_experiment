[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

$scriptDir = $PSScriptRoot
$generatorScript = Join-Path $scriptDir 'src\thirdparty\vst3projectgenerator\script\GenerateVST3Plugin.cmake'
$outputRoot = Join-Path $scriptDir 'src'

function Require-Command {
    param([string]$Name)
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "'$Name' is required but was not found in PATH."
    }
}

function ConvertTo-Slug {
    param([string]$Value)
    $lower = $Value.ToLowerInvariant()
    $replaced = [System.Text.RegularExpressions.Regex]::Replace($lower, '[^a-z0-9]+', '-')
    return $replaced.Trim('-')
}

function ConvertTo-Pascal {
    param([string]$Value)
    $tokens = [System.Text.RegularExpressions.Regex]::Split($Value, '[^a-zA-Z0-9]+') |
        Where-Object { $_ -ne '' }
    $sb = [System.Text.StringBuilder]::new()
    foreach ($token in $tokens) {
        $first = $token.Substring(0, 1).ToUpperInvariant()
        $rest = if ($token.Length -gt 1) { $token.Substring(1).ToLowerInvariant() } else { '' }
        [void]$sb.Append($first)
        [void]$sb.Append($rest)
    }
    return $sb.ToString()
}

Require-Command 'cmake'

if (-not (Test-Path $generatorScript)) {
    throw "Generator script was not found at: $generatorScript"
}

Write-Host 'VST Project Setup'
Write-Host '-----------------'

$vstName = ''
while (-not $vstName) {
    $nameInput = Read-Host 'VST name'
    $vstName = $nameInput.Trim()
    if (-not $vstName) {
        Write-Host 'Please enter a non-empty VST name.'
    }
}

$pluginCategory = ''
while (-not $pluginCategory) {
    Write-Host 'Select VST type:'
    Write-Host '  1) Instrument'
    Write-Host '  2) Fx'
    $choice = (Read-Host 'Choice [1-2]').Trim()

    switch -Regex ($choice) {
        '^(1|Instrument|instrument)$' { $pluginCategory = 'Instrument' }
        '^(2|Fx|fx|FX)$'               { $pluginCategory = 'Fx' }
        default { Write-Host 'Invalid choice. Enter 1 for Instrument or 2 for Fx.' }
    }
}

$nameSlug = ConvertTo-Slug $vstName
if (-not $nameSlug) {
    throw 'VST name must include at least one alphanumeric character.'
}

$pluginDirName = "$nameSlug-plugin"
$cmakeProjectName = "$nameSlug-vst3"
$pluginClassName = ConvertTo-Pascal $vstName

if (-not $pluginClassName) { $pluginClassName = 'GeneratedPlugin' }
if ($pluginClassName -match '^[0-9]') { $pluginClassName = "Plugin$pluginClassName" }

$vendorNamespace = "${pluginClassName}Vendor"
$identifierSlug = $nameSlug -replace '-', '_'
$pluginIdentifier = "com.example.vst3.$identifierSlug"
$pluginOutputDir = Join-Path $outputRoot $pluginDirName

if (Test-Path $pluginOutputDir) {
    throw "Target directory already exists: $pluginOutputDir"
}

Write-Host ''
Write-Host 'Generating project...'

& cmake `
    "-DSMTG_PLUGIN_NAME_CLI=$pluginDirName" `
    "-DSMTG_PLUGIN_CATEGORY_CLI=$pluginCategory" `
    "-DSMTG_CMAKE_PROJECT_NAME_CLI=$cmakeProjectName" `
    "-DSMTG_PLUGIN_BUNDLE_NAME_CLI=$cmakeProjectName" `
    "-DSMTG_PLUGIN_IDENTIFIER_CLI=$pluginIdentifier" `
    "-DSMTG_VENDOR_NAMESPACE_CLI=$vendorNamespace" `
    "-DSMTG_PLUGIN_CLASS_NAME_CLI=$pluginClassName" `
    '-DSMTG_VST3_SDK_SOURCE_DIR_CLI=${CMAKE_CURRENT_LIST_DIR}/../thirdparty/vst3sdk' `
    "-DSMTG_GENERATOR_OUTPUT_DIRECTORY_CLI=$outputRoot" `
    -P $generatorScript

if ($LASTEXITCODE -ne 0) {
    throw "CMake generator exited with code $LASTEXITCODE"
}

if (-not (Test-Path $pluginOutputDir -PathType Container)) {
    throw "Generator completed but output folder is missing: $pluginOutputDir"
}

Write-Host ''
Write-Host 'Done.'
Write-Host "- Project directory: $pluginOutputDir"
Write-Host "- VST category in cids: $pluginCategory"
Write-Host ''
