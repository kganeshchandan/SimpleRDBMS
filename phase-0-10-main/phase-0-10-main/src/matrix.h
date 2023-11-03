#ifndef MATRIX
#define MATRIX

#include "cursor.h"
enum IndexingStrategy
{
    BTREE,
    HASH,
    NOTHING
};

class Matrix
{
    
public:
    string sourceFileName = "";
    string matrixName = "";
    uint columnCount = 0;
    // uint rowCount= 0;
    uint blockDim=0;
    long long int rowCount = 0;
    uint blockCount = 0;
    bool indexed = false;
    string indexedColumn = "";
    // IndexingStrategy indexingStrategy = NOTHING;

    bool extractMatrixDims(string firstline);
    bool blockify();
    // void updateStatistics(vector<int> row);

    Matrix();
    Matrix(string matrixName);
    Matrix(string matrixName, vector<string> columns);
    bool load();
    
    bool isColumn(string columnName);
    void print();
    void makePermanent();
    bool isPermanent();
    void getNextPage(Cursor *cursor);
    Cursor getCursor();
    void unload();


template <typename T>
void writeRow(vector<T> row, ostream &fout)
{
    logger.log("Matrix::printRow");
    for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
    {
        if (columnCounter != 0)
            fout << ", ";
        fout << row[columnCounter];
    }
    fout << endl;
}


template <typename T>
void writeRow(vector<T> row)
{
    logger.log("Matrix:printRow");
    ofstream fout(this->sourceFileName, ios::app);
    this->writeRow(row, fout);
    fout.close();
}

};

#endif

