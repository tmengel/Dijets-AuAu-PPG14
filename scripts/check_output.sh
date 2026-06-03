#!/bin/bash
NAME=${1:-"overlay_jet20"}

INDIR="/sphenix/tg/tg01/jets/tmengel/ppg14/${NAME}"

LISTDIR="/sphenix/user/tmengel/Dijets-AuAu-PPG14/condor/hadding/lists"
THISDIR="${LISTDIR}/${NAME}"
mkdir -p "$LISTDIR"
mkdir -p "$THISDIR"
rm -f "${THISDIR}"/*.list

master="${THISDIR}/all.list"
find "$INDIR" -type f -name "*.root" | sort -u > "${master}"

split -d -a 3 -l 1000 \
  --additional-suffix=".list" \
  "${master}" \
  "${THISDIR}/${NAME}-1k-"

rm -f "${master}"

master="${LISTDIR}/${NAME}_args.list"
if [[ -f "$master" ]]; then
  rm -f "$master"
fi

# touch "$master"

find "$THISDIR" -type f -name "*.list" | sort -u > "$master"

echo "Argument list for $NAME written to $master"

