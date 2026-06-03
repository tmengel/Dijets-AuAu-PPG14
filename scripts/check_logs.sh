#!/bin/bash

# Usage:
#   ./clean_good_jobs.sh /path/to/logs
#
# Example:
#   ./clean_good_jobs.sh overlay_jet10/logs

logdir="${1:-}"

if [[ -z "$logdir" || ! -d "$logdir" ]]; then
    echo "Usage: $0 /path/to/logs" >&2
    exit 1
fi

bad_jobs=()
good_jobs=0
missing_jobs=0

shopt -s nullglob

for outfile in "$logdir"/*.out; do
    base="$(basename "$outfile" .out)"
    errfile="$logdir/$base.err"

    # Require numeric job id, e.g. 0.out, 1000.out
    if [[ ! "$base" =~ ^[0-9]+$ ]]; then
        echo "BAD: $outfile has non-numeric job id"
        bad_jobs+=("$base")
        continue
    fi

    job="$base"

    if [[ ! -f "$errfile" ]]; then
        echo "BAD: job $job missing err file: $errfile"
        bad_jobs+=("$job")
        ((missing_jobs++))
        continue
    fi

    # Check .err is empty
    if [[ -s "$errfile" ]]; then
        echo "BAD: job $job has non-empty err file: $errfile"
        bad_jobs+=("$job")
        continue
    fi

    # Check final line of .out has exact success form
    last_line="$(tail -n 1 "$outfile")"

    expected_regex="^Job ${job} completed successfully, output saved to .+$"

    if [[ ! "$last_line" =~ $expected_regex ]]; then
        echo "BAD: job $job has bad .out ending"
        echo "     last line: $last_line"
        bad_jobs+=("$job")
        continue
    fi

    # Good job: remove log files
    rm -f -- "$outfile" "$errfile"
    # echo "GOOD: job $job cleaned"
    ((good_jobs++))
done

# Also flag .err files without matching .out files
for errfile in "$logdir"/*.err; do
    base="$(basename "$errfile" .err)"
    outfile="$logdir/$base.out"

    if [[ ! -f "$outfile" ]]; then
        echo "BAD: job $base missing out file: $outfile"
        bad_jobs+=("$base")
        ((missing_jobs++))
    fi
done

echo
echo "Summary:"
echo "  good jobs cleaned: $good_jobs"
echo "  bad jobs found:    ${#bad_jobs[@]}"
echo "  missing pairs:     $missing_jobs"

if (( ${#bad_jobs[@]} > 0 )); then
    echo
    echo "Bad job ids:"
    printf '  %s\n' "${bad_jobs[@]}" | sort -n
    exit 2
fi
