#!/bin/bash

INPUTDIR="/sphenix/tg/tg01/commissioning/CaloCalibWG/bseidlitz/embed/jet20/OutDir*/"
OUTDIR="$(dirname $(readlink -f $0))"

# DST type prefix -> list file name mapping
# These match the inputs in Fun4All_run_sim.C
declare -A LISTMAP
LISTMAP["DST_TRUTH_G4HIT"]="g4hits.list"           # inputFile0, listfile[0]
LISTMAP["DST_CALO_"]="dst_calo_cluster.list"        # inputFile1, listfile[1]
LISTMAP["DST_GLOBAL_"]="dst_global.list"            # inputFile3, listfile[3]
LISTMAP["DST_TRUTH_JET_"]="dst_truth_jet.list"      # inputFile4, listfile[4]
LISTMAP["DST_CALOFITTING"]="dst_calofitting.list"   # pedestal/fitting overlay

for prefix in "${!LISTMAP[@]}"; do
    listfile="${OUTDIR}/${LISTMAP[$prefix]}"
    ls ${INPUTDIR}/${prefix}*.root 2>/dev/null | sort > "${listfile}"
    nfiles=$(wc -l < "${listfile}")
    echo "${LISTMAP[$prefix]}: ${nfiles} files"
done
