$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
$venvPath = Join-Path $projectRoot '.venv'

if (-not (Test-Path $venvPath)) {
    python -m venv $venvPath
}

& (Join-Path $venvPath 'Scripts/pip.exe') install -r (Join-Path $projectRoot 'requirements.txt')

Write-Host "Virtual environment created at $venvPath"
Write-Host "Activate with: `"$venvPath\Scripts\Activate.ps1`""
