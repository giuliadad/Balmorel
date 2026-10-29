* CHRONOHOUR: real length (hours) of each time step.
* Local override of the base default 168/card(T), which assumes T is sampled evenly across the
* whole 168 h week. This scenario uses consecutive hourly time steps (T073-T144), so each step = 1 h.
* Week scaling for inter-seasonal storage is moved to SSIZE.inc (see there).
PARAMETER CHRONOHOUR(SSS,TTT);
CHRONOHOUR(S,T)=1;
