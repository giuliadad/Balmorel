# ==========================================================
#  Download the main result files (MainResults*.gdx) of all scenarios from DTU HPC to local machine
#  Layout:  <LocalRoot>\<scenario folder>\<same sub-path>\<original file name>
#  (sub-paths are kept because e.g. BYOC has all_endofmodel.gdx in both model\ and Results\)
#  Files that already exist locally are skipped (whatever their size), so it can be re-run/resumed.
#  If a download was interrupted, delete the partial file before re-running.
#  Only MainResults*.gdx files are included. Excludes Z_Archive_Scenarios.
# ==========================================================
$User      = "s243704"
$HpcHost   = "transfer.gbar.dtu.dk"
$HpcRoot   = "/work3/s243704/Balmorel"
$LocalRoot = "C:\Balmorel"
$h = "$User@$HpcHost"

# relative path<TAB>size in bytes
$files = @'
base_dc/model/MainResults_3day.gdx	131793056
base_dc/model/MainResults_fullweek.gdx	303345122
base_dc/model/MainResults_refchrono.gdx	130409880
base_nodc/model/MainResults_3days.gdx	131662227
base_nodc/model/MainResults_fullweek.gdx	303288071
base_nodc/model/MainResults_refchrono.gdx	130327573
BYOC_Flex/model/MainResults.gdx	297203992
BYOC/model/MainResults.gdx	298541989
Flex_1/model/MainResults_12h.gdx	131740179
Flex_1/model/MainResults_24h.gdx	131742786
Flex_1/model/MainResults_fullweek_12h.gdx	303350089
Flex_1/model/MainResults_fullweek_1h.gdx	303256083
Flex_1/model/MainResults_fullweek_24h.gdx	303476971
Flex_1_share100_cap125_12h_3day/model/MainResults.gdx	131730980
Flex_1_share100_cap125_12h/model/MainResults.gdx	303198319
Flex_1_share100_cap125_1h_3day/model/MainResults.gdx	131583744
Flex_1_share100_cap125_1h/model/MainResults.gdx	302951297
Flex_1_share100_cap125_24h_3day/model/MainResults.gdx	131734397
Flex_1_share100_cap125_24h/model/MainResults.gdx	303362474
Flex_1_share20_cap125_12h/model/MainResults.gdx	303359469
Flex_1_share20_cap125_1h/model/MainResults.gdx	303505241
Flex_1_share20_cap125_24h/model/MainResults.gdx	303355203
Flex_1_share60_cap125_12h_3day/model/MainResults.gdx	131681785
Flex_1_share60_cap125_12h/model/MainResults.gdx	303510568
Flex_1_share60_cap125_1h_3day/model/MainResults.gdx	131641639
Flex_1_share60_cap125_1h/model/MainResults.gdx	303268209
Flex_1_share60_cap125_24h_3day/model/MainResults.gdx	131691503
Flex_1_share60_cap125_24h/model/MainResults.gdx	303537333
sen_sc_base_dc/model/MainResults.gdx	24140883
sen_sc_Flex_1_share100_cap125_12h/model/MainResults.gdx	303486217
'@ -split "`n" | Where-Object { $_.Trim() } | ForEach-Object { $p = $_.Trim() -split "`t"; [pscustomobject]@{ Path = $p[0]; Size = [int64]$p[1] } }

$i = 0
foreach ($f in $files) {
    $i++
    $dest = Join-Path $LocalRoot ($f.Path -replace "/", "\")
    if (Test-Path $dest) { Write-Host "[$i/$($files.Count)] skip  $($f.Path)"; continue }
    New-Item -ItemType Directory -Force -Path (Split-Path $dest) | Out-Null
    Write-Host "[$i/$($files.Count)] get   $($f.Path)"
    scp "${h}:$HpcRoot/$($f.Path)" $dest
    if ($LASTEXITCODE -ne 0) { Write-Warning "FAILED: $($f.Path)" }
}
Write-Host "Done. Files saved under $LocalRoot"
