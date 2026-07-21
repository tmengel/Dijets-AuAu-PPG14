#!/bin/bash

set -euo pipefail

real_path() {
    local rawpath="$1"
    local abspath=""

    if [[ "$rawpath" = /* ]]; then
        abspath="$rawpath"
    else
        abspath="$(cd "$(dirname "$rawpath")" && pwd)/$(basename "$rawpath")"
    fi

    abspath="${abspath/\/gpfs\/mnt\/gpfs02\/sphenix/\/sphenix}"
    abspath="${abspath/\/direct\/sphenix+tg+tg01/\/sphenix\/tg\/tg01}"
    abspath="${abspath/\/direct\/sphenix+u/\/sphenix\/u}"

    echo "$abspath"
}

PPG14_BASE="$(real_path "$(cd ../ && pwd)")"
echo "PPG14_BASE=$PPG14_BASE"

buildVersion="new"
installDir="$PPG14_BASE/install"
macroDir="$PPG14_BASE/macros"
macro="Fun4All_PPG14.C"
userName="$(whoami)"
condorBaseDir="$PPG14_BASE/condor"

TOTAL_SEGMENTS=15000
SEGMENTS_PER_JOB=5
NUM_JOBS=$(( TOTAL_SEGMENTS / SEGMENTS_PER_JOB ))

if (( TOTAL_SEGMENTS % SEGMENTS_PER_JOB != 0 )); then
    echo "Error: TOTAL_SEGMENTS must be divisible by SEGMENTS_PER_JOB"
    exit 1
fi

echo "buildVersion=$buildVersion"
echo "installDir=$installDir"
echo "userName=$userName"
echo "macroDir=$macroDir"
echo "macro=$macro"
echo "TOTAL_SEGMENTS=$TOTAL_SEGMENTS"
echo "SEGMENTS_PER_JOB=$SEGMENTS_PER_JOB"
echo "NUM_JOBS=$NUM_JOBS"

usage() {
    cat <<EOF
Usage:
  $0 <job_name> [output_dir]

Example:
  $0 sim_test /sphenix/tg/tg01/jets/${userName}/sim_test
EOF
}

if [ "$#" -ne 1 ]; then
    usage
    exit 1
fi

JOB_NAME="$1"
if [ -z "$JOB_NAME" ]; then
    echo "Error: job_name cannot be empty"
    usage
    exit 1
fi

# see if there is a second argument for output directory
if [ "$#" -ge 2 ]; then
    OUTDIR="$2"
else
    OUTDIR="/sphenix/tg/tg01/${userName}/${JOB_NAME}"
fi

if [ ! -d "$macroDir" ]; then
    echo "Error: macro directory not found: $macroDir"
    exit 1
fi

if [ ! -d "$installDir" ]; then
    echo "Error: install directory not found: $installDir"
    exit 1
fi

CONDOR_DIR="${condorBaseDir}/${JOB_NAME}"
INITIAL_DIR="$CONDOR_DIR"

mkdir -p \
    "${CONDOR_DIR}/logs" \
    "${CONDOR_DIR}/macros"

echo "Copying macros..."
find "$macroDir" -maxdepth 1 -name "*.C" -exec cp -f {} "${CONDOR_DIR}/macros/" \;

CONFIG_FILE="${CONDOR_DIR}/config.txt"

cat > "$CONFIG_FILE" <<EOF
int verbosity 0
int hijets_verbosity 0

int num_events -1
int run_number 31
int first_segment 0
int num_segments ${SEGMENTS_PER_JOB}

int jet_flag 10 

int is_overlay 0

int rescale_calo_flag 0
float upscale_calo_factor 1.0

int do_jets 1
int ue_flow_flag 0
float jet_R 0.3

int event_select 0

string cdbtag MDC2
string prodtag pythia8_Jet10_sHijing_0_20fm

vstring dsts DST_CALO_CLUSTER, DST_MBD_EPD, DST_GLOBAL, DST_TRUTH_G4HIT, DST_TRUTH_JET, DST_CALO_G4HIT
EOF

DRIVER_FILE="${CONDOR_DIR}/${JOB_NAME}.sh"
JOB_FILE="${CONDOR_DIR}/${JOB_NAME}.job"

cat > "$DRIVER_FILE" <<EOF
#!/bin/bash

PROCESS="\${1}"

initialDir="${INITIAL_DIR}"
configFile="${CONFIG_FILE}"

if [ ! -f "\$configFile" ]; then
    echo "Error: config file not found: \$configFile"
    exit 1
fi

MACRODIR="${CONDOR_DIR}/macros"
cd "\$MACRODIR" || exit 1

macro="${macro}"

INSTALLDIR="${installDir}"
source /opt/sphenix/core/bin/sphenix_setup.sh -n ${buildVersion}
source "\$OPT_SPHENIX/bin/setup_local.sh" \$INSTALLDIR

OUTDIR="${OUTDIR}"
mkdir -p "\$OUTDIR"

OUTFILE="\${OUTDIR}/${JOB_NAME}_\$(printf "%05d" "\$PROCESS").root"

ISEG=\$(( PROCESS * ${SEGMENTS_PER_JOB} ))
NSEGS=${SEGMENTS_PER_JOB}

root -l -b -q "\$macro(\"\$configFile\", \$ISEG, \$NSEGS, \"\$OUTFILE\")"
EXITCODE=\$?

if [ \$EXITCODE -ne 0 ]; then
    echo "Error: \${macro} failed for config \${configFile} with exit code \$EXITCODE"
    exit \$EXITCODE
fi

echo "Job \${PROCESS} completed successfully, output saved to \${OUTFILE}"
EOF

chmod +x "$DRIVER_FILE"

TEST_FILE="${CONDOR_DIR}/test.sh"

cat > "$TEST_FILE" <<EOF
#!/bin/bash
set -euo pipefail

cd "${INITIAL_DIR}"

./${JOB_NAME}.sh 0 "${CONFIG_FILE}"
EOF

chmod +x "$TEST_FILE"

cat > "$JOB_FILE" <<EOF
initialDir          = ${INITIAL_DIR}
Universe            = vanilla
Executable          = \$(initialDir)/${JOB_NAME}.sh
PeriodicHold        = (NumJobStarts>=1 && JobStatus == 1)
request_memory      = 4GB
batch_name          = ${JOB_NAME}
condorDir           = \$(initialDir)/logs/
Output              = \$(initialDir)/logs/\$(PROCESS).out
Error               = \$(initialDir)/logs/\$(PROCESS).err
Log                 = /tmp/condorjob-${userName}.log
Arguments           = "\$(PROCESS)"
Queue ${NUM_JOBS}
EOF

cat <<EOF

Created job directory:

  ${INITIAL_DIR}

Layout:

  ${INITIAL_DIR}/
    ${JOB_NAME}.job
    ${JOB_NAME}.sh
    config.txt
    test.sh
    logs/
    macros/

Generated:
  ${NUM_JOBS} jobs
  ${TOTAL_SEGMENTS} total segments
  ${SEGMENTS_PER_JOB} segments per job

Submit with:

  condor_submit ${JOB_FILE}

Test locally with:
    ${TEST_FILE}
EOF