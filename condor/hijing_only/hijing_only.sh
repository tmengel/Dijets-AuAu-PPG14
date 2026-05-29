#!/bin/bash

PROCESS="${1}"

initialDir="/sphenix/user/tmengel/Dijets-AuAu-PPG14/condor/hijing_only"

configFile="${initialDir}/hijing_only_config.txt"
if [ ! -f "$configFile" ]; then
    echo "Error: Config file $configFile not found"
    exit 1
fi

MACRODIR="${initialDir}/macros"
cd "$MACRODIR" || exit 1

macro="Fun4All_HijingOnly.C"

startingSegment=$(( PROCESS * 5 ))
numSegments=5

INSTALLDIR="${initialDir}/install"
source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source $OPT_SPHENIX/bin/setup_local.sh $INSTALLDIR

OUTDIR="/sphenix/tg/tg01/jets/tmengel/ppg14/hijing_only/output/"
mkdir -p "$OUTDIR"

OUTFILE="${OUTDIR}/hijing_only_$(printf "%05d" $PROCESS)_v1.root"

root -l -b -q "$macro(\"$configFile\", $startingSegment, $numSegments, \"$OUTFILE\")"
EXITCODE=$?

if [ $EXITCODE -ne 0 ]; then
    echo "Error: ${macro} failed for config ${configFile} with exit code $EXITCODE"
    exit $EXITCODE
fi

echo "Job ${PROCESS} completed successfully, output saved to ${OUTFILE}"
