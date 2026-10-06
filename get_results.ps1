# ==========================================================
#  Download Balmorel results from DTU HPC to local machine
#  Only change the scenario name below
# ==========================================================
$Scenario = "base_dc"

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

# 2. On the HPC, copy the log files Balmorel wrote with Windows-style
#    names (e.g. 'model/..\logerror\errors.out') into the logerror folder
$fixLogs = 'cd ' + $remote + '/model && mkdir -p ../logerror && for f in .*logerror*; do [ -e $f ] && cp -p -- $f ../logerror/${f##*\\}; done; true'
ssh $h $fixLogs

# 3. Copy the result files
scp "${h}:$remote/model/MainResults.gdx" `
    "${h}:$remote/model/all_endofmodel.gdx" `
    "${h}:$remote/model/BALBASE4_p.gdx" `
    "${h}:$remote/model/Balmorel.lst" `
    "$local\model\results\"

# 4. Copy the log folder
scp -r "${h}:$remote/logerror" "$local\"

Write-Host "Done: results for '$Scenario' saved to $local"
