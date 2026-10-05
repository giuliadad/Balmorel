$RemoteHost = "login.gbar.dtu.dk"
$RemoteUser = "s243704"
$RemoteRoot = "/zhome/3d/c/214482/Desktop/Balmorel"
$Scenario = "base_no_dc_8"
$LocalRoot = Join-Path "C:\Balmorel" "$Scenario\model"

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
        Write-Error "Failed to download $RemoteFile"
        exit $LASTEXITCODE
    }
}

Write-Host "Results saved to: $LocalRoot"