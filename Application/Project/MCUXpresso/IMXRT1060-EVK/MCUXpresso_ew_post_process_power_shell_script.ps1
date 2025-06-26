#*******************************************************************************
#
# E M B E D D E D   W I Z A R D   P R O J E C T
#
#                                                Copyright (c) TARA Systems GmbH
#                                    written by Paul Banach and Manfred Schweyer
#
#
# PowerShell script for modification of the MCUXpresso project configuration file
# according to Embedded Wizard project settings (color format, screen
# orientation, generated source files)
#
#*******************************************************************************


######################### FUNCTIONS ############################

function addElement($e1, $name2, $value2, $attr2, $value3, $attr3 )
{
   #Write-Host "addElement" $name2 $value2  $attr2

  if ($e1.gettype().name -eq "XmlDocument") {
    $e2 = $e1.CreateElement($name2, $namespace)
  }else {
    $e2 = $e1.ownerDocument.CreateElement($name2, $namespace)
  }

  if ($attr2) {
    $e2.setAttribute($value2,$attr2)
    if ($attr3) {
      $e2.setAttribute($value3,$attr3)
    }
  } elseif ($value2) {
    $e2.InnerText = "$value2"
  }

#  write-host "add" $e2.InnerText.ToString()
  $newChild = $e1.AppendChild($e2) | Out-Null
  return $newChild
}


######################### MAIN ############################


# the window title
$host.ui.RawUI.WindowTitle = "Embedded Wizard Post Process - updating MCUXpresso project configuration"
$executingScriptDirectory  = Split-Path -Path $MyInvocation.MyCommand.Definition -Parent

# check for PowerShell version
$invalidPsVersion = 0
$psVersion        = $PSVersionTable.PSVersion
if ( $psVersion )
{
  Write-Host "PowerShell version: " $psVersion
  If($psVersion -lt '4.0') {
    $invalidPsVersion = 1;
  }
}
Else {
  Write-Host "No PowerShell Version found"
  $invalidPsVersion = 1;
}

if ( $invalidPsVersion ) {
  Write-Host This Embedded Wizard Post Process needs at least V4.0 of Windows Powershell
  Write-Host Powershell update can be found at:
  Write-Host https://docs.microsoft.com/en-us/powershell/scripting/setup/installing-windows-powershell
  Write-Host
  Read-Host Press Enter to continue | Out-Null
  [Environment]::Exit(10)
}

######################### CHECK PRO VERSION ############################
$Path = Join-Path $executingScriptDirectory "../../../../PlatformPackage/RTE/ewcolor.c"

$UseLibaries = 1
if (Test-Path $path)
{
    $UseLibaries = 0
}

######################### COLORFORMAT and ROTATION ############################

# get color format
$NewColorFormat = $env:EmWi_ColorFormat

# check, if requested color format is supported
$ppDir = Join-Path $executingScriptDirectory "../../../../PlatformPackage/$NewColorFormat"
if ( -Not ( Test-Path -Path $ppDir ))
{
  Write-Host "Selected color format" $NewColorFormat "not supported!"
  Read-Host Press Enter to continue | Out-Null
  [Environment]::Exit(8)
}


# get screen orientation
$ScreenOrientation     = $env:EmWi_SurfaceRotation
if ( $ScreenOrientation -eq 90 ) {
  $ScreenOrientation_ext = "-r90"
} elseif ( $ScreenOrientation -eq 180 ) {
  $ScreenOrientation_ext = "-r180"
} elseif ( $ScreenOrientation -eq 270 ) {
  $ScreenOrientation_ext = "-r270"
} else {
  $ScreenOrientation_ext = ""
}


######################### EDIT MCUXpresso PROJECT ############################

# Open file
$executingScriptDirectory = Split-Path -Path $MyInvocation.MyCommand.Definition -Parent
$Path = Join-Path $executingScriptDirectory ".cproject"

if ( -Not ( Test-Path -Path $Path ))
{
  Write-Host eclipse project configuration file not found at path:
  Write-Host $Path
  Write-Host
  Read-Host Press Enter to continue | Out-Null
  [Environment]::Exit(6)
}

$xml = [xml](get-content $Path)
$xml.Load($Path)

######################### PATHS ############################

# Select include path node
$includePathNode = ($xml.cproject.storageModule.cconfiguration.storageModule.configuration.folderInfo.toolChain.tool.option | where {$_.superClass -eq "gnu.c.compiler.option.include.paths" -and $_.valueType -eq "includePath"})
if ($includePathNode) {
  # Delete all platform package include paths
  $DeleteNodes  = $includePathNode.listOptionValue | where {$_.value -like "*PlatformPackage*"}
  foreach($node in $DeleteNodes) {$node.ParentNode.RemoveChild($node) | Out-Null }

  # Insert new include paths
  addElement $includePathNode "listOptionValue" "builtIn" "false"  "value" "&quot;`${ProjDirPath}/../../../../PlatformPackage/RTE&quot;"
  addElement $includePathNode "listOptionValue" "builtIn" "false"  "value" "&quot;`${ProjDirPath}/../../../../PlatformPackage/$NewColorFormat&quot;"
}

########################## SYMBOLS ############################

$symbolsNode = ($xml.cproject.storageModule.cconfiguration.storageModule.configuration.folderInfo.toolChain.tool.option | where {$_.superClass -eq "gnu.c.compiler.option.preprocessor.def.symbols"   -and $_.valueType -eq "definedSymbols"})
if ($symbolsNode) {
  # Delete the symbols EW_FRAME_BUFFER_COLOR_FORMAT and EW_SURFACE_ROTATION
  $DeleteNodes  = $symbolsNode.listOptionValue | where {$_.value -like "*EW_FRAME_BUFFER_COLOR_FORMAT*"}
  foreach($node in $DeleteNodes) {$node.ParentNode.RemoveChild($node) | Out-Null }
  $DeleteNodes  = $symbolsNode.listOptionValue | where {$_.value -like "*EW_SURFACE_ROTATION*"}
  foreach($node in $DeleteNodes) {$node.ParentNode.RemoveChild($node) | Out-Null }

  # Insert new EW_ symbols
  addElement $symbolsNode "listOptionValue" "builtIn" "false"  "value" "EW_FRAME_BUFFER_COLOR_FORMAT=EW_FRAME_BUFFER_COLOR_FORMAT_$NewColorFormat"
  addElement $symbolsNode "listOptionValue" "builtIn" "false"  "value" "EW_SURFACE_ROTATION=$ScreenOrientation"
}

########################## LIBARIES ############################

  # Select libs node
  $libsNode = ($xml.cproject.storageModule.cconfiguration.storageModule.configuration.folderInfo.toolChain.tool.option | where {$_.superClass -like "gnu.c.link.option.libs" -and $_.valueType -like "libs"})
  # Delete all emwi libs
  $DeleteNodes  = $libsNode.listOptionValue | where {$_.value -like "*ewgfx*" -or $_.value -like "*ewrte*" }
  foreach($node in $DeleteNodes) {$node.ParentNode.RemoveChild($node) | Out-Null }

  # Select libary path node
  $libPathNode = ($xml.cproject.storageModule.cconfiguration.storageModule.configuration.folderInfo.toolChain.tool.option | where {$_.superClass -like "gnu.c.link.option.paths" -and $_.valueType -like "libPaths"})
  # Delete all emwi lib pathes
  $DeleteNodes  = $libPathNode.listOptionValue | where {$_.value -like "*PlatformPackage*"}
  foreach($node in $DeleteNodes) {$node.ParentNode.RemoveChild($node) | Out-Null }



if($UseLibaries)
{
  # detect lib variant
  $lib_variants = "-m4,-m7,-m33,-4ml"
  foreach($lv in $lib_variants.split(",")) {
    $libPath = Join-Path $executingScriptDirectory "../../../../PlatformPackage/RTE/libewrte$lv-gcc.a"
    if (Test-Path $libPath ) {
      $libVariant=$lv
    }
  }

  if ($libsNode) {
    # Insert new libaries
    addElement $libsNode "listOptionValue" "builtIn" "false"  "value" "ewgfx$libVariant$ScreenOrientation_ext-gcc"
    addElement $libsNode "listOptionValue" "builtIn" "false"  "value" "ewrte$libVariant-gcc"
  }

  if ($libPathNode) {
    # Insert new libaries
    addElement $libPathNode "listOptionValue" "builtIn" "false"  "value" "&quot;`${ProjDirPath}/../../../../PlatformPackage/$NewColorFormat&quot;"
    addElement $libPathNode "listOptionValue" "builtIn" "false"  "value" "&quot;`${ProjDirPath}/../../../../PlatformPackage/RTE&quot;"
  }
}

########################## EXCLUDES FROM BUILD ############################
$color_formats = "RGBA8888,RGBA4444,RGB888,RGB565,LumA44,Index8"
foreach($col in $color_formats.split(",")) {
  $ppPath = Join-Path $executingScriptDirectory "../../../../PlatformPackage/$col"
  if (( $NewColorFormat -notmatch $col ) -and ( Test-Path $ppPath )) {
    $excludes=$excludes + "PlatformPackage/" + $col + "|"
  }
}
$xml.cproject.storageModule.cconfiguration.storageModule.configuration.sourceEntries.entry.excluding = $excludes -replace ".$"


# Save and close file
$xml.Save($Path)

# Adapt ampersand
(get-content $Path) | foreach-object {$_ -replace "amp;quot;", "quot;"} | set-content $Path
