#!/bin/bash

JOB_NAME="hijing_only"
MACRODIR="/sphenix/user/tmengel/Dijets-AuAu-PPG14/macros/"
SRCDIR="/sphenix/user/tmengel/Dijets-AuAu-PPG14/src/dijetana/"
OUTDIR="/sphenix/tg/tg01/jets/tmengel/ppg14/${JOB_NAME}/output/"
MACRO="Fun4All_HijingOnly.C"

bash configure_jobs.sh "${JOB_NAME}" "${MACRODIR}" "${SRCDIR}" "${OUTDIR}" "${MACRO}"

