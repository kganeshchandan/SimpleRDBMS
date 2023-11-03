#include "global.h"


bool syntacticParseCROSS_TRANSPOSE()
{
    logger.log("syntacticParseCROSS_TRANSPOSE");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = CROSS_TRANSPOSE;
    parsedQuery.crossFirstRelationName  = tokenizedQuery[1];
    parsedQuery.crossSecondRelationName = tokenizedQuery[2];
    return true;
}

bool semanticParseCROSS_TRANSPOSE()
{
    logger.log("semanticParsePRINTMATRIX");
    if (!matrixCatalogue.isMatrix(parsedQuery.crossFirstRelationName))
    {
        cout << "SEMANTIC ERROR:"<<parsedQuery.crossFirstRelationName<<" Relation doesn't exist" << endl;
        return false;
    }
    if (!matrixCatalogue.isMatrix(parsedQuery.crossSecondRelationName))
    {
        cout << "SEMANTIC ERROR:"<<parsedQuery.crossSecondRelationName<<" Relation doesn't exist" << endl;
        return false;
    }
    return true;
}
void executeCROSS_TRANSPOSE()
{
    logger.log("executeCROSS_TRANSPOSE");
    cout << "Cross Transposing Matrices"<< endl;
    Matrix* m1 = matrixCatalogue.getMatrix(parsedQuery.crossFirstRelationName);
    Matrix* m2 = matrixCatalogue.getMatrix(parsedQuery.crossSecondRelationName);
    if(m1->columnCount != m2->columnCount) {cout<<"Both Matrices dont have same sizes"<<endl; return;}

    vector<int> row;
    vector<vector<int>> temp_block_1(m1->blockDim, row);
    vector<vector<int>> temp_block_2(m1->blockDim, row);
    Cursor m1_cursor(m1->matrixName, 0);
    Cursor m2_cursor(m2->matrixName, 0);
    int temp_var, m1_pageIndex, m2_pageIndex;
    for(int i=0; i<ceil(m1->rowCount/(float) m1->blockDim); i++)
    {
        for(int j=0; j<ceil(m1->columnCount/(float) m1->blockDim); j++)
        {
            m1_pageIndex = (i*ceil(m1->columnCount/(float) m1->blockDim)) + j;
            m1_cursor.nextPage(m1_pageIndex);
            for(int k=0; k<m1->blockDim; k++) temp_block_1[k] = m1_cursor.getNext();
            for(int k=0; k<m1->blockDim; k++)
            {
                for(int l=k+1; l<m1->blockDim; l++)
                {
                    temp_var = temp_block_1[k][l];
                    temp_block_1[k][l] = temp_block_1[l][k];
                    temp_block_1[l][k] = temp_var;
                }
            }
            m2_pageIndex = (j*ceil(m1->columnCount/(float) m1->blockDim)) + i;
            m2_cursor.nextPage(m2_pageIndex);
            for(int k=0; k<m1->blockDim; k++) temp_block_2[k] = m2_cursor.getNext();
            for(int k=0; k<m1->blockDim; k++)
            {
                for(int l=k+1; l<m1->blockDim; l++)
                {
                    temp_var = temp_block_2[k][l];
                    temp_block_2[k][l] = temp_block_2[l][k];
                    temp_block_2[l][k] = temp_var;
                }
            }

            bufferManager.writePage(m1->matrixName, m1_pageIndex, temp_block_2, m1->blockDim);
            bufferManager.writePage(m2->matrixName, m2_pageIndex, temp_block_1, m1->blockDim);
        }
    }
    bufferManager.clearPool();
    cout << "Cross Transposing Finished"<< endl;
    return;
}