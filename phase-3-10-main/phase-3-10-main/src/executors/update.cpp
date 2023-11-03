#include "global.h"
/**
 * @brief 
 * SYNTAX: UPDATE <table_name> COLUMN <column_name> <OPERATOR> <value>
 */
bool syntacticParseUPDATE()
{
    logger.log("syntacticParseUPDATE");
    if (tokenizedQuery.size() != 6 || tokenizedQuery[2] != "COLUMN") {cout << "SYNTAX ERROR" << endl; return false;}
    parsedQuery.updateRelationName = tokenizedQuery[1];
    // cout << "works" << endl;
    parsedQuery.updateOperand = stoi(tokenizedQuery[5]);

    parsedQuery.updateColumnName = tokenizedQuery[3];
    parsedQuery.queryType = UPDATE;
    string updateOperatorStr = tokenizedQuery[4];
    if (updateOperatorStr == "ADD")           parsedQuery.updateOperator = ADD;
    else if (updateOperatorStr == "SUBTRACT") parsedQuery.updateOperator = SUBTRACT;
    else if (updateOperatorStr == "MULTIPLY") parsedQuery.updateOperator = MULTIPLY;
    else { cout << "SYNTAX ERROR" << endl; return false; }
    // cout << "works !!!" << endl;
    return true;
}

bool semanticParseUPDATE()
{
    logger.log("semanticParseUPDATE");

    if (!tableCatalogue.isTable(parsedQuery.updateRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.updateColumnName, parsedQuery.updateRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    return true;
}

void executeUPDATE()
{
    logger.log("executeUPDATE");

    Table table = *tableCatalogue.getTable(parsedQuery.updateRelationName);
    Table *tp_tab = new Table("tp_tab", table.columns);
    
    int ID = table.getColumnIndex(parsedQuery.updateColumnName);
    Cursor cursor = table.getCursor();
    vector <int> row = cursor.getNext();

    while (!row.empty())
    {

        if (parsedQuery.updateOperator == ADD)            row[ID] = row[ID] + parsedQuery.updateOperand;
        else if (parsedQuery.updateOperator == SUBTRACT)  row[ID] = row[ID] - parsedQuery.updateOperand;
        else                                              row[ID] = row[ID] * parsedQuery.updateOperand;
        
        tp_tab->writeRow<int>(row);
        row = cursor.getNext();
    }

    rename(tp_tab->sourceFileName.c_str(), table.sourceFileName.c_str());

    parsedQuery.loadRelationName = table.tableName;
    tableCatalogue.deleteTable(table.tableName);
    
    executeLOAD();

    tp_tab->unload();
    delete tp_tab;
}