#!/bin/bash
set -euo pipefail

usage() {
    cat <<EOF
Usage:
  $0 <job_name> <macro_source_dir> <src_dir> <output_dir> <macro_name>

Example:
  $0 sim_test /path/to/macros /path/to/src /sphenix/user/tmengel/output Fun4All_Sim.C
EOF
}

if [ "$#" -ne 5 ]; then
    usage
    exit 1
fi

JOB_NAME="$1"
MACRO_SOURCE_DIR="$2"
SRC_SOURCE_DIR="$3"
OUTDIR="$4"
MACRO_NAME="$5"
TOTAL_SEGMENTS=15000
SEGMENTS_PER_JOB=5
NUM_JOBS=$(( TOTAL_SEGMENTS / SEGMENTS_PER_JOB ))

if [ $(( TOTAL_SEGMENTS % SEGMENTS_PER_JOB )) -ne 0 ]; then
    echo "Error: TOTAL_SEGMENTS must be divisible by SEGMENTS_PER_JOB"
    exit 1
fi

if [ ! -d "$MACRO_SOURCE_DIR" ]; then
    echo "Error: macro source directory not found: $MACRO_SOURCE_DIR"
    exit 1
fi

if [ ! -d "$SRC_SOURCE_DIR" ]; then
    echo "Error: src source directory not found: $SRC_SOURCE_DIR"
    exit 1
fi

mkdir -p "$JOB_NAME"
INITIAL_DIR="$(cd "$JOB_NAME" && pwd)"

mkdir -p \
    "${INITIAL_DIR}/logs" \
    "${INITIAL_DIR}/macros" \
    "${INITIAL_DIR}/src" \
    "${INITIAL_DIR}/install"

echo "Copying macros..."
rsync -a --delete "${MACRO_SOURCE_DIR}/" "${INITIAL_DIR}/macros/"

echo "Copying src..."
rsync -a --delete "${SRC_SOURCE_DIR}/" "${INITIAL_DIR}/src/"

echo "Generating ${NUM_JOBS} config files..."

CONFIG_FILE="${INITIAL_DIR}/${JOB_NAME}_config.txt"
cat > "$CONFIG_FILE" <<EOF
# config
int verbosity 0
int hijets_verbosity 0

int num_events -1
int run_number 31
int first_segment 0
int num_segments 5

# for hijing + pythia pT hat samps
int jet_flag 0 

int save_full_calo 1

int rescale_calo_flag 0
float upscale_calo_factor 1.0

# reco jets
int do_jets 0
int ue_flow_flag 0
float jet_R 0.3

int event_select 0

string cdbtag MDC2
string prodtag sHijing_0_20fm

string outfile dummy.root

vstring dsts DST_CALO_CLUSTER, DST_MBD_EPD, DST_GLOBAL, G4Hits
EOF

DRIVER_FILE="${INITIAL_DIR}/${JOB_NAME}.sh"
JOB_FILE="${INITIAL_DIR}/${JOB_NAME}.job"

cat > "$DRIVER_FILE" <<EOF
#!/bin/bash

PROCESS="\${1}"

initialDir="${INITIAL_DIR}"

configFile="\${initialDir}/${JOB_NAME}_config.txt"
if [ ! -f "\$configFile" ]; then
    echo "Error: Config file \$configFile not found"
    exit 1
fi

MACRODIR="\${initialDir}/macros"
cd "\$MACRODIR" || exit 1

macro="${MACRO_NAME}"

startingSegment=\$(( PROCESS * ${SEGMENTS_PER_JOB} ))
numSegments=${SEGMENTS_PER_JOB}

INSTALLDIR="\${initialDir}/install"
source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source \$OPT_SPHENIX/bin/setup_local.sh \$INSTALLDIR

OUTDIR="${OUTDIR}"
mkdir -p "\$OUTDIR"

OUTFILE="\${OUTDIR}/${JOB_NAME}_\$(printf "%05d" \$PROCESS).root"

root -l -b -q "\$macro(\"\$configFile\", \$startingSegment, \$numSegments, \"\$OUTFILE\")"
EXITCODE=\$?

if [ \$EXITCODE -ne 0 ]; then
    echo "Error: \${macro} failed for config \${configFile} with exit code \$EXITCODE"
    exit \$EXITCODE
fi

echo "Job \${PROCESS} completed successfully, output saved to \${OUTFILE}"
EOF

chmod +x "$DRIVER_FILE"

cat > "$JOB_FILE" <<EOF
initialDir          = ${INITIAL_DIR}
Universe            = vanilla
Executable          = \$(initialDir)/${JOB_NAME}.sh
PeriodicHold        = (NumJobStarts>=1 && JobStatus == 1)
request_memory      = 3GB
batch_name          = ${JOB_NAME}
condorDir           = \$(initialDir)/logs/
Output              = \$(initialDir)/logs/\$(PROCESS).out
Error               = \$(initialDir)/logs/\$(PROCESS).err
Log                 = /tmp/condorjob-tmengel.log
Arguments           = "\$(PROCESS)"
Queue ${NUM_JOBS}
EOF

cat <<EOF

Created job directory:

  ${INITIAL_DIR}

Layout:

  ${INITIAL_DIR}/
    ${JOB_NAME}_config.txt
    ${JOB_NAME}.job
    ${JOB_NAME}.sh
    logs/
    macros/
    src/
    install/

Generated:
  ${NUM_JOBS} configs
  ${TOTAL_SEGMENTS} total segments
  ${SEGMENTS_PER_JOB} segments per job

Submit with:

  condor_submit ${JOB_FILE}

EOF