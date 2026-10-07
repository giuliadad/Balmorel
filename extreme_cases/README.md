# Extreme cases

Four scenarios that push one input to an extreme, plus a reference run. They test whether the model reacts in the expected
direction (Part 2 of `NOTEBOOK_Verification.ipynb`). The scenario folders sit in the Balmorel root next to the other scenarios;
this folder only holds the generator and this description.

All five are copies of **`base_dc_4`** (same `DE.inc`, `DE_DATACENTER.inc`, solver options and job script) with a reduced time set so
that they solve quickly:

| Setting | Value | Taken from |
|---|---|---|
| Years | 2035 only | `Y.inc` written by the script |
| Weeks | S02 and S28 (one winter and one summer week) | `SEASONS` in the script |
| Hours | Thursday to Saturday of each week, every 3 hours (T073, T076, ..., T142, 24 steps) | `base_dc_8/data/T.inc` |

Files that are not in a scenario's `data` folder are read from `base/data`.

| Scenario | Change | Files overridden in `data/` | Expected outcome vs `ext_reference` |
|---|---|---|---|
| `ext_reference` | none | - | the run the others are compared with |
| `ext_no_pv` | No PV anywhere: PV full-load hours set to 0 (solar heat is allowed) | `SOLEFLH.inc` | No PV generation, no new PV; more wind and other generation; objective up |
| `ext_onshore_wind_capex_x10` | Investment cost of new onshore wind x 10 (36 technologies, all areas where they can be built) | `GINVCOST.inc` | Far less new onshore wind (replaced by other generation); existing onshore wind unchanged; objective up |
| `ext_no_transmission_expansion` | No new transmission capacity | `XINVCOST.inc` (empty) | No endogenous transmission capacity; more generation investment; larger price differences between regions; objective up |
| `ext_no_france_interconnectors` | All links of France removed and no new ones: FR <-> BE, CH, DE4-S, ES, IT, UK (existing capacity, `XKFX.inc`) and FR-LU (investment option only, `XINVCOST.inc`) | `XKFX.inc`, `XINVCOST.inc` | No exchange with France; France covers its demand alone; objective up |

## Running

From the Balmorel root, as for the other scenarios:

```sh
cd ext_reference && bsub < investment.sh        # and the same for the four cases
```

Run `ext_reference` too: the earlier `base_dc_4` results cannot be used as the reference, because they have three years and a
different time set. To download results set `$Scenario` in `get_results.ps1` to the folder name, e.g. `ext_no_pv`.
Part 2 of `NOTEBOOK_Verification.ipynb` reads `<scenario>/model/results/MainResults.gdx` and checks the expected outcomes.

## Notes

- **Single year:** with only 2035 simulated, all new capacity has to be built in 2035 (no 2025 or 2030 investments), and transmission
  investment is possible (it starts in 2035, `XINVESTSTART` in `base/model/balopt.opt`).
- **Memory:** the job scripts request 4 GB per core (`-R "rusage[mem=4GB]"`, `-M 4GB`); `base_dc_4` uses 6 GB.
- **Step length:** `CHRONOHOUR = 168 / number of steps`, so the 24 steps of a week count as 7 h each in the equations that use it
  (EV, storage, ...), although the data are 3 h apart. Energy accounting is correct.
- **PV:** the full-load hours are zeroed and the `SOLE_VAR_T` profile is left as it is on purpose. With a non-zero profile the model still writes
  the dispatch equation (generation = full-load hours x capacity x profile / sum of profile = 0). With an all-zero profile the equation
  is dropped for the area, and new PV capacity would have no upper limit on its generation.
- **Onshore wind:** `GINVCOST(area, technology)` overrides the default `GDINVCOST0` in `GDATA`. The existing onshore units (`..._ONS_Existing_RG*`) have no
  investment cost and are not touched. The factor is `ONSWIND_CAPEX_FACTOR` in `GINVCOST.inc`.
- **France:** the 12 `XKFX` entries and 14 `XINVCOST` entries with FR as sending or receiving region are blanked.
- **Rebuilding:** `python make_extreme_cases.py` regenerates the five folders (it overwrites `ext_*`). To use other weeks, hours or years, change
  `SEASONS`, `TIME_FILE` or `YEARS` at the top of the script.
- **Checked:** every scenario compiles with GAMS 50 (`gams Balmorel a=c`, no new errors compared with `base_dc_4`), and the 12 + 14 France entries
  were checked against the base tables. The model has not been executed or solved locally (the data-preparation step alone ran for more than an
  hour), so confirm after the first cluster run that the quantities moved as expected (Part 2 of the notebook does this).
