#include <vagabond/core/Flexibility.h>
#include "FlexibilityTypes.h"
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <array>

void Flexibility::writeJacobianToCSV(const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "[ERROR] Could not open file: " << filename << std::endl;
        return;
    }

    // --- header row: one label per column ---
    file << "row_label";
    for (auto &[col, c] : _constraintMap)
    {
        std::string typeName;
        switch (c.type)
        {
            case Distance:   typeName = "Dist";  break;
            case AngleAlpha: typeName = "AngA";  break;
            case AngleBeta:  typeName = "AngB";  break;
            case Dihedral_1: typeName = "Dih1";  break;
            case Dihedral_2: typeName = "Dih2";  break;
            default:         typeName = "?";     break;
        }

        std::string bondLabel = c.hbond->Donor->desc() + "-" + c.hbond->Acceptor->desc();
        file << "," << bondLabel << "_" << typeName;
    }
    file << "\n";

    // --- one row per DoF, with a label describing what it is ---
    for (auto &[row, dof] : _activeDoFMap)
    {
        std::string typeName;
        switch (dof.type)
        {
            case Torsion: typeName = "Tors_idx" + std::to_string(dof.idx); break;
            case TranslX: typeName = "TranslX"; break;
            case TranslY: typeName = "TranslY"; break;
            case TranslZ: typeName = "TranslZ"; break;
            case RotX:    typeName = "RotX";    break;
            case RotY:    typeName = "RotY";    break;
            case RotZ:    typeName = "RotZ";    break;
        }

        file << dof.chain << "_" << typeName;

        for (auto &[col, c] : _constraintMap)
        {
            file << "," << _jacobMtx(row, col);
        }
        file << "\n";
    }

    file.close();
    std::cout << "[DEBUG] Wrote Jacobian to " << filename << std::endl;
}

void Flexibility::writeVMatrixToCSV(const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "[ERROR] Could not open file: " << filename << std::endl;
        return;
    }

    int totalModes = (int)_V.cols();

    file << "dof_label";
    for (int col = 0; col < totalModes; col++)
        file << ",mode" << col;
    file << "\n";

    for (auto &[row, dof] : _activeDoFMap)
    {
        std::string typeName;
        switch (dof.type)
        {
            case Torsion: typeName = "Tors_idx" + std::to_string(dof.idx); break;
            case TranslX: typeName = "TranslX"; break;
            case TranslY: typeName = "TranslY"; break;
            case TranslZ: typeName = "TranslZ"; break;
            case RotX:    typeName = "RotX";    break;
            case RotY:    typeName = "RotY";    break;
            case RotZ:    typeName = "RotZ";    break;
        }

        file << dof.chain << "_" << typeName;
        for (int col = 0; col < totalModes; col++)
            file << "," << _V(row, col);
        file << "\n";
    }

    file.close();
    std::cout << "[DEBUG] Wrote V matrix to " << filename << std::endl;
}

void Flexibility::writeSingularValuesToCSV(const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "[ERROR] Could not open file: " << filename << std::endl;
        return;
    }

    file << "mode_index,singular_value\n";
    for (int i = 0; i < (int)_S.size(); i++)
        file << i << "," << _S(i) << "\n";

    file.close();
    std::cout << "[DEBUG] Wrote singular values to " << filename << std::endl;
}

void Flexibility::writeJacobianStatsToCSV(const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "[ERROR] Could not open file: " << filename << std::endl;
        return;
    }

    // --- per-chain DoF counts ---
    std::map<std::string, int> torsionCount, rbCount;
    for (auto &[row, dof] : _dofMap)
    {
        if (dof.type == Torsion) torsionCount[dof.chain]++;
        else rbCount[dof.chain]++;
    }

    // --- per-constraint-type totals, and H-bond/VdW intra/inter counts ---
    // (reusing _constraintMap's cached donorGroup/acceptorGroup, no re-searching subsets)
    std::map<ConstraintType, int> constraintTypeCount;
    int hbondIntra = 0, hbondInter = 0;
    int vdwIntra = 0, vdwInter = 0;
    std::set<BondEntity*> seen;

    for (auto &[col, c] : _constraintMap)
    {
        constraintTypeCount[c.type]++;   // every column counts here

        if (seen.count(c.hbond)) continue;   // dedupe only for the bond-level intra/inter counts
        seen.insert(c.hbond);

        bool intra = (c.donorGroup == c.acceptorGroup);
        bool isVdW = (dynamic_cast<VdWBondEntity*>(c.hbond) != nullptr);

        if (isVdW)
        {
            if (intra) vdwIntra++; else vdwInter++;
        }
        else
        {
            if (intra) hbondIntra++; else hbondInter++;
        }
    }

    // --- matrix-level stats (only meaningful once _jacobMtx is built) ---
    int totalRows = (int)_jacobMtx.rows();
    int totalCols = (int)_jacobMtx.cols();
    long nnz = (_jacobMtx.array() != 0.0f).count();
    double density = (totalRows > 0 && totalCols > 0)
                    ? (double)nnz / ((double)totalRows * totalCols) : 0.0;

    int zeroRows = 0;
    for (int r = 0; r < totalRows; r++)
        if (_jacobMtx.row(r).isZero()) zeroRows++;

    int zeroCols = 0;
    for (int c = 0; c < totalCols; c++)
        if (_jacobMtx.col(c).isZero()) zeroCols++;

    float minVal = (totalRows && totalCols) ? _jacobMtx.minCoeff() : 0.f;
    float maxVal = (totalRows && totalCols) ? _jacobMtx.maxCoeff() : 0.f;
    float meanNonZero = nnz > 0 ? (float)(_jacobMtx.array().abs().sum() / nnz) : 0.f;

    // --- write ---
    file << "stat,value\n";

    for (auto &[chain, count] : torsionCount)
        file << "torsion_DoF_" << chain << "," << count << "\n";
    for (auto &[chain, count] : rbCount)
        file << "rigidbody_DoF_" << chain << "," << count << "\n";

    file << "total_torsion_DoF," << [&]{ int s=0; for(auto&p:torsionCount) s+=p.second; return s; }() << "\n";
    file << "total_rigidbody_DoF," << [&]{ int s=0; for(auto&p:rbCount) s+=p.second; return s; }() << "\n";

    file << "constraints_Distance," << constraintTypeCount[Distance] << "\n";
    file << "constraints_AngleAlpha," << constraintTypeCount[AngleAlpha] << "\n";
    file << "constraints_AngleBeta," << constraintTypeCount[AngleBeta] << "\n";
    file << "constraints_Dihedral_1," << constraintTypeCount[Dihedral_1] << "\n";
    file << "constraints_Dihedral_2," << constraintTypeCount[Dihedral_2] << "\n";

    file << "hbonds_intra_chain," << hbondIntra << "\n";
    file << "hbonds_inter_chain," << hbondInter << "\n";
    file << "vdw_intra_chain," << vdwIntra << "\n";
    file << "vdw_inter_chain," << vdwInter << "\n";

    file << "matrix_rows," << totalRows << "\n";
    file << "matrix_cols," << totalCols << "\n";
    file << "matrix_nnz," << nnz << "\n";
    file << "matrix_density," << density << "\n";
    file << "matrix_zero_rows," << zeroRows << "\n";
    file << "matrix_zero_cols," << zeroCols << "\n";
    file << "matrix_min_value," << minVal << "\n";
    file << "matrix_max_value," << maxVal << "\n";
    file << "matrix_mean_abs_nonzero," << meanNonZero << "\n";

    file.close();
    std::cout << "[DEBUG] Wrote Jacobian stats to " << filename << std::endl;
}

void Flexibility::writeConstraintMapToCSV(const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "[ERROR] Could not open file: " << filename << std::endl;
        return;
    }

    file << "col_idx,type,donorGroup_chain,donorGroup_size,"
         << "acceptorGroup_chain,acceptorGroup_size,"
         << "Donor,Hydrogen,Acceptor,TorsionVec_size\n";

    for (auto& [col, hbc] : _constraintMap)
    {
        std::string typeName;
        switch(hbc.type)
        {
            case Distance:   typeName = "Distance";   break;
            case AngleAlpha: typeName = "AngleAlpha"; break;
            case AngleBeta:  typeName = "AngleBeta";  break;
            case Dihedral_1: typeName = "Dihedral_1"; break;
            case Dihedral_2: typeName = "Dihedral_2"; break;
            default:         typeName = "Unknown";    break;
        }

        file << hbc.col_idx << ","
             << typeName << ","
             << hbc.donorGroup->atomVector()[0]->chain() << ","
             << hbc.donorGroup->size() << ","
             << hbc.acceptorGroup->atomVector()[0]->chain() << ","
             << hbc.acceptorGroup->size() << ","
             << hbc.hbond->Donor->desc() << ","
             << hbc.hbond->Acceptor->desc() << ","
             << hbc.hbond->TorsionVec.size() << "\n";
    }

    file.close();
}


void Flexibility::printRigidBodyWeights(const std::vector<float> &v_i)
{
    std::map<AtomGroup*, std::array<float,6>> rbWeights;

    for (auto &[row, dof] : _activeDoFMap)
    {
        if (dof.type == Torsion) continue;

        int idx = -1;
        switch (dof.type)
        {
            case TranslX: idx = 0; break;
            case TranslY: idx = 1; break;
            case TranslZ: idx = 2; break;
            case RotX:    idx = 3; break;
            case RotY:    idx = 4; break;
            case RotZ:    idx = 5; break;
            default: continue;
        }
        rbWeights[dof.atoms][idx] = v_i[row];
    }

    for (auto &[group, w] : rbWeights)
    {
        std::cout << "[DEBUG RB weights] chain " << group->atomVector()[0]->chain()
                  << " | v: (" << w[0] << ", " << w[1] << ", " << w[2] << ")"
                  << " | w: (" << w[3] << ", " << w[4] << ", " << w[5] << ")"
                  << std::endl;
    }
}



