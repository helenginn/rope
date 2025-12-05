void Flexibility::calculateFlexWeights()
{
    std::cout << "Calculating flex weights..." << std::endl;

    // Get all the torsions of the protein
    int totalTorsionNum = _resources.sequences->torsionBasis()->parameterCount();

    //try
    std::vector<float> weightColumn(_V.rows());
    _allTorsions = std::vector<float>(totalTorsionNum, 0.0f); // Initialize with zeros
    // Debug size of _globalTorsionVector
    if (_globalTorsionVector.size() != weightColumn.size())
    {
        std::cerr << "Error: Size mismatch between _globalTorsionVector ("
                  << _globalTorsionVector.size() << ") and weightColumn ("
                  << weightColumn.size() << ")." << std::endl;
        return;
    }
    // finish try
    
    std::vector<std::vector<float>> V_columns; // To store all v_i vectors

    // Extract columns from _V and store as vectors
    for (int colIdx = 0; colIdx < _V.cols(); ++colIdx)
    {
        std::vector<float> v_i;
        // this can be used to select a specific column of the _V. 
        // could be used instead of colIdx
        int selectedCol = _V.cols() - 1; 

        for (int rowIdx = 0; rowIdx < _V.rows(); ++rowIdx)
        {
            // float value = _V(rowIdx, colIdx);
            float value = _V(rowIdx, selectedCol);
            v_i.push_back(value);
        }
        V_columns.push_back(v_i);

        // Assign weights from this column to _allTorsions
        for (int i = 0; i < _globalTorsionVector.size(); ++i)
        {
            int index = _globalTorsionVector[i];
            if (index < 0 || index >= totalTorsionNum)
            {
                std::cerr << "Error: Index out of bounds in _globalTorsionVector: "
                          << index << std::endl;
                continue;
            }
            _allTorsions[index] = v_i[i];
        }
        _allTorsionsHistory.push_back(_allTorsions);
    }
    std::cout << "V_columns.size()" << std::endl;
    std::cout << V_columns.size() << std::endl;

}