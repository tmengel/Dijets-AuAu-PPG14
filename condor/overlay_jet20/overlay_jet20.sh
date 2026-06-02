#!/bin/bash

PROCESS="${1}"

initialDir="/sphenix/user/tmengel/Dijets-AuAu-PPG14/condor/overlay_jet20"
configFile="/sphenix/user/tmengel/Dijets-AuAu-PPG14/condor/overlay_jet20/config.txt"

if [ ! -f "$configFile" ]; then
    echo "Error: config file not found: $configFile"
    exit 1
fi

MACRODIR="/sphenix/user/tmengel/Dijets-AuAu-PPG14/condor/overlay_jet20/macros"
cd "$MACRODIR" || exit 1

macro="Fun4All_PPG14.C"

INSTALLDIR="/sphenix/user/tmengel/Dijets-AuAu-PPG14/install"
source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source "$OPT_SPHENIX/bin/setup_local.sh" $INSTALLDIR

OUTDIR="/sphenix/tg/tg01/jets/tmengel/ppg14/overlay_jet20"
mkdir -p "$OUTDIR"

OUTFILE="${OUTDIR}/overlay_jet20_$(printf "%05d" "$PROCESS").root"

ISEG=$(( PROCESS * 5 ))
NSEGS=5

root -l -b -q "$macro(\"$configFile\", $ISEG, $NSEGS, \"$OUTFILE\")"
EXITCODE=$?

if [ $EXITCODE -ne 0 ]; then
    echo "Error: ${macro} failed for config ${configFile} with exit code $EXITCODE"
    exit $EXITCODE
fi

echo "Job ${PROCESS} completed successfully, output saved to ${OUTFILE}"
