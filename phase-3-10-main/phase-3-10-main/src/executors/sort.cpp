#include"global.h"
/**
 * @brief File contains method to process SORT commands.
 * 
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order
 * 
 * sorting_order = ASC | DESC 
 */
bool syntacticParseSORT(){
    logger.log("syntacticParseSORT");
    if(tokenizedQuery.size() < 8 || tokenizedQuery[4] != "BY" || tokenizedQuery[6] != "IN" ){
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }
    else if (tokenizedQuery.size() == 10) {
        parsedQuery.sortBufferSize = stoi(tokenizedQuery[9]);
    }
    else if (tokenizedQuery.size() != 8){
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }

    parsedQuery.queryType = SORT;
    parsedQuery.sortResultRelationName = tokenizedQuery[0];
    parsedQuery.sortRelationName = tokenizedQuery[3];
    parsedQuery.sortColumnName = tokenizedQuery[5];
    string sortingStrategy = tokenizedQuery[7];
    if(sortingStrategy == "ASC")
        parsedQuery.sortingStrategy = ASC;
    else if(sortingStrategy == "DESC")
        parsedQuery.sortingStrategy = DESC;
    else{
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }
    
    return true;
}

bool semanticParseSORT(){
    logger.log("semanticParseSORT");

    if(tableCatalogue.isTable(parsedQuery.sortResultRelationName)){
        cout<<"SEMANTIC ERROR: Resultant relation already exists"<<endl;
        return false;
    }

    if(!tableCatalogue.isTable(parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    if(!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnName, parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
        return false;
    }

    return true;
}

bool sortFunc(int i, int j)
{
    if (parsedQuery.sortingStrategy == ASC)
    {
        return i > j;
    }
    return j > i;
}

void executeSORT(){
    vector<vector<int>> sortingBuffer;
    
    Table table = *tableCatalogue.getTable(parsedQuery.sortRelationName);
    int sortId = table.getColumnIndex(parsedQuery.sortColumnName);

    uint buffsize = parsedQuery.sortBufferSize; int no_sub_files = 0;
    for (uint i = 0; i < table.blockCount; i += buffsize) 
    {
        Table *curTab = new Table(table);
        curTab->tableName = table.tableName + "_subfile" + to_string(no_sub_files);
        curTab->sourceFileName = "../data/" + curTab->tableName + ".csv";
        curTab->rowCount = 0; curTab->blockCount = 0;
        curTab->rowsPerBlockCount.clear();

        for (uint j = i; j < min(table.blockCount, i + buffsize); j++) 
        {
            Page activePage = Page(table.tableName, j);
            int rowIdx = 0; vector <int> tp = activePage.getRow(rowIdx++);
            while (!tp.empty()) { sortingBuffer.push_back(tp); tp = activePage.getRow(rowIdx++); }
        }
        for (int si = 1; si < sortingBuffer.size(); si++) 
            for (int sj = 0; sj < sortingBuffer.size() - 1; sj++) 
                if (sortFunc(sortingBuffer[sj][sortId], sortingBuffer[sj + 1][sortId]))
                    swap(sortingBuffer[sj], sortingBuffer[sj + 1]);
    

        vector<vector<int>> write_block;
        while (!sortingBuffer.empty()) 
        {
            write_block.push_back(sortingBuffer.front());
            sortingBuffer.erase(sortingBuffer.begin());
            curTab->rowCount++;
     
            if ((write_block.size() == table.maxRowsPerBlock) || (sortingBuffer.size() == 0)) 
            {
                Page(table.tableName + "_subfile" + to_string(no_sub_files), curTab->blockCount, write_block, write_block.size()).writePage();
                curTab->blockCount++;
                curTab->rowsPerBlockCount.push_back(write_block.size());
                write_block.clear();
            }
        }

        sortingBuffer.clear();
        no_sub_files++;

        tableCatalogue.insertTable(curTab);
    }


    uint n_Files = no_sub_files;
    while (n_Files > 1)
    {
        int file_ct = 0;
        for (uint i = 0; i < n_Files; i += (buffsize - 1)) 
        {
            int num_sfiles = min(i + buffsize - 1, n_Files) - i;
            vector <Page>         page_buffer;
            vector <vector<int>>  RowList;
            vector <bool>         boolVec (num_sfiles, false);
            vector <int>          vakali (num_sfiles, 0), gomala (num_sfiles, 0);

            for (uint j = 0; j < num_sfiles; j++) page_buffer.push_back(Page(table.tableName + "_subfile" + to_string(i + j), gomala[j]++));

            vector<int> rowLessPrior, current_row;
            long long int PageRow, pages_written = 0, rows_written = 0;
            Table* temp_table = tableCatalogue.getTable(table.tableName + "_subfile" + to_string(file_ct));
            while (!all_of(boolVec.begin(), boolVec.end(), [](bool v) { return v; })) 
            {
                for (uint j = 0; j < num_sfiles; j++) 
                {
                    if (!boolVec[j]) 
                    {
                        current_row = page_buffer[j].getRow(vakali[j]);
                        if (current_row.empty()) 
                        {
                            try {
                                vakali[j] = 0;
                                page_buffer[j] = Page(table.tableName + "_subfile" + to_string(i + j), gomala[j]++);
                                current_row = page_buffer[j].getRow(vakali[j]);
                            } 
                            catch (...) 
                            {
                                boolVec[j] = true;
                            }
                        }
                        if (boolVec[j]) continue;
                        if (rowLessPrior.empty() || sortFunc(rowLessPrior[sortId], current_row[sortId])) { rowLessPrior = current_row; PageRow = j; }
                    }
                }
                vakali[PageRow]++;
                RowList.push_back(rowLessPrior);
                rowLessPrior.clear();
                rows_written++;
                if (RowList.size() == table.maxRowsPerBlock) 
                {
                    Page(table.tableName + "_temp_subfile", pages_written, RowList, RowList.size()).writePage();
                    temp_table->rowsPerBlockCount.push_back(RowList.size());
                    RowList.clear();
                    pages_written++;
                }
            }
            if (!RowList.empty()) 
            {
                Page(table.tableName + "_temp_subfile", pages_written, RowList, RowList.size()).writePage();
                temp_table->rowsPerBlockCount.push_back(RowList.size());
                RowList.clear();
                pages_written++;
            }
            temp_table->rowCount = rows_written; temp_table->blockCount = pages_written;
            for (int j = 0; j < temp_table->blockCount; j++) remove(("../data/temp/" + table.tableName + "_subfile" + 
                                                                    to_string(file_ct) + "_Page" + to_string(j)).c_str());

            for (int j = 0; j < pages_written; j++)          rename(("../data/temp/" + table.tableName + "_temp_subfile_Page" + to_string(j)).c_str(), 
                                                                    ("../data/temp/" + table.tableName + "_subfile" + to_string(file_ct) + "_Page" + to_string(j)).c_str());
            file_ct++;
        }
        n_Files = file_ct;
    }

    for (int i = 1; i < no_sub_files; i++) tableCatalogue.deleteTable(table.tableName + "_subfile" + to_string(i));

    Table* resultant_table = tableCatalogue.getTable(table.tableName + "_subfile" + to_string(0));
    resultant_table->tableName = parsedQuery.sortResultRelationName;
    for (int i = 0; i < resultant_table->blockCount; i++) rename(("../data/temp/" + table.tableName + "_subfile" + to_string(0) + "_Page" + to_string(i)).c_str(),
                                                                 ("../data/temp/" + parsedQuery.sortResultRelationName + "_Page" + to_string(i)).c_str());
    tableCatalogue.renameTable(table.tableName + "_subfile" + to_string(0), parsedQuery.sortResultRelationName);
}