# ==========================================================
#  Download Balmorel results from DTU HPC to local machine
#  Only change the scenario name below
# ==========================================================
$Scenario = "base_no_dc_4_full"

# ---- Settings (normally no need to change) ----
$User      = "s243704"
$HpcHost   = "transfer.gbar.dtu.dk"
$HpcRoot   = "/zhome/3d/c/214482/Desktop/Balmorel"
$LocalRoot = "C:\Balmorel"

$h      = "$User@$HpcHost"
$remote = "$HpcRoot/$Scenario"
$local  = "$LocalRoot\$Scenario"

# 1. Create the local results folder
New-Item -ItemType Directory -Force -Path "$local\model\results" | Out-Null


# 3. Copy the result files
scp "${h}:$remote/model/MainResults.gdx" `
    "${h}:$remote/model/BALBASE4_p.gdx" `
    "$local\model\results\"


Write-Host "Done: results for '$Scenario' saved to $local"
