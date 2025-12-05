
void Flexibility::calculateFlexWeights()
{
/**
 * @brief Computes flexibility weights for protein torsions.
 *
 * This function calculates flexibility weights based on the last column of the 
 * precomputed SVD matrix (_V). It retrieves all torsion angles in the system 
 * and assigns corresponding weights to them.
 *
 * Key steps:
 * - Extracts the last column of the SVD matrix as the weight column.
 * - Initializes the torsion vector with zeros.
 * - Ensures the weight column size matches the global torsion vector size.
 * - Assigns weights to the appropriate torsion indices, with error handling 
 *   for out-of-bounds indices.
 *
 * Error handling:
 * - Prints an error if `_globalTorsionVector` and `weightColumn` sizes do not match.
 * - Checks for invalid torsion indices before assigning values.
 *
 * @note This function assumes that `_V` has been computed and that 
 *       `_globalTorsionVector` correctly maps torsions.
 */

    std::cout << "Calculating flex weights..." << std::endl;

    // Get all the torsions of the protein
    int totalTorsionNum = _resources.sequences->torsionBasis()->parameterCount();


    // std::vector<float> weightColumn(_V.rows());
    Eigen::VectorXf modeWeights = Eigen::VectorXf::Ones(_V.cols()); 
    // Weighted combination: V * modeWeights
    Eigen::VectorXf weightedTorsions = _V * modeWeights;  // This gives a single vector of size _V.rows()



    // for (int i = 0; i < _V.rows(); ++i)
    // {
    //     // weightColumn[i] = _V(i, _V.cols() - 1);
    //     weightColumn[i] = _V(i, 0);
    // }

    // _allTorsions = std::vector<float>(totalTorsionNum); // Initialize with zeros


    // Debug size of _globalTorsionVector


    // if (_globalTorsionVector.size() != weightColumn.size())
    // {
    //     std::cerr << "Error: Size mismatch between _globalTorsionVector ("
    //               << _globalTorsionVector.size() << ") and weightColumn ("
    //               << weightColumn.size() << ")." << std::endl;
    //     return;
    // }


    //// begin new
    if (_globalTorsionVector.size() != weightedTorsions.size())
    {
        std::cerr << "Error: Size mismatch between _globalTorsionVector ("
                  << _globalTorsionVector.size() << ") and weightedTorsions ("
                  << weightedTorsions.size() << ")." << std::endl;
        return;
    }

    _allTorsions = std::vector<float>(totalTorsionNum, 0.0f); // Initialize with zeros
    for (int i = 0; i < _globalTorsionVector.size(); ++i)
    {
        int index = _globalTorsionVector[i];
        if (index < 0 || index >= totalTorsionNum)
        {
            std::cerr << "Error: Index out of bounds in _globalTorsionVector: "
                      << index << std::endl;
            continue;
        }
        _allTorsions[index] = weightedTorsions[i];
    }
    //// end new


    // Assign weights to torsion vectors
    // for (int i = 0; i < _globalTorsionVector.size(); ++i)
    // {
    //     int index = _globalTorsionVector[i];
    //     if (index < 0 || index >= totalTorsionNum)
    //     {
    //         std::cerr << "Error: Index out of bounds in _globalTorsionVector: "
    //                   << index << std::endl;
    //         continue;
    //     }
    //     _allTorsions[index] = weightColumn[i];
    // }

    std::cout << "Finished calculating flex weights." << std::endl;

}