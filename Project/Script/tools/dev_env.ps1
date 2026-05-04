[CmdletBinding()]
param(
    [string]$IdfPath = 'S:\Embedded\ESP\v5.4.3\esp-idf',
    [string]$KeilUv4Path = 'S:\Embedded\Keil\UV4\uVision.com',
    [string]$McpPython = 'D:\GraduationProject\Project\xiaozhi-esp32\.venv\Scripts\python.exe',
    [string]$McpUrl = '',
    [string]$EspPort = '',
    [string]$Ai8051ComPort = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Set-EnvironmentValue {
    param(
        [string]$Name,
        [string]$Value,
        [switch]$AllowEmpty
    )

    if ([string]::IsNullOrWhiteSpace($Value) -and (-not $AllowEmpty)) {
        return
    }

    Set-Item -Path "Env:$Name" -Value $Value
}

function Write-EnvironmentLine {
    param(
        [string]$Name,
        [string]$Value
    )

    Write-Host ("{0}={1}" -f $Name, $Value)
}

Set-EnvironmentValue -Name 'WS2812_IDF_PATH' -Value $IdfPath
Set-EnvironmentValue -Name 'WS2812_KEIL_UV4_PATH' -Value $KeilUv4Path
Set-EnvironmentValue -Name 'WS2812_MCP_PYTHON' -Value $McpPython
Set-EnvironmentValue -Name 'WS2812_MCP_URL' -Value $McpUrl -AllowEmpty

if (-not [string]::IsNullOrWhiteSpace($EspPort)) {
    Set-EnvironmentValue -Name 'WS2812_ESP_PORT' -Value $EspPort
}

if (-not [string]::IsNullOrWhiteSpace($Ai8051ComPort)) {
    Set-EnvironmentValue -Name 'WS2812_AI8051_COM_PORT' -Value $Ai8051ComPort
}

Write-Host 'WS2812 development environment loaded for the current PowerShell session.'
Write-EnvironmentLine -Name 'WS2812_IDF_PATH' -Value $env:WS2812_IDF_PATH
Write-EnvironmentLine -Name 'WS2812_KEIL_UV4_PATH' -Value $env:WS2812_KEIL_UV4_PATH
Write-EnvironmentLine -Name 'WS2812_MCP_PYTHON' -Value $env:WS2812_MCP_PYTHON

if (-not [string]::IsNullOrWhiteSpace($env:WS2812_MCP_URL)) {
    Write-EnvironmentLine -Name 'WS2812_MCP_URL' -Value $env:WS2812_MCP_URL
}

if (-not [string]::IsNullOrWhiteSpace($env:WS2812_ESP_PORT)) {
    Write-EnvironmentLine -Name 'WS2812_ESP_PORT' -Value $env:WS2812_ESP_PORT
}

if (-not [string]::IsNullOrWhiteSpace($env:WS2812_AI8051_COM_PORT)) {
    Write-EnvironmentLine -Name 'WS2812_AI8051_COM_PORT' -Value $env:WS2812_AI8051_COM_PORT
}

Write-Host 'Next step: run .\.venv\Scripts\python.exe .\Project\Script\tools\ws2812_auto_debug.py in the same PowerShell session.'