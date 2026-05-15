param(
  [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")),
  [string]$Configuration = $env:WINDOWS_CONFIGURATION,
  [string]$Platform = $env:WINDOWS_PLATFORM
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

if ([string]::IsNullOrWhiteSpace($Configuration)) { $Configuration = "Release" }
if ([string]::IsNullOrWhiteSpace($Platform)) { $Platform = "x64" }
$Root = (Resolve-Path -LiteralPath $Root).Path

function Write-Section([string]$Message) {
  Write-Host ""
  Write-Host "==> $Message"
}

function New-Directory([string]$Path) {
  New-Item -ItemType Directory -Force -Path $Path | Out-Null
}

function Invoke-WithRetry([scriptblock]$Action, [int]$Attempts = 3) {
  for ($i = 1; $i -le $Attempts; $i++) {
    try {
      & $Action
      return
    } catch {
      if ($i -eq $Attempts) { throw }
      Write-Warning "Attempt $i failed: $($_.Exception.Message). Retrying..."
      Start-Sleep -Seconds ([Math]::Min(30, 5 * $i))
    }
  }
}

function Expand-DependencyArchive {
  param(
    [Parameter(Mandatory=$true)][string]$Name,
    [Parameter(Mandatory=$true)][string]$Uri,
    [Parameter(Mandatory=$true)][string]$Destination,
    [Parameter(Mandatory=$true)][string]$Marker
  )

  if (Test-Path -LiteralPath (Join-Path $Destination $Marker)) {
    Write-Host "$Name already hydrated ($Marker found)."
    return
  }

  Write-Section "Hydrating $Name"
  New-Directory $Destination
  $runnerTemp = if ([string]::IsNullOrWhiteSpace($env:RUNNER_TEMP)) { [IO.Path]::GetTempPath() } else { $env:RUNNER_TEMP }
  $tempRoot = Join-Path $runnerTemp "kainote-$Name"
  Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
  New-Directory $tempRoot
  $archive = Join-Path $tempRoot "$Name.zip"

  Invoke-WithRetry {
    Invoke-WebRequest -Uri $Uri -OutFile $archive -UseBasicParsing
  }

  Expand-Archive -LiteralPath $archive -DestinationPath $tempRoot -Force
  $roots = Get-ChildItem -LiteralPath $tempRoot -Directory | Where-Object { $_.FullName -ne $Destination }
  if ($roots.Count -lt 1) { throw "No extracted root directory found for $Name" }
  $sourceRoot = $roots[0].FullName
  Copy-Item -Path (Join-Path $sourceRoot "*") -Destination $Destination -Recurse -Force

  if (-not (Test-Path -LiteralPath (Join-Path $Destination $Marker))) {
    throw "$Name hydration did not create expected marker: $Marker"
  }
}

function Get-VCTargetsPath {
  if (-not [string]::IsNullOrWhiteSpace($env:VCTargetsPath) -and (Test-Path -LiteralPath $env:VCTargetsPath)) {
    return (Resolve-Path -LiteralPath $env:VCTargetsPath).Path
  }

  $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
  if (-not (Test-Path -LiteralPath $vswhere)) { throw "vswhere.exe not found and VCTargetsPath is not set" }
  $installPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
  if ([string]::IsNullOrWhiteSpace($installPath)) { throw "Visual Studio with VC tools not found" }
  $candidate = Join-Path $installPath "MSBuild\Microsoft\VC\v170\"
  if (-not (Test-Path -LiteralPath $candidate)) { throw "VCTargetsPath not found: $candidate" }
  return (Resolve-Path -LiteralPath $candidate).Path
}

function Copy-IfPresent([string]$Source, [string]$Destination) {
  if (Test-Path -LiteralPath $Source) {
    if (Test-Path -LiteralPath $Destination) {
      $sourceResolved = (Resolve-Path -LiteralPath $Source).Path
      $destinationResolved = (Resolve-Path -LiteralPath $Destination).Path
      if ([string]::Equals($sourceResolved, $destinationResolved, [StringComparison]::OrdinalIgnoreCase)) {
        return
      }
    }

    New-Directory (Split-Path -Parent $Destination)
    Copy-Item -LiteralPath $Source -Destination $Destination -Force
  }
}

function Ensure-ToolOnPath([string]$ExeName, [string[]]$Candidates) {
  $cmd = Get-Command $ExeName -ErrorAction SilentlyContinue
  if ($cmd) { return $cmd.Source }

  foreach ($candidate in $Candidates) {
    if (Test-Path -LiteralPath $candidate) {
      $dir = Split-Path -Parent $candidate
      $env:PATH = "$dir;$env:PATH"
      if ($env:GITHUB_PATH) { Add-Content -LiteralPath $env:GITHUB_PATH -Value $dir }
      return $candidate
    }
  }

  throw "$ExeName not found. Checked PATH and: $($Candidates -join ', ')"
}

Write-Section "Preparing ignored third-party dependency trees"
$thirdparty = Join-Path $Root "Thirdparty"
Expand-DependencyArchive -Name "boost" -Uri "https://archives.boost.io/release/1.61.0/source/boost_1_61_0.zip" -Destination (Join-Path $thirdparty "boost") -Marker "boost\flyweight.hpp"
Expand-DependencyArchive -Name "icu" -Uri "https://github.com/unicode-org/icu/releases/download/release-60-2/icu4c-60_2-src.zip" -Destination (Join-Path $thirdparty "icu") -Marker "source\common\unicode\utypes.h"
Expand-DependencyArchive -Name "zlib" -Uri "https://github.com/madler/zlib/archive/refs/tags/v1.2.13.zip" -Destination (Join-Path $thirdparty "zlib") -Marker "zlib.h"
$legacyZlib = Join-Path $thirdparty "zlib-1.2.12"
if (-not (Test-Path -LiteralPath (Join-Path $legacyZlib "zlib.h"))) {
  Write-Host "Creating zlib-1.2.12 compatibility tree for stale project include paths."
  New-Directory $legacyZlib
  Copy-Item -Path (Join-Path (Join-Path $thirdparty "zlib") "*") -Destination $legacyZlib -Recurse -Force
}
Expand-DependencyArchive -Name "curl" -Uri "https://github.com/curl/curl/archive/976eb1d50d56dcb1fe55a65ebe095d5012627f7e.zip" -Destination (Join-Path $thirdparty "curl") -Marker "include\curl\curl.h"

Write-Section "Adding compatibility shims for the pinned curl snapshot"
$curlRoot = Join-Path $thirdparty "curl"
$curlCompat = @{
  "lib\strdup.c" = "/* Kainote CI compatibility shim: Curl.vcxproj still lists lib/strdup.c. */`n#include `"curlx/strdup.c`"`n";
  "lib\curl_rtmp.c" = "/* Kainote CI compatibility shim: RTMP support was removed from the pinned curl snapshot. */`n";
  "lib\vquic\curl_osslq.c" = "/* Kainote CI compatibility shim: OpenSSL QUIC backend source was removed from the pinned curl snapshot. */`n";
}

foreach ($entry in $curlCompat.GetEnumerator()) {
  $path = Join-Path $curlRoot $entry.Key
  if (-not (Test-Path -LiteralPath $path)) {
    New-Directory (Split-Path -Parent $path)
    Set-Content -LiteralPath $path -Value $entry.Value -Encoding ASCII
  }
}

Write-Section "Generating transient zlib MSBuild project"
$zlibProjectDir = Join-Path $thirdparty "Build\Zlib"
New-Directory $zlibProjectDir
$zlibProject = @'
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="17.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32"><Configuration>Debug</Configuration><Platform>Win32</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64"><Configuration>Debug</Configuration><Platform>x64</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32"><Configuration>Release</Configuration><Platform>Win32</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64"><Configuration>Release</Configuration><Platform>x64</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Rel_AVX|x64"><Configuration>Rel_AVX</Configuration><Platform>x64</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Rel_AVX2|x64"><Configuration>Rel_AVX2</Configuration><Platform>x64</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Rel_AVX512|Win32"><Configuration>Rel_AVX512</Configuration><Platform>Win32</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Rel_AVX512|x64"><Configuration>Rel_AVX512</Configuration><Platform>x64</Platform></ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <ProjectGuid>{56E35122-0B51-43D4-A721-472F2F956E0F}</ProjectGuid>
    <RootNamespace>Zlib</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration">
    <ConfigurationType>StaticLibrary</ConfigurationType>
    <UseDebugLibraries Condition="'$(Configuration)'=='Debug'">true</UseDebugLibraries>
    <UseDebugLibraries Condition="'$(Configuration)'!='Debug'">false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>MultiByte</CharacterSet>
    <WholeProgramOptimization Condition="'$(Configuration)'!='Debug'">true</WholeProgramOptimization>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <PropertyGroup>
    <OutDir>$(SolutionDir)bin\$(Platform)\$(Configuration)\</OutDir>
    <IntDir>$(SolutionDir)obj\$(Platform)\$(Configuration)\$(ProjectName)\</IntDir>
    <TargetName>zlib</TargetName>
  </PropertyGroup>
  <ItemDefinitionGroup>
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <PreprocessorDefinitions>WIN32;_LIB;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <AdditionalIncludeDirectories>$(SolutionDir)Thirdparty\zlib;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <RuntimeLibrary Condition="'$(Configuration)'=='Debug'">MultiThreadedDebugDLL</RuntimeLibrary>
      <RuntimeLibrary Condition="'$(Configuration)'!='Debug'">MultiThreadedDLL</RuntimeLibrary>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
    </ClCompile>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="..\..\zlib\adler32.c" />
    <ClCompile Include="..\..\zlib\compress.c" />
    <ClCompile Include="..\..\zlib\crc32.c" />
    <ClCompile Include="..\..\zlib\deflate.c" />
    <ClCompile Include="..\..\zlib\gzclose.c" />
    <ClCompile Include="..\..\zlib\gzlib.c" />
    <ClCompile Include="..\..\zlib\gzread.c" />
    <ClCompile Include="..\..\zlib\gzwrite.c" />
    <ClCompile Include="..\..\zlib\infback.c" />
    <ClCompile Include="..\..\zlib\inffast.c" />
    <ClCompile Include="..\..\zlib\inflate.c" />
    <ClCompile Include="..\..\zlib\inftrees.c" />
    <ClCompile Include="..\..\zlib\trees.c" />
    <ClCompile Include="..\..\zlib\uncompr.c" />
    <ClCompile Include="..\..\zlib\zutil.c" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\..\zlib\crc32.h" />
    <ClInclude Include="..\..\zlib\deflate.h" />
    <ClInclude Include="..\..\zlib\inffast.h" />
    <ClInclude Include="..\..\zlib\inffixed.h" />
    <ClInclude Include="..\..\zlib\inflate.h" />
    <ClInclude Include="..\..\zlib\inftrees.h" />
    <ClInclude Include="..\..\zlib\trees.h" />
    <ClInclude Include="..\..\zlib\zconf.h" />
    <ClInclude Include="..\..\zlib\zlib.h" />
    <ClInclude Include="..\..\zlib\zutil.h" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
'@
Set-Content -LiteralPath (Join-Path $zlibProjectDir "Zlib.vcxproj") -Value $zlibProject -Encoding UTF8

Write-Section "Installing MSBuild NASM/YASM build customizations"
$vcTargets = Get-VCTargetsPath
$buildCustomizations = Join-Path $vcTargets "BuildCustomizations"
New-Directory $buildCustomizations
$yasmSource = Join-Path $thirdparty "xy-VSFilter-xy_sub_filter_rc5\src"
foreach ($stem in @("YASM", "vsyasm")) {
  Copy-Item -LiteralPath (Join-Path $yasmSource "YASM.props") -Destination (Join-Path $buildCustomizations "$stem.props") -Force
  Copy-Item -LiteralPath (Join-Path $yasmSource "YASM.targets") -Destination (Join-Path $buildCustomizations "$stem.targets") -Force
  Copy-Item -LiteralPath (Join-Path $yasmSource "YASM.xml") -Destination (Join-Path $buildCustomizations "$stem.xml") -Force
}
Copy-Item -LiteralPath (Join-Path $thirdparty "Build\Libass\nasm.targets") -Destination (Join-Path $buildCustomizations "nasm.targets") -Force
$nasmProps = @'
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <AvailableItemName Include="Nasm">
      <Targets>_NasmAssemble</Targets>
    </AvailableItemName>
  </ItemGroup>
  <ItemDefinitionGroup>
    <Nasm>
      <ExcludeFromBuild>false</ExcludeFromBuild>
    </Nasm>
  </ItemDefinitionGroup>
</Project>
'@
Set-Content -LiteralPath (Join-Path $buildCustomizations "nasm.props") -Value $nasmProps -Encoding UTF8

$nasm = Ensure-ToolOnPath "nasm.exe" @(
  "C:\Program Files\NASM\nasm.exe",
  "C:\ProgramData\chocolatey\bin\nasm.exe",
  "C:\msys64\usr\bin\nasm.exe",
  "C:\msys64\ucrt64\bin\nasm.exe"
)
New-Directory "C:\NASM"
Copy-Item -LiteralPath $nasm -Destination "C:\NASM\Nasm.exe" -Force
[void](Ensure-ToolOnPath "yasm.exe" @("C:\msys64\usr\bin\yasm.exe", "C:\msys64\ucrt64\bin\yasm.exe"))

Write-Section "Generating AviSynth version headers"
$arch = if ($Platform -eq "x64") { "x86_64" } else { "x86_32" }
$seqrev = (& git -C $Root rev-list --count HEAD).Trim()
$branch = (& git -C $Root rev-parse --abbrev-ref HEAD).Trim() -replace '[^A-Za-z0-9_]', '_'
$coreDir = Join-Path $thirdparty "AviSynthPlus\avs_core\core"
$generatedDirs = @(
  $coreDir,
  (Join-Path $thirdparty "Build\AvisynthPlus\avs_core")
)
foreach ($dir in $generatedDirs) {
  New-Directory $dir
  $archContent = (Get-Content -LiteralPath (Join-Path $coreDir "arch.h.in") -Raw).Replace("@AVS_ARCH@", $arch)
  $versionContent = (Get-Content -LiteralPath (Join-Path $coreDir "version.h.in") -Raw).Replace("@AVS_SEQREV@", $seqrev).Replace("@AVS_BRANCH@", $branch)
  Set-Content -LiteralPath (Join-Path $dir "arch.h") -Value $archContent -Encoding ASCII
  Set-Content -LiteralPath (Join-Path $dir "version.h") -Value $versionContent -Encoding ASCII
}

Write-Section "Normalizing LuaJIT split-case source tree and generated headers"
$luaUpper = Join-Path $thirdparty "LuaJIT"
$luaLower = Join-Path $thirdparty "luajit"
New-Directory (Join-Path $luaUpper "src\host")
New-Directory (Join-Path $luaLower "src\host")
New-Directory (Join-Path $luaUpper "dynasm")
New-Directory (Join-Path $luaUpper "src\dynasm")

Get-ChildItem -LiteralPath (Join-Path $luaLower "src\host") -File -ErrorAction SilentlyContinue | ForEach-Object {
  Copy-IfPresent $_.FullName (Join-Path (Join-Path $luaUpper "src\host") $_.Name)
}
Get-ChildItem -LiteralPath (Join-Path $luaUpper "src\host") -File -ErrorAction SilentlyContinue | ForEach-Object {
  Copy-IfPresent $_.FullName (Join-Path (Join-Path $luaLower "src\host") $_.Name)
}
Copy-IfPresent (Join-Path $luaLower "dynasm\dynasm.lua") (Join-Path $luaUpper "dynasm\dynasm.lua")
Copy-IfPresent (Join-Path $luaLower "dynasm\dynasm.lua") (Join-Path $luaUpper "src\dynasm\dynasm.lua")
Copy-IfPresent (Join-Path $luaUpper "src\host\genversion.lua") (Join-Path $luaLower "src\host\genversion.lua")
Copy-IfPresent (Join-Path $luaUpper "src\luajit_rolling.h") (Join-Path $luaLower "src\luajit_rolling.h")
Copy-IfPresent (Join-Path $luaUpper "src\vm_x64.dasc") (Join-Path $luaLower "src\vm_x64.dasc")
Copy-IfPresent (Join-Path $luaLower "src\vm_x86.dasc") (Join-Path $luaUpper "src\vm_x86.dasc")
foreach ($dir in @($luaUpper, $luaLower)) {
  $relver = Join-Path $dir "src\luajit_relver.txt"
  if (-not (Test-Path -LiteralPath $relver)) { Set-Content -LiteralPath $relver -Value "ROLLING" -Encoding ASCII }
}

Write-Section "Prebuilding minilua and generated LuaJIT headers"
$solutionDir = $Root
if (-not $solutionDir.EndsWith([IO.Path]::DirectorySeparatorChar)) {
  $solutionDir += [IO.Path]::DirectorySeparatorChar
}
& msbuild (Join-Path $thirdparty "Build\Minilua\Minilua.vcxproj") `
  /m `
  /p:Configuration=$Configuration `
  /p:Platform=$Platform `
  /p:SolutionDir=$solutionDir `
  /p:PreferredToolArchitecture=x64 `
  /v:minimal
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$minilua = Join-Path $Root "bin\$Platform\$Configuration\minilua.exe"
if (-not (Test-Path -LiteralPath $minilua)) { $minilua = Join-Path $Root "bin\$Platform\$Configuration\minilua" }
if (-not (Test-Path -LiteralPath $minilua)) { throw "minilua was not built in bin\$Platform\$Configuration" }

$dynasm = Join-Path $luaUpper "dynasm\dynasm.lua"
$buildvmArch = Join-Path $luaUpper "src\host\buildvm_arch.h"
$vmDasc = Join-Path $luaUpper "src\vm_x64.dasc"
if ($Platform -ne "x64") { $vmDasc = Join-Path $luaUpper "src\vm_x86.dasc" }
$dynasmArgs = @($dynasm, "-LN", "-D", "WIN", "-D", "JIT", "-D", "FFI")
if ($Platform -eq "x64") { $dynasmArgs += @("-D", "P64") }
$dynasmArgs += @("-o", $buildvmArch, $vmDasc)
& $minilua @dynasmArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
if (-not (Test-Path -LiteralPath $buildvmArch)) { throw "LuaJIT buildvm_arch.h was not generated" }
Copy-IfPresent $buildvmArch (Join-Path $luaLower "src\host\buildvm_arch.h")

$genversion = Join-Path $luaUpper "src\host\genversion.lua"
$rolling = Join-Path $luaUpper "src\luajit_rolling.h"
$relverFile = Join-Path $luaUpper "src\luajit_relver.txt"
$luajitHeader = Join-Path $luaUpper "src\luajit.h"
& $minilua $genversion $rolling $relverFile $luajitHeader
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Copy-IfPresent $luajitHeader (Join-Path $luaLower "src\luajit.h")

Write-Section "Windows dependency preparation complete"
