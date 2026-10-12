* DCFLEX internal parameters and sets (Senga et al. 2026)

* Baseline DC load L_t (MW): same expression as the DATACENTER term in QEEQ
PARAMETER IDCF_L(Y,RRR,S,T) 'Baseline data-centre load L_t (MW)';
IDCF_L(Y,IR,S,T)$(IDE_SUMST(IR,'DATACENTER') GT 0)
   = DE(Y,IR,'DATACENTER') * DE_VAR_T(IR,'DATACENTER',S,T) / IDE_SUMST(IR,'DATACENTER');

* DC capacity C (MW): DC_CAPFACTOR x peak baseline load (= x average load for a flat profile)
PARAMETER IDCF_C(Y,RRR) 'Data-centre capacity C (MW)';
IDCF_C(Y,IR) = DC_CAPFACTOR * SMAX((S,T), IDCF_L(Y,IR,S,T));

* Regions and years with flexible DC load
SET IDCF(Y,RRR) 'Regions with flexible data-centre load';
IDCF(Y,IR) = YES$(IDCF_C(Y,IR) GT 0);

* Forward window: ITALIAS lies 1..h steps after T (circular within the season)
SET IDCF_WINFWD(T,ITALIAS) 'Hours within the shifting horizon after T';
IDCF_WINFWD(T,ITALIAS) = YES$(     mod(ord(ITALIAS)-ord(T)+card(T), card(T)) GE 1
                               AND mod(ord(ITALIAS)-ord(T)+card(T), card(T)) LE DC_SHIFTHORIZON );

ABORT$(DC_SHIFTHORIZON GE CARD(T)) 'DCFLEX: DC_SHIFTHORIZON must be smaller than the number of time steps per season';
