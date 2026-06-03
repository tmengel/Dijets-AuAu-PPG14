#!/bin/bash

LIST="${1}"
NAME=$(basename "${LIST}" .list)
BATCH_NAME="${2}"

MACRODIR="/sphenix/user/tmengel/Dijets-AuAu-PPG14/condor/hadding/macros"
cd "$MACRODIR" || exit 1

INSTALLDIR="/sphenix/user/tmengel/Dijets-AuAu-PPG14/install"
source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source "$OPT_SPHENIX/bin/setup_local.sh" $INSTALLDIR

OUTDIR="/sphenix/tg/tg01/jets/tmengel/ppg14/hadding/${BATCH_NAME}"
mkdir -p "$OUTDIR"
OUTFILE="${OUTDIR}/${NAME}.root"

hadd -k -f "${OUTFILE}" $(cat "${LIST}")
EXITCODE=$?

if [ $EXITCODE -ne 0 ]; then
    echo "Error: hadd command failed with exit code $EXITCODE."
    exit $EXITCODE
fi
echo "Successfully created ${OUTFILE} from files listed in ${LIST}."