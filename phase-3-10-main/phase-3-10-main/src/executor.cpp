#include"global.h"

void executeCommand(){

    int lf;
    struct flock lck;

    if (parsedQuery.queryType == UPDATE) {
        lf = open(("../data/temp/" + parsedQuery.updateRelationName + ".lck").c_str(), O_WRONLY | O_CREAT);
        while (lf < 0) lf = open(("../data/temp/" + parsedQuery.updateRelationName + ".lck").c_str(), O_WRONLY | O_CREAT);
        memset(&lck, 0, sizeof (lck));
        lck.l_type = F_WRLCK;
        fcntl(lf, F_SETLKW, &lck);
    }


    switch(parsedQuery.queryType){
        case UPDATE: executeUPDATE(); break;
        case CLEAR: executeCLEAR(); break;
        case CROSS: executeCROSS(); break;
        case DISTINCT: executeDISTINCT(); break;
        case EXPORT: executeEXPORT(); break;
        case INDEX: executeINDEX(); break;
        case JOIN: executeJOIN(); break;
        case LIST: executeLIST(); break;
        case LOAD: executeLOAD(); break;
        case PRINT: executePRINT(); break;
        case PROJECTION: executePROJECTION(); break;
        case RENAME: executeRENAME(); break;
        case SELECTION: executeSELECTION(); break;
        case SORT: executeSORT(); break;
        case SOURCE: executeSOURCE(); break;
        default: cout<<"PARSING ERROR"<<endl;
    }

    if (lf >= 0 && lck.l_type == F_WRLCK) {
        lck.l_type = F_UNLCK;
        fcntl(lf, F_SETLKW, &lck);
        close(lf);
        remove(("../data/temp/" + parsedQuery.updateRelationName + ".lck").c_str());
    }

    return;
}

void printRowCount(int rowCount){
    cout<<"\n\nRow Count: "<<rowCount<<endl;
    return;
}