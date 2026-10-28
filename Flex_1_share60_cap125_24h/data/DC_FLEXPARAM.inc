* Data-centre flexibility parameters (Senga et al. 2026): 60% shiftable load, capacity 1.25 x peak load, shifting horizon 24 h.
* The load in each time step can be reduced by up to DC_FLEXSHARE x L_t (down to 40% of L_t),
* and the load after shifting can never exceed the capacity C (QDCF_CAP: S - D <= C - L_t).
SCALAR DC_CAPFACTOR 'DC capacity C relative to DC demand L (C = DC_CAPFACTOR x L)' / 1.25 /;
SCALAR DC_FLEXSHARE 'Share s of DC demand that can be deferred (D <= s x L)' / 0.6 /;
SCALAR DC_SHIFTHORIZON 'Shifting horizon h in time steps (= hours at hourly T)' / 24 /;
