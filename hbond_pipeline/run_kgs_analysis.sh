#!/bin/bash
# =============================================================================
# run_kgs_analysis.sh
# Automates KGS hierarchy analysis for a given protein PDB file.
# 
# Usage:
#   ./run_kgs_analysis.sh <input.pdb> [options]
#
# Example:
#   ./run_kgs_analysis.sh 8Q3L.pdb
#   ./run_kgs_analysis.sh 8Q3L.pdb --chain A --samples 10 --stepSize 0.1
#   ./run_kgs_analysis.sh 8Q3L.pdb --skip-reduce --skip-prepare
#
# Requirements:
#   - phenix.reduce (from PHENIX suite)
#   - kgs_prepare.py (from KGS)
#   - kgs_hierarchy (from KGS)
# =============================================================================

set -e  # Exit on error

# ── Defaults ──
SAMPLES=10
STEP_SIZE=0.1
COLLISION_FACTOR=0.75
SVD_CUTOFF=1.0e-12
SAVE_DATA=3          # save everything (pdb, q, stats)
COLLISION_CHECK="all"
CHAIN=""
SKIP_REDUCE=false
SKIP_PREPARE=false
OUTPUT_BASE="kgs_output"

# ── Parse arguments ──
INPUT_PDB=""
while [[ $# -gt 0 ]]; do
    case $1 in
        --chain)        CHAIN="$2"; shift 2 ;;
        --samples)      SAMPLES="$2"; shift 2 ;;
        --stepSize)     STEP_SIZE="$2"; shift 2 ;;
        --collisionFactor) COLLISION_FACTOR="$2"; shift 2 ;;
        --svdCutoff)    SVD_CUTOFF="$2"; shift 2 ;;
        --saveData)     SAVE_DATA="$2"; shift 2 ;;
        --output)       OUTPUT_BASE="$2"; shift 2 ;;
        --skip-reduce)  SKIP_REDUCE=true; shift ;;
        --skip-prepare) SKIP_PREPARE=true; shift ;;
        -*)             echo "Unknown option: $1"; exit 1 ;;
        *)              INPUT_PDB="$1"; shift ;;
    esac
done

if [ -z "$INPUT_PDB" ]; then
    echo "Usage: $0 <input.pdb> [options]"
    echo ""
    echo "Options:"
    echo "  --chain <A|B|X|...>    Extract specific chain before processing"
    echo "  --samples <int>        Number of samples (default: $SAMPLES)"
    echo "  --stepSize <float>     Step size for sampling (default: $STEP_SIZE)"
    echo "  --collisionFactor <f>  VdW collision factor (default: $COLLISION_FACTOR)"
    echo "  --svdCutoff <float>    SVD cutoff (default: $SVD_CUTOFF)"
    echo "  --saveData <0-3>       Save level (default: $SAVE_DATA)"
    echo "  --output <dir>         Output base directory (default: $OUTPUT_BASE)"
    echo "  --skip-reduce          Skip phenix.reduce step"
    echo "  --skip-prepare         Skip kgs_prepare.py step"
    exit 1
fi

# ── Derive names ──
BASENAME=$(basename "$INPUT_PDB" .pdb)
BASENAME=$(basename "$BASENAME" .ent)
WORKDIR="${OUTPUT_BASE}/${BASENAME}"
mkdir -p "$WORKDIR"

# Copy original PDB
cp "$INPUT_PDB" "$WORKDIR/${BASENAME}_original.pdb"
CURRENT_PDB="$WORKDIR/${BASENAME}_original.pdb"

echo "=============================================="
echo "KGS Analysis Pipeline"
echo "=============================================="
echo "Input:            $INPUT_PDB"
echo "Basename:         $BASENAME"
echo "Working dir:      $WORKDIR"
echo "Samples:          $SAMPLES"
echo "Step size:        $STEP_SIZE"
echo "Collision factor: $COLLISION_FACTOR"
echo "SVD cutoff:       $SVD_CUTOFF"
echo "=============================================="

# ── Step 0: Extract chain if requested ──
if [ -n "$CHAIN" ]; then
    echo ""
    echo "[Step 0] Extracting chain $CHAIN..."
    CHAIN_PDB="$WORKDIR/${BASENAME}_chain${CHAIN}.pdb"
    # Simple chain extraction using grep
    grep -E "^(ATOM|HETATM)" "$CURRENT_PDB" | awk -v chain="$CHAIN" '$22==chain' > "$CHAIN_PDB.tmp"
    # Add header and footer
    echo "REMARK Chain $CHAIN extracted from $BASENAME" > "$CHAIN_PDB"
    cat "$CHAIN_PDB.tmp" >> "$CHAIN_PDB"
    echo "END" >> "$CHAIN_PDB"
    rm "$CHAIN_PDB.tmp"
    CURRENT_PDB="$CHAIN_PDB"
    echo "  -> Saved: $CHAIN_PDB"
    echo "  -> Atoms: $(grep -c '^ATOM' "$CHAIN_PDB")"
fi

# ── Step 1: Add hydrogens with phenix.reduce ──
if [ "$SKIP_REDUCE" = false ]; then
    echo ""
    echo "[Step 1] Running phenix.reduce to add hydrogens..."
    REDUCED_PDB="$WORKDIR/${BASENAME}_h.pdb"
    
    if command -v phenix.reduce &> /dev/null; then
        phenix.reduce "$CURRENT_PDB" > "$REDUCED_PDB" 2> "$WORKDIR/reduce.log"
        echo "  -> Saved: $REDUCED_PDB"
        echo "  -> Atoms before: $(grep -c '^ATOM' "$CURRENT_PDB")"
        echo "  -> Atoms after:  $(grep -c '^ATOM' "$REDUCED_PDB")"
    else
        echo "  WARNING: phenix.reduce not found. Skipping hydrogen addition."
        echo "  Make sure your PDB already has hydrogens."
        REDUCED_PDB="$CURRENT_PDB"
    fi
    CURRENT_PDB="$REDUCED_PDB"
else
    echo ""
    echo "[Step 1] Skipping phenix.reduce (--skip-reduce)"
fi

# ── Step 2: Prepare for KGS with kgs_prepare.py ──
if [ "$SKIP_PREPARE" = false ]; then
    echo ""
    echo "[Step 2] Running kgs_prepare.py..."
    PREPARED_PDB="$WORKDIR/${BASENAME}_prepared.pdb"
    
    if command -v kgs_prepare.py &> /dev/null; then
        kgs_prepare.py "$CURRENT_PDB" "$PREPARED_PDB" 2>&1 | tee "$WORKDIR/prepare.log"
        echo "  -> Saved: $PREPARED_PDB"
    elif command -v python3 &> /dev/null && [ -f "$(which kgs_prepare.py 2>/dev/null)" ]; then
        python3 "$(which kgs_prepare.py)" "$CURRENT_PDB" "$PREPARED_PDB" 2>&1 | tee "$WORKDIR/prepare.log"
        echo "  -> Saved: $PREPARED_PDB"
    else
        echo "  WARNING: kgs_prepare.py not found. Using reduced PDB directly."
        PREPARED_PDB="$CURRENT_PDB"
    fi
    CURRENT_PDB="$PREPARED_PDB"
else
    echo ""
    echo "[Step 2] Skipping kgs_prepare.py (--skip-prepare)"
fi

# ── Step 3: Run KGS hierarchy ──
echo ""
echo "[Step 3] Running kgs_hierarchy..."
KGS_OUTDIR="$WORKDIR/kgs_results"
mkdir -p "$KGS_OUTDIR"

KGS_CMD="kgs_hierarchy \
    --initial $CURRENT_PDB \
    --workingDirectory $KGS_OUTDIR \
    --samples $SAMPLES \
    --stepSize $STEP_SIZE \
    --collisionFactor $COLLISION_FACTOR \
    --svdCutoff $SVD_CUTOFF \
    --saveData $SAVE_DATA \
    --collisionCheck $COLLISION_CHECK"

echo "  Command: $KGS_CMD"
echo ""

# Run and capture both stdout and stderr
$KGS_CMD 2>&1 | tee "$KGS_OUTDIR/kgs_hierarchy.log"

echo ""
echo "[Step 3] KGS hierarchy complete."

# ── Step 4: Extract diagnostics from log ──
echo ""
echo "[Step 4] Extracting diagnostics..."
LOG_FILE="$KGS_OUTDIR/kgs_hierarchy.log"
DIAG_FILE="$WORKDIR/diagnostics.txt"

{
    echo "=============================================="
    echo "KGS DIAGNOSTICS: $BASENAME"
    echo "Date: $(date)"
    echo "=============================================="
    echo ""
    
    # Extract key numbers from log
    echo "--- Structure Summary ---"
    grep -i "atoms" "$LOG_FILE" | head -5 || echo "  (not found)"
    grep -i "hydrogen bond" "$LOG_FILE" | head -5 || echo "  (not found)"
    grep -i "hydrophobic" "$LOG_FILE" | head -5 || echo "  (not found)"
    grep -i "covalent" "$LOG_FILE" | head -5 || echo "  (not found)"
    
    echo ""
    echo "--- DoF Summary ---"
    grep -i "dof\|degree" "$LOG_FILE" | head -10 || echo "  (not found)"
    grep -i "free\|cycle\|total" "$LOG_FILE" | head -10 || echo "  (not found)"
    
    echo ""
    echo "--- Nullspace / SVD ---"
    grep -i "nullspace\|null space\|singular\|rank" "$LOG_FILE" | head -10 || echo "  (not found)"
    grep -i "rigid" "$LOG_FILE" | head -10 || echo "  (not found)"
    
    echo ""
    echo "--- Sampling ---"
    grep -i "step size\|collision\|sample\|direction" "$LOG_FILE" | head -20 || echo "  (not found)"
    
    echo ""
    echo "--- Violation Check ---"
    grep -i "violation\|predicted\|observed" "$LOG_FILE" | head -20 || echo "  (not found)"
    
} > "$DIAG_FILE"

echo "  -> Diagnostics saved: $DIAG_FILE"

# ── Step 5: List output files ──
echo ""
echo "[Step 5] Output files:"
echo "  Working directory: $WORKDIR"
echo ""
echo "  Key files:"
ls -la "$KGS_OUTDIR"/*.pdb 2>/dev/null | awk '{print "    " $NF " (" $5 " bytes)"}' || echo "    (no PDB files)"
ls -la "$KGS_OUTDIR"/*.txt 2>/dev/null | awk '{print "    " $NF " (" $5 " bytes)"}' || echo "    (no TXT files)"
ls -la "$KGS_OUTDIR"/*.log 2>/dev/null | awk '{print "    " $NF " (" $5 " bytes)"}' || echo "    (no LOG files)"

# ── Step 6: Create comparison-ready summary ──
SUMMARY_FILE="$WORKDIR/comparison_summary.txt"
{
    echo "=============================================="
    echo "COMPARISON SUMMARY: $BASENAME"
    echo "=============================================="
    echo ""
    echo "Input PDB:         $INPUT_PDB"
    echo "Chain:             ${CHAIN:-all}"
    echo "Samples:           $SAMPLES"
    echo "Step size:         $STEP_SIZE"
    echo "SVD cutoff:        $SVD_CUTOFF"
    echo "Collision factor:  $COLLISION_FACTOR"
    echo ""
    
    # Count output structures
    N_SAMPLES=$(ls "$KGS_OUTDIR"/sample_*.pdb 2>/dev/null | wc -l | tr -d ' ')
    echo "Samples generated: $N_SAMPLES"
    
    # Extract H-bond count
    N_HBONDS=$(grep -c "hydrogen bond" "$LOG_FILE" 2>/dev/null || echo "unknown")
    echo "H-bonds:           $N_HBONDS (grep count, verify from stats file)"
    
    echo ""
    echo "--- Files for ROPE comparison ---"
    echo "Initial structure:  $CURRENT_PDB"
    echo "KGS samples:        $KGS_OUTDIR/sample_*.pdb"
    echo "KGS stats:           $KGS_OUTDIR/stats.txt (if saveData>=3)"
    echo "KGS hbonds:          $KGS_OUTDIR/hbonds.txt (if saveData>=3)"
    echo "KGS hierarchy data:  $KGS_OUTDIR/hierarchy_data.txt (if saveData>=3)"
    
} > "$SUMMARY_FILE"

echo ""
echo "  -> Summary saved: $SUMMARY_FILE"

echo ""
echo "=============================================="
echo "Pipeline complete!"
echo "=============================================="
echo ""
echo "Next steps:"
echo "  1. Check diagnostics:  cat $DIAG_FILE"
echo "  2. Check summary:      cat $SUMMARY_FILE"
echo "  3. Compare H-bonds:    cat $KGS_OUTDIR/hbonds.txt"
echo "  4. Load samples in ROPE for distance-distance comparison"
echo ""