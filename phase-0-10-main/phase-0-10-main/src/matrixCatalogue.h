#include "matrix.h"


class MatrixCatalogue
{
    unordered_map<string, Matrix*> matrixes;

public:
    MatrixCatalogue() {}
    void insertMatrix(Matrix* matrix);
    void deleteMatrix(string matrixName);
    Matrix* getMatrix(string matrixName);
    bool isMatrix(string matrixName);
    void print();
    ~MatrixCatalogue();
};