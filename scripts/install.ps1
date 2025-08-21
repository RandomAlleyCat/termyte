$ErrorActionPreference = 'Stop'

if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
    Write-Error 'python is not installed.'
    exit 1
}

try {
    python -m pip --version | Out-Null
} catch {
    Write-Error 'pip is not installed.'
    exit 1
}

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
$venvPath = Join-Path $projectRoot '.venv'

if (-not (Test-Path $venvPath)) {
    python -m venv $venvPath
}

$venvPython = Join-Path $venvPath 'Scripts/python.exe'
& $venvPython -m pip install --upgrade pip

$requirements = Join-Path $projectRoot 'requirements.txt'
if ($IsLinux) {
    & $venvPython -m pip install -r $requirements
} else {
    $tmp = New-TemporaryFile
    Get-Content $requirements | Where-Object { $_ -notmatch '^evdev' } | Set-Content $tmp
    & $venvPython -m pip install -r $tmp
    Remove-Item $tmp
}

Write-Host "Virtual environment created at $venvPath"
Write-Host "Activate with: `"$venvPath\Scripts\Activate.ps1`""
