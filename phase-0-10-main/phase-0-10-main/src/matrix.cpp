#include "global.h"

uint BLOCK_MATRIX_DIM = (uint) sqrt(BLOCK_SIZE * 1000);
// uint BLOCK_MATRIX_DIM = 3;

Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

Matrix::Matrix(string matrixName)
{
    logger.log("matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
}

bool Matrix::load()
{
    logger.log("Matrix::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if(getline(fin, line))
    {
        if (this->extractMatrixDims(line))
            if(this->blockify())
                return true;
    }
    fin.close();
    return false;
}

bool Matrix::extractMatrixDims(string firstline)
{
    logger.log("Matrix::extractMatrixDims");
    string word;
    stringstream s(firstline);
    int len = 0;
    while(getline(s,word,','))
    {    
        len++;
    }
    this->columnCount = len;
    this->rowCount = len;
    if (len > 0)
        return true;
    else return false;

}

bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    int N = this->columnCount;
    vector<int>row(N, 0);
    if (BLOCK_MATRIX_DIM > N) BLOCK_MATRIX_DIM = N;
    this->blockDim = BLOCK_MATRIX_DIM;
    int pageCount = pow(ceil(N / (float)BLOCK_MATRIX_DIM), 2);
    vector<vector<int>> rowsInPage(BLOCK_MATRIX_DIM, row);
    int pageCounter = 0;
    cout << "BLOCK_MATRIX_DIM: " << BLOCK_MATRIX_DIM << "\n";
    cout << "N: " << N << "\n";
    cout << "pageCount: " << pageCount << "\n";

    while(getline(fin, line))
    {
        stringstream s(line);
        for(int columnCounter = 0; columnCounter < N; columnCounter++)
        {
            if (!getline(s, word, ','))
                return false;
            row[columnCounter] = stoi(word);
            rowsInPage[pageCounter][columnCounter] = row[columnCounter];
        }
        pageCounter++;
        if (pageCounter == BLOCK_MATRIX_DIM)
        {
            vector<vector<int>> blockMatrix(BLOCK_MATRIX_DIM, vector<int>(BLOCK_MATRIX_DIM,0));
            int colindex = 0;
            for(int i = 0; i< (uint)sqrt(pageCount); i++){
                for ( int j= 0; j< BLOCK_MATRIX_DIM; j++){
                    for(int k=0; k< BLOCK_MATRIX_DIM; k++){
                        blockMatrix[j][k] = rowsInPage[j][colindex+k];
                    }
                }
                colindex += BLOCK_MATRIX_DIM;
                bufferManager.writePage(this->matrixName, this->blockCount, blockMatrix, BLOCK_MATRIX_DIM);
                this->blockCount++;
                }   
            pageCounter = 0;   
        }
        
    }
    if (pageCounter)
    {
        vector<vector<int>> blockMatrix(BLOCK_MATRIX_DIM, vector<int>(BLOCK_MATRIX_DIM,0));
        int colindex = 0;
        for(int i = 0; i< (uint)sqrt(pageCount); i++){
            for ( int j= 0; j< BLOCK_MATRIX_DIM && j < pageCounter; j++){
                for(int k=0; k< BLOCK_MATRIX_DIM; k++){
                    blockMatrix[j][k] = rowsInPage[j][colindex+k];
                }
            }
            colindex += BLOCK_MATRIX_DIM;
            bufferManager.writePage(this->matrixName, this->blockCount, blockMatrix, BLOCK_MATRIX_DIM);
            this->blockCount++;
            }   
        pageCounter = 0;   
    }

    if (this->rowCount == 0)
        return false;
    return true;

}

void Matrix::print()
{
    logger.log("Matrix::print");
    uint count = min((long long)PRINT_COUNT, this->rowCount);
    Cursor cursor(this->matrixName, 0);
    vector<int> row;
    vector<vector<int>> rowsInPage(BLOCK_MATRIX_DIM, vector<int>(count));
    for (int rowCounter = 0; rowCounter < (uint)ceil(count/(float)BLOCK_MATRIX_DIM); rowCounter++)
    {
        for(int i=0; i<(uint)ceil(count/(float)BLOCK_MATRIX_DIM); i++)
        {
            for(int j=0; j<BLOCK_MATRIX_DIM; j++)
            {
                row = cursor.getNext();
                for(int k=0; k<BLOCK_MATRIX_DIM && i*BLOCK_MATRIX_DIM + k<count; k++) rowsInPage[j][i*BLOCK_MATRIX_DIM + k]  = row[k];
            }
        }
        for(int j=0; j<BLOCK_MATRIX_DIM && rowCounter*BLOCK_MATRIX_DIM + j < count; j++) this->writeRow(rowsInPage[j], cout);
    }
    printRowCount(this->rowCount);
}

void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNext");

        if (cursor->pageIndex < this->blockCount - 1)
        {
            cursor->nextPage(cursor->pageIndex+1);
        }
}

void Matrix::makePermanent()
{
    logger.log("Matrix::makePermanent");
    if(!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);


    uint count = this->rowCount;
    Cursor cursor(this->matrixName, 0);
    vector<int> row;
    vector<vector<int>> rowsInPage(BLOCK_MATRIX_DIM, vector<int>(this->columnCount));
    for (int rowCounter = 0; rowCounter < (uint)ceil(count/(float)BLOCK_MATRIX_DIM); rowCounter++)
    {
        for(int i=0; i<(uint)ceil(this->columnCount/(float)BLOCK_MATRIX_DIM); i++)
        {
            for(int j=0; j<BLOCK_MATRIX_DIM; j++)
            {
                row = cursor.getNext();
                for(int k=0; k<BLOCK_MATRIX_DIM && i*BLOCK_MATRIX_DIM + k<this->columnCount; k++) rowsInPage[j][i*BLOCK_MATRIX_DIM + k]  = row[k];
            }
        }
        for(int j=0; j<BLOCK_MATRIX_DIM && rowCounter*BLOCK_MATRIX_DIM + j < count; j++) this->writeRow(rowsInPage[j], fout);
    }
    fout.close();


    
}

bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
    return true;
    return false;
}

void Matrix::unload(){
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

Cursor Matrix::getCursor()
{
    logger.log("Matrix::getCursor");
    Cursor cursor(this->matrixName, 0);
    return cursor;
}

