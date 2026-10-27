* Default data-centre flexibility parameters (Senga et al. 2026). Override with a local ../data/DC_FLEXPARAM.inc
SCALAR DC_CAPFACTOR    'DC capacity C relative to DC demand L (C = DC_CAPFACTOR x L)' / 1.25 /;
SCALAR DC_FLEXSHARE    'Share s of DC demand that can be deferred (D <= s x L)'      / 0.2 /;
SCALAR DC_SHIFTHORIZON 'Shifting horizon h in time steps (= hours at hourly T)'      / 24 /;




