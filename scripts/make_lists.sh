#!/bin/bash
jetSample="${1:-jet20}"
INPUTDIR="/sphenix/tg/tg01/commissioning/CaloCalibWG/bseidlitz/embed/${jetSample}/OutDir*/"
OUTDIR=${2:-$(dirname $(readlink -f $0))}
mkdir -p "${OUTDIR}"

declare -A LISTMAP
LISTMAP["DST_CALO_"]="dst_calo_cluster.list"        
LISTMAP["DST_GLOBAL_"]="dst_global.list"            
LISTMAP["DST_TRUTH_JET_"]="dst_truth_jet.list"    
  
for prefix in "${!LISTMAP[@]}"; do
    listfile="${OUTDIR}/${LISTMAP[$prefix]}"
    ls ${INPUTDIR}/${prefix}*.root 2>/dev/null | sort > "${listfile}"
    nfiles=$(wc -l < "${listfile}")
    echo "${LISTMAP[$prefix]}: ${nfiles} files"
done
