ifd_evol config="./settings/config.ini"  outdir="../results/run1"

ifd_evol config="./settings/config.ini" G=10000 initial_comp=0.2 changerate=0 outdir="../results/nochange_inilow"
ifd_evol config="./settings/config.ini" G=10000 initial_comp=1.2 changerate=0 outdir="../results/nochange_inihigh"


