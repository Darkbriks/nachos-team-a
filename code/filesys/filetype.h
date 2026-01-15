#ifndef FILETYPE_H
#define FILETYPE_H

class DirectoryEntry;


enum File_Type {
    NULL_T,
    FILE_T,
    DIRECTORY_T
};

inline char* file_type_to_str(const File_Type t) {
    switch(t){
        case(FILE_T):
            return (char *) "FILE";
        case (DIRECTORY_T):
            return (char *) "DIRECTORY";
        case NULL_T:
            return (char *) "ERROR OCCURS, NULL TYPE";
        default:
            return (char *) "UNKNOW TYPE";
    }
}

inline char file_type_to_char(const File_Type t) {
    switch(t){
        case(FILE_T):
            return '-';
        case (DIRECTORY_T):
            return 'd';
        case NULL_T:
            return 'X';
        default:
            return 'U';
    }
}


#endif // FILETYPE_H
