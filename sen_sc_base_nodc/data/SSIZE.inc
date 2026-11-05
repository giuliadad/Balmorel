* SSIZE: scaling of a season's net inter-seasonal storage change to the full period it represents.
* = (weeks per season) x (168 h / hours covered by the T block). Together with CHRONOHOUR this keeps
* CHRONOHOUR x SSIZE consistent with IHOURSINST. Reduces to the base default 52/CARD(S) when the
* T block covers the whole week (e.g. every 3rd hour with CHRONOHOUR=3).
PARAMETER SSIZE(SSS)  'The number of Seasons that are aggregated into Season S';
SSIZE(S)=52/CARD(S)*168/SUM(T,CHRONOHOUR(S,T));
