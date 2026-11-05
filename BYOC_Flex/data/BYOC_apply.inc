* File BYOC_apply.inc
* BYOC (Bring Your Own Clean-energy) scenario: data centres are served by new renewables and storage only.
* Included at the end of the data section by BYOC/model/bb4datainc.inc, i.e. after all data files and all
* add-on additions (GKFX, AGKN) have been read, and before the model derives IAGKFX, IAGKN and IGMAXF from them.
*
* 1. Capacity of the reference run (base_nodc), net of investments and decommissioning, becomes exogenous (GKFX).
* 2. New investments (AGKN) are blocked for technologies that use a non-renewable fuel.
*    Wind, solar, biomass, storage, heat pumps etc. stay investable. Transmission cannot be invested in (TransInvest NO in balopt.opt).
* 3. The annual use of every non-renewable fuel is limited to what the reference run used (GMAXF, constraint "<=").
*
* All settings (results path, level of the fuel limit, list of non-renewable fuels) are in BYOC_settings.inc.

$include '../data/BYOC_settings.inc'

*-------------------------------------------------------------------------------
* 1. Exogenous capacity from the reference run
*-------------------------------------------------------------------------------
* Net capacity in year Y of the reference run = GKFX + VGKNACCUMNET - VDECOM_EXO_ACCUM.
* This is the split that the reference run itself reports as EXOGENOUS + ENDOGENOUS + DECOMMISSIONING in G_CAP_YCRAF.
* GKFX is in MW, except for storages where it is MWh. VGKNACCUMNET follows the same convention.
PARAMETER BYOC_GKFX_REF(YYY,AAA,GGG)  'GKFX of the reference run at the end of the run (MW, storages MWh)';
PARAMETER BYOC_GKFX_NEW(YYY,AAA,GGG)  'Net capacity of the reference run, used as GKFX in this scenario (MW, storages MWh)';
VARIABLE  BYOC_VGKNACCUMNET(YYY,AAA,GGG) 'VGKNACCUMNET of the reference run';
VARIABLE  BYOC_VDECOM_EXO_ACCUM(YYY,AAA,GGG) 'VDECOM_EXO_ACCUM of the reference run';

execute_load '%BYOC_CAPACITY_GDX%', BYOC_GKFX_REF=GKFX, BYOC_VGKNACCUMNET=VGKNACCUMNET, BYOC_VDECOM_EXO_ACCUM=VDECOM_EXO_ACCUM;

BYOC_GKFX_NEW(Y,AAA,GGG) = BYOC_GKFX_REF(Y,AAA,GGG) + BYOC_VGKNACCUMNET.L(Y,AAA,GGG) - BYOC_VDECOM_EXO_ACCUM.L(Y,AAA,GGG);
* Remove numerical noise and negative rounding residues
BYOC_GKFX_NEW(Y,AAA,GGG)$(BYOC_GKFX_NEW(Y,AAA,GGG) LT 1E-6) = 0;

* Every simulated year must be present in the reference run
SET BYOC_NOYEAR(YYY) 'Simulated years without capacity in the reference run';
BYOC_NOYEAR(Y)$(SUM((AAA,GGG), BYOC_GKFX_NEW(Y,AAA,GGG)) EQ 0) = YES;
ABORT$CARD(BYOC_NOYEAR) "BYOC: no capacity found in the reference run (BYOC_CAPACITY_GDX) for these simulated years:", BYOC_NOYEAR;

* The reference capacity replaces all exogenous capacity of the simulated years (it already contains the add-on capacities).
GKFX(Y,AAA,GGG) = BYOC_GKFX_NEW(Y,AAA,GGG);

*-------------------------------------------------------------------------------
* 2. No new investments in technologies fuelled by non-renewable fuels
*-------------------------------------------------------------------------------
SET BYOC_NONRENEWABLE(FFF) 'Fuels that are limited to the use in the reference run and blocked for new investments' / %BYOC_NONRENEWABLE_FUELS% /;
SET BYOC_BLOCKED(GGG)      'Technologies with a non-renewable fuel (no new investments)';

* Same relation as IGF(G,FFF) in Balmorelbb4.inc
BYOC_BLOCKED(G) = YES$SUM(FFF$(BYOC_NONRENEWABLE(FFF) AND (GDATA(G,'GDFUEL') EQ FDATA(FFF,'FDACRONYM'))), 1);
AGKN(AAA,G)$BYOC_BLOCKED(G) = NO;

*-------------------------------------------------------------------------------
* 3. Fuel use of non-renewable fuels limited to the reference run
*-------------------------------------------------------------------------------
$ifi not %GMAXF_DOL%==YYY_CCCRRRAAA_FFF $abort "BYOC: GMAXF_DOL must be YYY_CCCRRRAAA_FFF"
$ifi not %BYOC_FUELCAP_LEVEL%==AREA    $ifi not %BYOC_FUELCAP_LEVEL%==REGION    $ifi not %BYOC_FUELCAP_LEVEL%==COUNTRY    $abort "BYOC: BYOC_FUELCAP_LEVEL must be AREA, REGION or COUNTRY"

* Annual fuel use of the reference run: F_CONS_YCRA(Y,C,RRR,AAA,G,FFF,TECH_TYPE,UNITS) in TWh
SET BYOC_UNIT 'Units label in F_CONS_YCRA' / TWh /;
ALIAS(BYOC_U1,*);
ALIAS(BYOC_U2,*);
ALIAS(BYOC_U3,*);
ALIAS(BYOC_U4,*);
PARAMETER BYOC_FCONS(*,*,*,*,*,*,*,*) 'Fuel consumption of the reference run (TWh)';
$gdxin '%BYOC_FUEL_GDX%'
$load BYOC_FCONS=F_CONS_YCRA
$gdxin

PARAMETER BYOC_FUELUSE(YYY,CCCRRRAAA,FFF) 'Fuel use of the reference run at the chosen level (GJ)';
SET       BYOC_HASFUEL(YYY,CCCRRRAAA,FFF) 'Fuel is used by exogenous capacity at the chosen level';
PARAMETER BYOC_CAP(YYY,CCCRRRAAA,FFF)     'New upper limit on fuel use (GJ)';

* GJ = 3.6E6 * TWh. The sums over the aliases of the universe only collapse the dimensions that are not used at the chosen level.
$ifi %BYOC_FUELCAP_LEVEL%==AREA     BYOC_FUELUSE(Y,AAA,FFF)$BYOC_NONRENEWABLE(FFF) = 3.6E6*SUM((BYOC_U1,BYOC_U2,BYOC_U3,BYOC_U4,BYOC_UNIT), BYOC_FCONS(Y,BYOC_U1,BYOC_U2,AAA,BYOC_U3,FFF,BYOC_U4,BYOC_UNIT));
$ifi %BYOC_FUELCAP_LEVEL%==REGION   BYOC_FUELUSE(Y,RRR,FFF)$BYOC_NONRENEWABLE(FFF) = 3.6E6*SUM((BYOC_U1,BYOC_U2,BYOC_U3,BYOC_U4,BYOC_UNIT), BYOC_FCONS(Y,BYOC_U1,RRR,BYOC_U2,BYOC_U3,FFF,BYOC_U4,BYOC_UNIT));
$ifi %BYOC_FUELCAP_LEVEL%==COUNTRY  BYOC_FUELUSE(Y,CCC,FFF)$BYOC_NONRENEWABLE(FFF) = 3.6E6*SUM((BYOC_U1,BYOC_U2,BYOC_U3,BYOC_U4,BYOC_UNIT), BYOC_FCONS(Y,CCC,BYOC_U1,BYOC_U2,BYOC_U3,FFF,BYOC_U4,BYOC_UNIT));

* Where exogenous capacity burns the fuel, the limit must also exist if the fuel use was zero in the reference run.
* A zero limit would be read as "no limit", so EPS is used (the model tests the limit with $IGMAXF, which is true for EPS).
SET BYOC_AREAFUEL(YYY,AAA,FFF);
BYOC_AREAFUEL(Y,AAA,FFF)$BYOC_NONRENEWABLE(FFF) = YES$SUM(G$(GKFX(Y,AAA,G) AND (GDATA(G,'GDFUEL') EQ FDATA(FFF,'FDACRONYM'))), 1);
$ifi %BYOC_FUELCAP_LEVEL%==AREA     BYOC_HASFUEL(Y,AAA,FFF)$BYOC_AREAFUEL(Y,AAA,FFF) = YES;
$ifi %BYOC_FUELCAP_LEVEL%==REGION   BYOC_HASFUEL(Y,RRR,FFF)$SUM(AAA$(RRRAAA(RRR,AAA) AND BYOC_AREAFUEL(Y,AAA,FFF)), 1) = YES;
$ifi %BYOC_FUELCAP_LEVEL%==COUNTRY  BYOC_HASFUEL(Y,CCC,FFF)$SUM((RRR,AAA)$(CCCRRR(CCC,RRR) AND RRRAAA(RRR,AAA) AND BYOC_AREAFUEL(Y,AAA,FFF)), 1) = YES;

BYOC_CAP(Y,CCCRRRAAA,FFF)$(BYOC_HASFUEL(Y,CCCRRRAAA,FFF) OR BYOC_FUELUSE(Y,CCCRRRAAA,FFF)) = EPS;
BYOC_CAP(Y,CCCRRRAAA,FFF)$(BYOC_FUELUSE(Y,CCCRRRAAA,FFF) GT 0) = BYOC_FUELUSE(Y,CCCRRRAAA,FFF)*(1+%BYOC_FUELCAP_TOL%);

* Combine with limits that already exist in GMAXF (for example Estonian oil shale): the tighter limit applies.
* An existing EPS limit is kept, since EPS counts as zero in the comparison.
GMAXF(Y,CCCRRRAAA,FFF)$(BYOC_CAP(Y,CCCRRRAAA,FFF) AND GMAXF(Y,CCCRRRAAA,FFF) AND (BYOC_CAP(Y,CCCRRRAAA,FFF) LT GMAXF(Y,CCCRRRAAA,FFF))) = BYOC_CAP(Y,CCCRRRAAA,FFF);
GMAXF(Y,CCCRRRAAA,FFF)$(BYOC_CAP(Y,CCCRRRAAA,FFF) AND NOT GMAXF(Y,CCCRRRAAA,FFF)) = BYOC_CAP(Y,CCCRRRAAA,FFF);

* Summary for the listing (totals only)
PARAMETER BYOC_SUMMARY(YYY,*) 'BYOC totals per simulated year';
BYOC_SUMMARY(Y,'GKFX total (GW or GWh)')   = SUM((AAA,GGG), GKFX(Y,AAA,GGG))/1000;
BYOC_SUMMARY(Y,'Fuel limits (number)')     = SUM((CCCRRRAAA,FFF)$BYOC_CAP(Y,CCCRRRAAA,FFF), 1);
BYOC_SUMMARY(Y,'Fuel limits (PJ)')         = SUM((CCCRRRAAA,FFF)$BYOC_CAP(Y,CCCRRRAAA,FFF), BYOC_CAP(Y,CCCRRRAAA,FFF))/1E6;
BYOC_SUMMARY(Y,'Reference fuel use (PJ)')  = SUM((CCCRRRAAA,FFF), BYOC_FUELUSE(Y,CCCRRRAAA,FFF))/1E6;
DISPLAY BYOC_SUMMARY;
