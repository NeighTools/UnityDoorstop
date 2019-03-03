param($projectPath, $templatePath, $arch)

function Merge-Tokens($template, $tokens)
{
    return [regex]::Replace($template, '\$(?<token>\w+)\$', 
        { param($match) $tokens[$match.Groups['token'].Value] })
}

echo "Generating proxy template..."

$ptrType = ""
if($arch -eq "32") { $ptrType = "DWORD" }
elseif($arch -eq "64") { $ptrType = "QWORD" }

$defFilePath = Join-Path $projectPath "dll.def"
$defFile = (Get-Content $defFilePath).Split([System.Environment]::NewLine) | Select-Object -Skip 1

$sbDefs = [System.Text.StringBuilder]::new()
$sbVariables = [System.Text.StringBuilder]::new()
$sbJmps = [System.Text.StringBuilder]::new()

$sbProxyDefs = [System.Text.StringBuilder]::new()
$sbProxyAdd = [System.Text.StringBuilder]::new()

foreach($func in $defFile) {
    [void]$sbDefs.AppendLine("public $func")
    [void]$sbDefs.AppendLine("public __${func}__")
    [void]$sbVariables.AppendLine("  __${func}__ $ptrType 0")
    [void]$sbJmps.AppendLine("${func}:")
    [void]$sbJmps.AppendLine("  jmp $ptrType ptr __${func}__")

    [void]$sbProxyDefs.AppendLine("extern FARPROC __${func}__;")
    [void]$sbProxyAdd.AppendLine("__${func}__ = GetProcAddress(dll, ""${func}"");")
}



$asmTemplatePath = Join-Path $templatePath "dllproxy.asm.tmpl"
$asmTemplate = [IO.File]::ReadAllText($asmTemplatePath)
$asm = Merge-Tokens $asmTemplate @{ definitions = $sbDefs.ToString(); variables = $sbVariables.ToString(); jmps = $sbJmps.ToString() }
$asmPath = Join-Path $projectPath "dllproxy.asm" 
Set-Content $asmPath $asm
$cTemplatePath = Join-Path $templatePath "proxy.c.tmpl"
$cTemplate = [IO.File]::ReadAllText($cTemplatePath)
$cProxy = Merge-Tokens $cTemplate @{ proxyDefs = $sbProxyDefs.ToString(); proxyAdd = $sbProxyAdd.ToString() }
$cPath = Join-Path $projectPath "proxy.c" 
Set-Content $cPath $cProxy


