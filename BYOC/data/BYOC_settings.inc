* File BYOC_settings.inc
* Single place to configure the BYOC (Bring Your Own Clean-energy) scenario.
* It is included by BYOC/data/GKFX.inc, GMAXF.inc and AGKN.inc, so change the results path here only.
* All paths are relative to BYOC/model (the folder the model is run from).

* Results of the reference run (base_nodc) that BYOC is built on. When base_nodc is rerun, change the folder here.
$if not setglobal BYOC_REF_DIR       $setglobal BYOC_REF_DIR       ../../base_nodc/model

* Raw end-of-model gdx of the reference run: gives the capacity that is made exogenous (GKFX, VGKNACCUMNET, VDECOM_EXO_ACCUM).
* The processed G_CAP_YCRAF in MainResults is not used for capacity, because it is rescaled (heat output basis for CHP,
* GW instead of MW, storage power instead of volume) and would have to be inverted.
$if not setglobal BYOC_CAPACITY_GDX  $setglobal BYOC_CAPACITY_GDX  %BYOC_REF_DIR%/all_endofmodel_fullweek.gdx

* Processed results of the reference run: gives the fuel use (F_CONS_YCRA, TWh) that is used as an upper limit.
$if not setglobal BYOC_FUEL_GDX      $setglobal BYOC_FUEL_GDX      %BYOC_REF_DIR%/MainResults_fullweek.gdx

* Geographical level of the fuel limit: AREA, REGION or COUNTRY.
* AREA is closest to "no plant runs more than in the reference", COUNTRY is the loosest (fuel may move between regions).
$if not setglobal BYOC_FUELCAP_LEVEL $setglobal BYOC_FUELCAP_LEVEL REGION

* Relative tolerance added to the fuel limit (1e-4 = 0.01 %), so that the reference dispatch itself stays feasible
* in spite of solver tolerances.
$if not setglobal BYOC_FUELCAP_TOL   $setglobal BYOC_FUELCAP_TOL   1e-4

* Fuels that are limited to the use in the reference run (everything that is neither renewable nor an energy carrier).
* Edit this list to change what counts as "non-renewable". Renewables as defined for BYOC:
*   wind, solar (thermal and PV), geothermal, ambient energy (heat pumps), tidal/wave/ocean, hydro,
*   biomass, landfill gas, sewage-treatment-plant gas and biogas.
* Fuels that appear in neither list (ELECTRIC, HEAT, WASTEHEAT, SURPLUS_H, GH2FUEL, THERMGAS, DUMMY) are carriers
* and are neither limited nor blocked.
$if not setglobal BYOC_NONRENEWABLE_FUELS $setglobal BYOC_NONRENEWABLE_FUELS NUCLEAR,NATGAS,COAL,LIGNITE,FUELOIL,LIGHTOIL,HEAVYFUELOIL,ORIMULSION,SHALE,PEAT,MUNIWASTE,HAZARDOUSWASTE,LPG,LNG,OTHERGAS,RETORTGAS,COKE,COAL_EXETS,NATGAS_EXETS,FUELOIL_EXETS,LPG_EXETS,HEAVYFUELOIL_EXETS,COAL_CCS,NG_CCS
