#!/bin/bash
# =============================================================================
# run_kgs_batch.sh
# Run KGS hierarchy analysis on a list of PDB files.
#
# Usage:
#   ./run_kgs_batch.sh <pdb_dir_or_list> [options passed to run_kgs_analysis.sh]
#
# Examples:
#   # Run on all PDBs in a directory
#   ./run_kgs_batch.sh ./pdbs/
#
#   # Run on specific files listed in a text file (one PDB path per line)
#   ./run_kgs_batch.sh protein_list.txt
#
#   # Run on all PDBs with chain A extraction and 20 samples
#   ./run_kgs_batch.sh ./pdbs/ --chain A --samples 20
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
KGS_SCRIPT="$SCRIPT_DIR/run_kgs_analysis.sh"

if [ ! -f "$KGS_SCRIPT" ]; then
    echo "ERROR: Cannot find run_kgs_analysis.sh at $KGS_SCRIPT"
    exit 1
fi

INPUT="$1"
shift  # remaining args passed through to run_kgs_analysis.sh

if [ -z "$INPUT" ]; then
    echo "Usage: $0 <pdb_directory | file_list.txt> [extra options]"
    exit 1
fi

# Build list of PDB files
PDB_FILES=()
if [ -d "$INPUT" ]; then
    # Directory: find all .pdb files
    while IFS= read -r -d '' f; do
        PDB_FILES+=("$f")
    done < <(find "$INPUT" -name "*.pdb" -print0 | sort -z)
elif [ -f "$INPUT" ]; then
    if [[ "$INPUT" == *.pdb ]]; then
        # Single PDB file
        PDB_FILES+=("$INPUT")
    else
        # Text file with list of PDB paths
        while IFS= read -r line; do
            [ -z "$line" ] && continue
            [[ "$line" == \#* ]] && continue
            PDB_FILES+=("$line")
        done < "$INPUT"
    fi
fi

if [ ${#PDB_FILES[@]} -eq 0 ]; then
    echo "ERROR: No PDB files found in $INPUT"
    exit 1
fi

echo "=============================================="
echo "KGS Batch Analysis"
echo "=============================================="
echo "Found ${#PDB_FILES[@]} PDB files"
echo "Extra options: $@"
echo "=============================================="
echo ""

# Track results
SUCCESSES=0
FAILURES=0
FAILED_LIST=""

for pdb in "${PDB_FILES[@]}"; do
    BASENAME=$(basename "$pdb" .pdb)
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Processing: $BASENAME"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    if bash "$KGS_SCRIPT" "$pdb" "$@"; then
        SUCCESSES=$((SUCCESSES + 1))
        echo "[OK] $BASENAME completed successfully"
    else
        FAILURES=$((FAILURES + 1))
        FAILED_LIST="$FAILED_LIST $BASENAME"
        echo "[FAIL] $BASENAME failed"
    fi
done

echo ""
echo "=============================================="
echo "Batch Summary"
echo "=============================================="
echo "Total:     ${#PDB_FILES[@]}"
echo "Successes: $SUCCESSES"
echo "Failures:  $FAILURES"
if [ $FAILURES -gt 0 ]; then
    echo "Failed:   $FAILED_LIST"
fi
echo "=============================================="