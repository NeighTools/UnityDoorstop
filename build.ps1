$VERSION = "2.5.2"

function writeErrorTip($msg) {
    Write-Host $msg -BackgroundColor Red -ForegroundColor White
}

[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"

$TOOLS_DIR = Join-Path $PSScriptRoot "tools"
$XMAKE_DIR = Join-Path $TOOLS_DIR "xmake"

if ((Test-Path $PSScriptRoot) -and !(Test-Path $TOOLS_DIR)) {
    Write-Verbose -Message "Creating tools dir..."
    New-Item -Path $TOOLS_DIR -ItemType "directory" | Out-Null
}

if (!(Test-Path $XMAKE_DIR)) {
    $outfile = Join-Path $temppath "$pid-xmake.zip"
    $x64arch = @('AMD64', 'IA64', 'ARM64')
    $arch = if ($env:PROCESSOR_ARCHITECTURE -in $x64arch -or $env:PROCESSOR_ARCHITEW6432 -in $x64arch) { 'win64' } else { 'win32' }
    $url = "https://github.com/xmake-io/xmake/releases/download/v$VERSION/xmake-v$VERSION.$arch.zip"
    Write-Host "Downloading xmake ($arch) from $url..."

    try {
        Invoke-WebRequest $url -OutFile $outfile -UseBasicParsing
    }
    catch {
        writeErrorTip "Download failed!"
        throw
    }
    
    try {
        Expand-Archive -Path $outfile -DestinationPath $TOOLS_DIR
    }
    catch {
        writeErrorTip "Failed to extract!"
    }
    finally {
        Remove-Item -Path $outfile
    }
}

$XMAKE_EXE = Join-Path $XMAKE_DIR "xmake.exe"
Invoke-Expression "& $XMAKE_EXE f -a x86"
Invoke-Expression "& $XMAKE_EXE $($args -join " ")"
Invoke-Expression "& $XMAKE_EXE f -a x64"
Invoke-Expression "& $XMAKE_EXE $($args -join " ")"