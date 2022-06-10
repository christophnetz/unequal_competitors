#ifd_evol config="./settings/config.ini"  outdir="../results/run1"

ifd_evol config="./settings/config.ini" G=10000 initial_comp=0.2 mutation_shape=0.01 base_intake=0.1 changerate=0 outdir="../results/nochange_inilow"
ifd_evol config="./settings/config.ini" G=10000 initial_comp=1.2 mutation_shape=0.01 base_intake=0.1 changerate=0 outdir="../results/nochange_inihigh"


