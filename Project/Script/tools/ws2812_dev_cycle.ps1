[CmdletBinding()]
param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ForwardArgs
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..'))
$pythonExe = Join-Path $repoRoot '.venv\Scripts\python.exe'
$entryScript = Join-Path $PSScriptRoot 'ws2812_dev_cycle.py'

if (-not (Test-Path $pythonExe)) {
    throw "Missing Python entrypoint: $pythonExe"
}

& $pythonExe $entryScript @ForwardArgs
exit $LASTEXITCODE