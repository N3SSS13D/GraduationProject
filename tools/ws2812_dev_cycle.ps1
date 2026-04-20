[CmdletBinding()]
param(
    [ValidateSet('Run', 'Ai8051Monitor')]
    [string]$Mode = 'Run',

    [string]$IdfPath = $env:WS2812_IDF_PATH,
    [string]$XiaozhiProject = '',
    [string]$EspPort = $env:WS2812_ESP_PORT,
    [int]$EspMonitorBaud = 115200,

    [string]$KeilUv4Path = $env:WS2812_KEIL_UV4_PATH,
    [string]$KeilProject = '',

    [string]$Ai8051ComPort = $env:WS2812_AI8051_COM_PORT,
    [int]$Ai8051BaudRate = 9600,
    [int]$Ai8051ReconnectDelaySeconds = 20,
    [switch]$RunAi8051BtDebug,
    [string]$Ai8051BtCommandSequence = 'BT SEND AT|BT SEND AT+VERSION?|BT SEND AT+ADDR?|BT SEND AT+NAME?|BT SEND AT+PSWD?|BT SEND AT+UART?|BT STATUS',
    [int]$Ai8051BtInitialDelayMs = 1500,
    [int]$Ai8051BtCommandDelayMs = 1200,

    [string]$McpPython = $env:WS2812_MCP_PYTHON,
    [string]$McpScript = '',
    [string]$McpUrl = $env:WS2812_MCP_URL,
    [int]$McpHttpPort = 8765,

    [switch]$SkipMcp,
    [switch]$SkipXiaozhiMonitor,
    [switch]$SkipAi8051Monitor,
    [switch]$ValidateSnapshotControl,
    [int]$SnapshotQuality = 50,

    [switch]$Watch,
    [int]$DebounceSeconds = 3,
    [string[]]$WatchPaths = @(),

    [switch]$DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$script:XiaozhiMonitorProcess = $null
$script:McpProcess = $null
$script:Ai8051MonitorProcess = $null

function Write-Stage {
    param(
        [string]$Message
    )

    $timestamp = Get-Date -Format 'yyyy-MM-dd HH:mm:ss'
    Write-Host "[$timestamp] $Message"
}

function Set-DefaultPath {
    param(
        [string]$CurrentValue,
        [string]$FallbackValue
    )

    if ([string]::IsNullOrWhiteSpace($CurrentValue)) {
        return $FallbackValue
    }

    return $CurrentValue
}

function ConvertTo-PowerShellToken {
    param(
        [string]$Token
    )

    if ($Token -match '^[A-Za-z0-9_./:\\-]+$') {
        return $Token
    }

    return "'" + $Token.Replace("'", "''") + "'"
}

function Join-PowerShellTokens {
    param(
        [string[]]$Tokens
    )

    return (($Tokens | ForEach-Object { ConvertTo-PowerShellToken -Token $_ }) -join ' ')
}

function ConvertTo-EncodedCommand {
    param(
        [string]$CommandText
    )

    $bytes = [System.Text.Encoding]::Unicode.GetBytes($CommandText)
    return [Convert]::ToBase64String($bytes)
}

function Invoke-ManagedPowerShell {
    param(
        [string]$CommandText,
        [string]$WorkingDirectory,
        [string]$StepName
    )

    $encodedCommand = ConvertTo-EncodedCommand -CommandText $CommandText
    $arguments = @(
        '-NoLogo',
        '-NoProfile',
        '-ExecutionPolicy', 'Bypass',
        '-EncodedCommand', $encodedCommand
    )

    if ($DryRun) {
        Write-Stage "[dry-run] $StepName"
        Write-Host $CommandText
        return
    }

    Push-Location -Path $WorkingDirectory
    try {
        & powershell.exe @arguments
        if ($LASTEXITCODE -ne 0) {
            throw "$StepName failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        Pop-Location
    }
}

function Start-ManagedWindowProcess {
    param(
        [string]$Title,
        [string]$CommandText,
        [string]$WorkingDirectory,
        [string]$StepName
    )

    $fullCommand = "`$Host.UI.RawUI.WindowTitle = $(ConvertTo-PowerShellToken -Token $Title)`r`n$CommandText"
    $encodedCommand = ConvertTo-EncodedCommand -CommandText $fullCommand
    $arguments = @(
        '-NoLogo',
        '-NoExit',
        '-ExecutionPolicy', 'Bypass',
        '-EncodedCommand', $encodedCommand
    )

    if ($DryRun) {
        Write-Stage "[dry-run] $StepName"
        Write-Host $fullCommand
        return $null
    }

    return Start-Process -FilePath 'powershell.exe' -ArgumentList $arguments -WorkingDirectory $WorkingDirectory -PassThru
}

function Stop-ManagedProcess {
    param(
        [System.Diagnostics.Process]$Process,
        [string]$Name
    )

    if (($null -eq $Process) -or $Process.HasExited) {
        return $null
    }

    Write-Stage "Stopping $Name (PID $($Process.Id))."
    if (-not $DryRun) {
        Stop-Process -Id $Process.Id -Force
        $Process.WaitForExit()
    }
    return $null
}

function Resolve-PythonCommand {
    param(
        [string]$RepoRoot,
        [string]$PreferredPython,
        [string]$ProjectRoot
    )

    $candidates = @()
    if (-not [string]::IsNullOrWhiteSpace($PreferredPython)) {
        $candidates += $PreferredPython
    }
    $candidates += @(
        (Join-Path $RepoRoot '.venv\Scripts\python.exe'),
        (Join-Path $ProjectRoot '.venv\Scripts\python.exe'),
        'py.exe',
        'python.exe'
    )

    foreach ($candidate in $candidates) {
        if ([string]::IsNullOrWhiteSpace($candidate)) {
            continue
        }

        if (Test-Path -Path $candidate) {
            return (Resolve-Path -Path $candidate).Path
        }

        $command = Get-Command -Name $candidate -ErrorAction SilentlyContinue
        if ($null -ne $command) {
            return $command.Source
        }
    }

    throw 'Unable to locate a Python interpreter for gp_mcp_endpoint_client.py. Set WS2812_MCP_PYTHON or pass -McpPython.'
}

function Wait-ForHttpJson {
    param(
        [string]$Uri,
        [int]$TimeoutSeconds,
        [string]$Description
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while ((Get-Date) -lt $deadline) {
        try {
            return Invoke-RestMethod -Uri $Uri -Method Get -TimeoutSec 3
        }
        catch {
            Start-Sleep -Seconds 1
        }
    }

    throw "Timed out waiting for $Description at $Uri."
}

function Get-DryRunMcpStatus {
    param(
        [int]$HttpPort
    )

    return [pscustomobject]@{
        connected = $true
        initialized = $true
        status_url = "http://127.0.0.1:$HttpPort/status"
    }
}

function Invoke-HttpSnapshotValidation {
    param(
        [string]$Uri,
        [int]$Quality
    )

    $payload = @{
        quality = $Quality
        timeout = 10
    } | ConvertTo-Json

    if ($DryRun) {
        Write-Stage "[dry-run] Validate snapshot control via $Uri"
        return
    }

    Invoke-RestMethod -Uri $Uri -Method Post -ContentType 'application/json' -Body $payload -TimeoutSec 15 | Out-Null
}

function Invoke-XiaozhiBuildAndFlash {
    param(
        [string]$ExportScript,
        [string]$ProjectRoot,
        [string]$Port
    )

    $idfArgs = @('build', 'flash')
    if (-not [string]::IsNullOrWhiteSpace($Port)) {
        $idfArgs = @('-p', $Port) + $idfArgs
    }

    $idfCommand = '& ' + (Join-PowerShellTokens -Tokens (@('idf.py') + $idfArgs))
    $commandText = @"
`$ErrorActionPreference = 'Stop'
Set-ExecutionPolicy -Scope Process Bypass -Force
. $(ConvertTo-PowerShellToken -Token $ExportScript)
Set-Location $(ConvertTo-PowerShellToken -Token $ProjectRoot)
$idfCommand
"@

    Invoke-ManagedPowerShell -CommandText $commandText -WorkingDirectory $ProjectRoot -StepName 'XiaoZhi build/flash'
}

function Start-XiaozhiMonitor {
    param(
        [string]$ExportScript,
        [string]$ProjectRoot,
        [string]$Port,
        [int]$MonitorBaud
    )

    $idfArgs = @('monitor')
    if ($MonitorBaud -gt 0) {
        $idfArgs = @('-b', [string]$MonitorBaud) + $idfArgs
    }
    if (-not [string]::IsNullOrWhiteSpace($Port)) {
        $idfArgs = @('-p', $Port) + $idfArgs
    }

    $idfCommand = '& ' + (Join-PowerShellTokens -Tokens (@('idf.py') + $idfArgs))
    $commandText = @"
`$ErrorActionPreference = 'Stop'
Set-ExecutionPolicy -Scope Process Bypass -Force
. $(ConvertTo-PowerShellToken -Token $ExportScript)
Set-Location $(ConvertTo-PowerShellToken -Token $ProjectRoot)
Write-Host 'Press Ctrl+] to exit the ESP-IDF monitor.'
$idfCommand
"@

    return Start-ManagedWindowProcess -Title 'WS2812 XiaoZhi Monitor' -CommandText $commandText -WorkingDirectory $ProjectRoot -StepName 'Start XiaoZhi monitor'
}

function Start-McpBridge {
    param(
        [string]$ProjectRoot,
        [string]$PythonPath,
        [string]$ScriptPath,
        [string]$Url,
        [int]$HttpPort
    )

    $arguments = @($PythonPath, $ScriptPath, '--verbose', '--http-port', [string]$HttpPort)
    if (-not [string]::IsNullOrWhiteSpace($Url)) {
        $arguments += @('--url', $Url)
    }

    $commandText = @"
`$ErrorActionPreference = 'Stop'
Set-Location $(ConvertTo-PowerShellToken -Token $ProjectRoot)
& $(Join-PowerShellTokens -Tokens $arguments)
"@

    return Start-ManagedWindowProcess -Title 'WS2812 MCP Bridge' -CommandText $commandText -WorkingDirectory $ProjectRoot -StepName 'Start MCP bridge'
}

function Invoke-KeilBuild {
    param(
        [string]$Uv4Path,
        [string]$ProjectPath
    )

    if ($DryRun) {
        Write-Stage "[dry-run] Keil build $ProjectPath"
        return
    }

    Push-Location -Path (Split-Path -Path $ProjectPath -Parent)
    try {
        & $Uv4Path -b $ProjectPath
        if ($LASTEXITCODE -ne 0) {
            throw "Keil build failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        Pop-Location
    }
}

function Start-Ai8051Monitor {
    param(
        [string]$ScriptPath,
        [string]$ProjectRoot,
        [string]$ComPort,
        [int]$BaudRate,
        [switch]$RunBtDebug,
        [string]$BtCommandSequence,
        [int]$BtInitialDelayMs,
        [int]$BtCommandDelayMs
    )

    $arguments = @(
        '-NoLogo',
        '-NoExit',
        '-ExecutionPolicy', 'Bypass',
        '-File', $ScriptPath,
        '-Mode', 'Ai8051Monitor',
        '-Ai8051ComPort', $ComPort,
        '-Ai8051BaudRate', [string]$BaudRate,
        '-Ai8051BtCommandSequence', $BtCommandSequence,
        '-Ai8051BtInitialDelayMs', [string]$BtInitialDelayMs,
        '-Ai8051BtCommandDelayMs', [string]$BtCommandDelayMs
    )

    if ($RunBtDebug) {
        $arguments += '-RunAi8051BtDebug'
    }

    if ($DryRun) {
        Write-Stage "[dry-run] Start AI8051 serial monitor on $ComPort"
        if ($RunBtDebug) {
            Write-Host "BT monitor commands: $BtCommandSequence"
        }
        return $null
    }

    return Start-Process -FilePath 'powershell.exe' -ArgumentList $arguments -WorkingDirectory $ProjectRoot -PassThru
}

function Get-Ai8051BtCommands {
    param(
        [string]$CommandSequence
    )

    if ([string]::IsNullOrWhiteSpace($CommandSequence)) {
        return @()
    }

    return @(
        $CommandSequence.Split('|') |
            ForEach-Object { $_.Trim() } |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) }
    )
}

function Write-Ai8051SerialChunk {
    param(
        [System.IO.Ports.SerialPort]$SerialPort
    )

    $chunk = $SerialPort.ReadExisting()
    if (-not [string]::IsNullOrEmpty($chunk)) {
        Write-Host -NoNewline $chunk
    }
}

function Invoke-Ai8051BtDebugSequence {
    param(
        [System.IO.Ports.SerialPort]$SerialPort,
        [string[]]$Commands,
        [int]$InitialDelayMs,
        [int]$CommandDelayMs
    )

    $command = ''
    $deadline = Get-Date

    if (($null -eq $Commands) -or ($Commands.Count -eq 0)) {
        return
    }

    if ($InitialDelayMs -gt 0) {
        Start-Sleep -Milliseconds $InitialDelayMs
        Write-Ai8051SerialChunk -SerialPort $SerialPort
    }

    foreach ($command in $Commands) {
        Write-Host ("[HOST_BT_TX] {0}" -f $command)
        $SerialPort.Write("$command`r`n")
        $deadline = (Get-Date).AddMilliseconds($CommandDelayMs)
        while ((Get-Date) -lt $deadline) {
            Write-Ai8051SerialChunk -SerialPort $SerialPort
            Start-Sleep -Milliseconds 120
        }
    }
}

function Invoke-Ai8051MonitorLoop {
    param(
        [string]$ComPort,
        [int]$BaudRate
    )

    if ([string]::IsNullOrWhiteSpace($ComPort)) {
        throw 'Ai8051 serial monitor requires -Ai8051ComPort.'
    }

    Add-Type -AssemblyName System.IO.Ports

    $serialPort = New-Object System.IO.Ports.SerialPort $ComPort, $BaudRate, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
    $serialPort.ReadTimeout = 250
    $serialPort.DtrEnable = $false
    $serialPort.RtsEnable = $false

    try {
        $Host.UI.RawUI.WindowTitle = "WS2812 AI8051 Monitor ($ComPort)"
        $serialPort.Open()
        Write-Host "Listening on $ComPort @ $BaudRate 8N1. Press Ctrl+C to close."
        if ($RunAi8051BtDebug) {
            Invoke-Ai8051BtDebugSequence -SerialPort $serialPort -Commands (Get-Ai8051BtCommands -CommandSequence $Ai8051BtCommandSequence) -InitialDelayMs $Ai8051BtInitialDelayMs -CommandDelayMs $Ai8051BtCommandDelayMs
        }
        while ($true) {
            try {
                Write-Ai8051SerialChunk -SerialPort $serialPort
                Start-Sleep -Milliseconds 120
            }
            catch [System.TimeoutException] {
            }
        }
    }
    finally {
        if ($serialPort.IsOpen) {
            $serialPort.Close()
        }
        $serialPort.Dispose()
    }
}

function New-DefaultWatchSpecs {
    param(
        [string]$RepoRoot,
        [string]$ProjectRoot,
        [string[]]$CustomPaths
    )

    if (($null -ne $CustomPaths) -and ($CustomPaths.Count -gt 0)) {
        return ($CustomPaths | ForEach-Object {
            @{
                Path = $_
                Filter = '*.*'
                IncludeSubdirectories = $true
            }
        })
    }

    return @(
        @{
            Path = Join-Path $ProjectRoot 'main'
            Filter = '*.*'
            IncludeSubdirectories = $true
        },
        @{
            Path = Join-Path $ProjectRoot 'GP_Port'
            Filter = '*.*'
            IncludeSubdirectories = $true
        },
        @{
            Path = $ProjectRoot
            Filter = 'sdkconfig*'
            IncludeSubdirectories = $false
        },
        @{
            Path = $ProjectRoot
            Filter = 'CMakeLists.txt'
            IncludeSubdirectories = $false
        },
        @{
            Path = Join-Path $RepoRoot 'STC51\Project\ws2812_driver\Sources'
            Filter = '*.*'
            IncludeSubdirectories = $true
        },
        @{
            Path = Join-Path $RepoRoot 'STC51\Project\ws2812_driver'
            Filter = 'ws2812_driver.uvproj'
            IncludeSubdirectories = $false
        }
    )
}

function Start-Watchers {
    param(
        [hashtable[]]$Specs,
        [hashtable]$Signal
    )

    $createdWatchers = @()
    $subscriptionIds = @()
    $action = {
        $state = $Event.MessageData
        $state.Pending = $true
        $state.LastChange = Get-Date
        if (($null -ne $EventArgs) -and ($null -ne $EventArgs.FullPath)) {
            $state.LastPath = $EventArgs.FullPath
        }
    }

    foreach ($spec in $Specs) {
        if (-not (Test-Path -Path $spec.Path)) {
            continue
        }

        $watcher = New-Object System.IO.FileSystemWatcher
        $watcher.Path = $spec.Path
        $watcher.Filter = $spec.Filter
        $watcher.IncludeSubdirectories = [bool]$spec.IncludeSubdirectories
        $watcher.NotifyFilter = [System.IO.NotifyFilters]'FileName, LastWrite, CreationTime, DirectoryName'
        $watcher.EnableRaisingEvents = $true
        $createdWatchers += $watcher

        foreach ($eventName in @('Changed', 'Created', 'Deleted', 'Renamed')) {
            $subscription = Register-ObjectEvent -InputObject $watcher -EventName $eventName -Action $action -MessageData $Signal
            $subscriptionIds += $subscription.Id
        }
    }

    return @{
        Watchers = $createdWatchers
        SubscriptionIds = $subscriptionIds
    }
}

function Stop-Watchers {
    param(
        [hashtable]$WatcherState
    )

    foreach ($subscriptionId in $WatcherState.SubscriptionIds) {
        Unregister-Event -SubscriptionId $subscriptionId -ErrorAction SilentlyContinue
    }

    foreach ($watcher in $WatcherState.Watchers) {
        $watcher.EnableRaisingEvents = $false
        $watcher.Dispose()
    }
}

function Invoke-DevelopmentCycle {
    param(
        [string]$RepoRoot,
        [string]$ProjectRoot,
        [string]$ExportScript,
        [string]$Uv4Path,
        [string]$UvProjectPath,
        [string]$ComPort,
        [string]$PythonPath,
        [string]$McpScriptPath,
        [string]$DevicePort
    )

    $script:XiaozhiMonitorProcess = Stop-ManagedProcess -Process $script:XiaozhiMonitorProcess -Name 'XiaoZhi monitor'
    $script:Ai8051MonitorProcess = Stop-ManagedProcess -Process $script:Ai8051MonitorProcess -Name 'AI8051 monitor'

    Write-Stage 'Step 1/7: Build and flash XiaoZhi firmware.'
    Invoke-XiaozhiBuildAndFlash -ExportScript $ExportScript -ProjectRoot $ProjectRoot -Port $DevicePort

    if (-not $SkipXiaozhiMonitor) {
        Write-Stage 'Step 2/7: Start XiaoZhi monitor.'
        $script:XiaozhiMonitorProcess = Start-XiaozhiMonitor -ExportScript $ExportScript -ProjectRoot $ProjectRoot -Port $DevicePort -MonitorBaud $EspMonitorBaud
    }

    if (-not $SkipMcp) {
        Write-Stage 'Step 3/7: Restart local MCP bridge.'
        $script:McpProcess = Stop-ManagedProcess -Process $script:McpProcess -Name 'MCP bridge'
        $script:McpProcess = Start-McpBridge -ProjectRoot $ProjectRoot -PythonPath $PythonPath -ScriptPath $McpScriptPath -Url $McpUrl -HttpPort $McpHttpPort

        Write-Stage 'Step 4/7: Wait for MCP HTTP status endpoint.'
        $statusUri = "http://127.0.0.1:$McpHttpPort/status"
        if ($DryRun) {
            $status = Get-DryRunMcpStatus -HttpPort $McpHttpPort
        }
        else {
            $status = Wait-ForHttpJson -Uri $statusUri -TimeoutSeconds 20 -Description 'MCP status endpoint'
        }
        Write-Host ("MCP status: connected={0} initialized={1} status_url={2}" -f $status.connected, $status.initialized, $status.status_url)

        if ($ValidateSnapshotControl) {
            Write-Stage 'Step 5/7: Validate /control/snapshot trigger.'
            Invoke-HttpSnapshotValidation -Uri "http://127.0.0.1:$McpHttpPort/control/snapshot" -Quality $SnapshotQuality
        }
    }

    Write-Stage 'Step 6/7: Build AI8051 Keil project.'
    Invoke-KeilBuild -Uv4Path $Uv4Path -ProjectPath $UvProjectPath

    if ($Ai8051ReconnectDelaySeconds -gt 0) {
        Write-Stage "Waiting $Ai8051ReconnectDelaySeconds seconds before reopening AI8051 serial monitor."
        if (-not $DryRun) {
            Start-Sleep -Seconds $Ai8051ReconnectDelaySeconds
        }
    }

    if (-not $SkipAi8051Monitor) {
        Write-Stage 'Step 7/7: Start AI8051 serial monitor.'
        $script:Ai8051MonitorProcess = Start-Ai8051Monitor -ScriptPath $script:ToolScriptPath -ProjectRoot $RepoRoot -ComPort $ComPort -BaudRate $Ai8051BaudRate -RunBtDebug:$RunAi8051BtDebug -BtCommandSequence $Ai8051BtCommandSequence -BtInitialDelayMs $Ai8051BtInitialDelayMs -BtCommandDelayMs $Ai8051BtCommandDelayMs
    }
}

$script:ToolScriptPath = $PSCommandPath
$repoRoot = Resolve-Path -Path (Join-Path $PSScriptRoot '..')
$XiaozhiProject = Set-DefaultPath -CurrentValue $XiaozhiProject -FallbackValue (Join-Path $repoRoot 'External\xiaozhi-esp32')
$KeilProject = Set-DefaultPath -CurrentValue $KeilProject -FallbackValue (Join-Path $repoRoot 'STC51\Project\ws2812_driver\ws2812_driver.uvproj')
$McpScript = Set-DefaultPath -CurrentValue $McpScript -FallbackValue (Join-Path $XiaozhiProject 'GP_Port\gp_mcp_endpoint_client.py')
$IdfPath = Set-DefaultPath -CurrentValue $IdfPath -FallbackValue 'S:\Embedded\ESP\v5.4.3\esp-idf'
$KeilUv4Path = Set-DefaultPath -CurrentValue $KeilUv4Path -FallbackValue 'S:\Embedded\Keil\UV4\UV4.exe'

if ($Mode -eq 'Ai8051Monitor') {
    Invoke-Ai8051MonitorLoop -ComPort $Ai8051ComPort -BaudRate $Ai8051BaudRate
    return
}

$exportScript = Join-Path $IdfPath 'export.ps1'
if (-not (Test-Path -Path $exportScript)) {
    throw "ESP-IDF export script not found: $exportScript"
}
if (-not (Test-Path -Path $XiaozhiProject)) {
    throw "XiaoZhi project not found: $XiaozhiProject"
}
if (-not (Test-Path -Path $KeilProject)) {
    throw "Keil project not found: $KeilProject"
}
if (-not (Test-Path -Path $KeilUv4Path)) {
    throw "Keil UV4 executable not found: $KeilUv4Path"
}
if ((-not $SkipAi8051Monitor) -and [string]::IsNullOrWhiteSpace($Ai8051ComPort)) {
    throw 'AI8051 serial monitor requires -Ai8051ComPort or WS2812_AI8051_COM_PORT.'
}
if ((-not $SkipMcp) -and (-not (Test-Path -Path $McpScript))) {
    throw "MCP script not found: $McpScript"
}

$resolvedMcpPython = $null
if (-not $SkipMcp) {
    $resolvedMcpPython = Resolve-PythonCommand -RepoRoot $repoRoot -PreferredPython $McpPython -ProjectRoot $XiaozhiProject
}

Write-Stage "Repository root: $repoRoot"
Write-Stage "XiaoZhi project: $XiaozhiProject"
Write-Stage "Keil project: $KeilProject"
if (-not $SkipMcp) {
    Write-Stage "MCP Python: $resolvedMcpPython"
}

$watcherState = $null

try {
    Invoke-DevelopmentCycle -RepoRoot $repoRoot -ProjectRoot $XiaozhiProject -ExportScript $exportScript -Uv4Path $KeilUv4Path -UvProjectPath $KeilProject -ComPort $Ai8051ComPort -PythonPath $resolvedMcpPython -McpScriptPath $McpScript -DevicePort $EspPort

    if ($Watch) {
        $watchSignal = [hashtable]::Synchronized(@{
            Pending = $false
            LastChange = Get-Date
            LastPath = ''
        })
        $watcherState = Start-Watchers -Specs (New-DefaultWatchSpecs -RepoRoot $repoRoot -ProjectRoot $XiaozhiProject -CustomPaths $WatchPaths) -Signal $watchSignal
        Write-Stage 'Watch mode is active. Waiting for source changes.'

        while ($true) {
            $eventRecord = Wait-Event -Timeout 1
            if ($null -ne $eventRecord) {
                Remove-Event -EventIdentifier $eventRecord.EventIdentifier -ErrorAction SilentlyContinue
            }

            if ($watchSignal.Pending) {
                $elapsed = (Get-Date) - $watchSignal.LastChange
                if ($elapsed.TotalSeconds -ge $DebounceSeconds) {
                    $watchSignal.Pending = $false
                    Write-Stage "Change detected at $($watchSignal.LastPath). Restarting development cycle."
                    Invoke-DevelopmentCycle -RepoRoot $repoRoot -ProjectRoot $XiaozhiProject -ExportScript $exportScript -Uv4Path $KeilUv4Path -UvProjectPath $KeilProject -ComPort $Ai8051ComPort -PythonPath $resolvedMcpPython -McpScriptPath $McpScript -DevicePort $EspPort
                    Write-Stage 'Watch mode is active. Waiting for the next source change.'
                }
            }
        }
    }
}
finally {
    if ($null -ne $watcherState) {
        Stop-Watchers -WatcherState $watcherState
    }

    if ($Watch) {
        $script:XiaozhiMonitorProcess = Stop-ManagedProcess -Process $script:XiaozhiMonitorProcess -Name 'XiaoZhi monitor'
        $script:McpProcess = Stop-ManagedProcess -Process $script:McpProcess -Name 'MCP bridge'
        $script:Ai8051MonitorProcess = Stop-ManagedProcess -Process $script:Ai8051MonitorProcess -Name 'AI8051 monitor'
    }

    if (-not $Watch) {
        Write-Stage 'One-shot cycle finished. Child monitor windows stay open until you close them manually.'
    }
}