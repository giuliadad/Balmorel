param(
    [string]$RemoteHost = "login2.hpc.dtu.dk",
    [string]$RemoteUser = "s243704",
    [string]$RemoteRoot = "/zhome/3d/c/214482/Desktop/Balmorel",
    [string]$Scenario = "base_dc",
    [string]$LocalRoot = (Join-Path $PSScriptRoot "$Scenario\model\results")
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command scp -ErrorAction SilentlyContinue)) {
    throw "OpenSSH scp was not found. Install/enable OpenSSH Client and retry."
}

New-Item -ItemType Directory -Force -Path $LocalRoot | Out-Null

$Files = @(
    "$RemoteRoot/$Scenario/model/MainResults.gdx"
    "$RemoteRoot/$Scenario/model/BALBASE4_p.gdx"
    "$RemoteRoot/$Scenario/model/all_endofmodel.gdx"
    "$RemoteRoot/$Scenario/output/inputout/INPUTDATAOUT.gdx"
    "$RemoteRoot/$Scenario/logerror/logfile.out"
    "$RemoteRoot/$Scenario/logerror/errors.out"
)

foreach ($RemoteFile in $Files) {
    $FileName = Split-Path $RemoteFile -Leaf
    $LocalFile = Join-Path $LocalRoot $FileName
    $RemoteSpec = "$($RemoteUser)@$($RemoteHost):$RemoteFile"

    Write-Host "Downloading $FileName..."

    & scp $RemoteSpec $LocalFile

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to download '$RemoteFile'. Check the remote path, VPN/DTU network connection, and SSH credentials."
        exit $LASTEXITCODE
    }
}

Write-Host "Results saved to: $LocalRoot"