#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <wait.h>

#define STDERR 2
#define CONFIG_MAX_LENGTH 160
#define ERROR "Error in system call\n"
#define STRING_MAX_LENGTH 255
#define ARRAY_OF_COMMANDS 5

#define WRITE_ONLY 1
#define READ_ONLY 2
#define CREATE_FILE 3
#define ADD_LINE 4

#define SYSTEM_FAIL -1


/**
 * The students data structure
 * holding all the necessary data for future processing.
 */
typedef struct studentInfo {
    char name[STRING_MAX_LENGTH];
    char grade[STRING_MAX_LENGTH];
    char info[STRING_MAX_LENGTH];
    int isGraded;
    char dirPath[STRING_MAX_LENGTH];
    char cFilePath[STRING_MAX_LENGTH];
    char compiledFileName[STRING_MAX_LENGTH];
} studentInfo;

void printError();

char *strCopy(char *dest, const char *src);

char *strConcatenate(char *dest, const char *src);

int strCompare(char *string1, char *string2);

unsigned int strLength(const char *s);

int insufArgs(int argc);

int openFile(const char *filePath, int selectedMode);

char* itoa(int i, char b[]);

void readLineFromFile(int file, char *array);

void closeFile(int file);

void readConfigFile(char *const *argv, char *studentFolders, char *testInput, char *correctOutPut);

int countSubmittedFolders(char *folders);

void assignFolderAndName(studentInfo *pStudents,char *folders);

void findStudentsCFiles(int submissionsCount, studentInfo *myStudents);

void compileAllCFiles(int submissionsCount, studentInfo *myStudents);

char* findCFilePath(char *cpath);

void executeSubmissions(studentInfo *pStudents, int submissionsCount,
                        char *inputFilePath, char *outputFilePath);

void gradeStudent(studentInfo *pStudents, int i, char *grade, char *info);

int compareOutputs(studentInfo *pStudents, const char *outputFilePath, int i, int retCode, int *value,char *outputFileName);

int executeCFile(char *inputFilePath, char **args, int retCode,int i,char *outputFileName);

void writeToCSV(studentInfo *pStudents, int count);

char *studentToString(studentInfo *pStudents, int i);

void executeCommand(char **args);

int checkCompileSuccess(char *compiledFile,char *compiledFilePath);

int string_ends_with(char * str, char * suffix);

/**
 * The main function.
 * @param argc - number of provided command line arguments.
 * @param argv - an array holding the passed cmd arguments.
 * @return
 */
int main(int argc, char *argv[]) {
    //checking for a config file as a command line input.
    insufArgs(argc);

    //the location of the folders containing the c files.
    char studentFolders[CONFIG_MAX_LENGTH + 1];
    //the location of the inputs that we would like to run.
    char testInput[CONFIG_MAX_LENGTH + 1];
    //the location of the text file containing the correct output.
    char correctOutPut[CONFIG_MAX_LENGTH + 1];

    readConfigFile(argv, studentFolders, testInput, correctOutPut);

    //checking how much students\folders there are to go through and grade.
    int submissionsCount = countSubmittedFolders(studentFolders);

    studentInfo* myStudents=(studentInfo *)malloc(submissionsCount * sizeof(studentInfo));
    // checking if data allocated accordingly.
    if (myStudents==NULL) {
        printError();
        exit(SYSTEM_FAIL);
    }

    //go over all the folders in the studentFolder & Process the information.
    assignFolderAndName(myStudents, studentFolders);

    //find all the submitted c files.
    findStudentsCFiles(submissionsCount, myStudents);

    //compile the c files.
    compileAllCFiles(submissionsCount, myStudents);

    //execute all the .out files & grade them upon performance.
    executeSubmissions(myStudents, submissionsCount, testInput, correctOutPut);

    //write the score according to result.
    writeToCSV(myStudents,submissionsCount);

    //freeing the allocated data before returning.
    free(myStudents);
    return 0;
}

/**
 * The function executes each of the compiled c files & grades them.
 * @param pStudents - an array holding all the information of the studentInfo submissions.
 * @param submissionsCount - the amount of submissions we need to process.
 * @param inputFilePath - path to the input that we would like to use.
 * @param outputFilePath - path to the correct output we are expecting.
 */
void executeSubmissions(studentInfo *pStudents, int submissionsCount,
                        char *inputFilePath, char *outputFilePath) {
    for (int i = 0; i < submissionsCount; ++i) {

        if(pStudents[i].isGraded == 1){
            continue;
        }
        //preparing an array for the command.
        char *args[ARRAY_OF_COMMANDS];
        for (int j = 0; j < ARRAY_OF_COMMANDS; ++j) {
            args[j] = NULL;
        }
        char string[STRING_MAX_LENGTH];
        strCopy(string, "./");
        strConcatenate(string, pStudents[i].compiledFileName);
        args[0] = string;


        char array[STRING_MAX_LENGTH];
        strCopy(array,"output");
        char itoaArray[STRING_MAX_LENGTH];
        itoa(i,itoaArray);
        strConcatenate(array,itoaArray);
        strConcatenate(array,".txt");

        //forking the main process to execute the compiled c file on the child process.
        int stat, retCode;
        pid_t pid;
        pid = fork();

        // execute the c file in the child process.
        if (pid == 0) {
            retCode = executeCFile(inputFilePath, args, retCode,i,array);

        }else{
            int sleepTimer = 0;
            int status;
            pid_t returnPid;

            for (int j = 0; j < 5; ++j) {
                sleep(1);
                returnPid = waitpid(pid, &status, WNOHANG);
                if(returnPid != 0){
                    int a = unlink(pStudents[i].compiledFileName);
                    break;
                }
            }
            //if temp.out still running
            if (returnPid == 0) {
                int a = unlink(pStudents[i].compiledFileName);
                int b = unlink(array);
                gradeStudent(pStudents,i, "0", "TIMEOUT");
                continue;
            } else {
                status = compareOutputs(pStudents, outputFilePath, i, retCode, &status,array);

            }
        }


    }
}

/**
 * executing the compiled c file and sending the wanted stdin.
 * @param inputFilePath - the location of the file holding the input we want to run.
 * @param args - the command we want to execute.
 * @param retCode
 * @return
 */
int executeCFile(char *inputFilePath, char **args, int retCode,int i,char *outputFileName) {
    int programOutputFD;
    int programInputFD;

    programOutputFD = open(outputFileName, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    programInputFD = open(inputFilePath, O_RDONLY);

    if (programInputFD < 0 || programOutputFD < 0) {
        printError();
        exit(SYSTEM_FAIL);
    }

    retCode = dup2(programInputFD, STDIN_FILENO);
    if(retCode == SYSTEM_FAIL){
        printError();
        exit(SYSTEM_FAIL);
    }

    retCode = dup2(programOutputFD, STDOUT_FILENO);
    if(retCode == SYSTEM_FAIL){
        printError();
        exit(SYSTEM_FAIL);
    }

    retCode = execvp(args[0], &args[0]);
    if(retCode == SYSTEM_FAIL){
        printError();
        exit(SYSTEM_FAIL);
    }

    closeFile(programInputFD);
    closeFile(programOutputFD);
    return retCode;
}

/**
 * the function compares between the wanted result and the result that we got
 * using comp.out.
 * @param pStudents - the array of studentInfo.
 * @param outputFilePath
 * @param i - the number of the student we are currently working on.
 * @param retCode - the return code from the execvp call.
 * @param value - the value that got returned from the execution of comp.out
 * @return
 */
int compareOutputs(studentInfo *pStudents, const char *outputFilePath, int i,
                   int retCode, int *value,char *outputFileName) {
    //preparing an array for the command.
    const char *args[] = {
            "/home/virgoa/Desktop/ex3_os/tal_proj/comp.out",
            outputFileName,
            outputFilePath,
            NULL
    };

    char **array = (char **)malloc(ARRAY_OF_COMMANDS*sizeof(char *));
    for (int i=0;i<ARRAY_OF_COMMANDS-2;i++)
    {
        array[i]=(char*)malloc(STRING_MAX_LENGTH*sizeof(char));
    }
    strCopy(array[0],"/home/virgoa/Desktop/ex3_os/tal_proj/comp.out");
    strCopy(array[1],outputFileName);
    strCopy(array[2],outputFilePath);
    array[3] = NULL;


    pid_t pid = 0;
    pid = fork();
    //executing the program in the child process.
    if (pid == 0) {
        retCode = execvp(args[0], array);
        if(retCode == SYSTEM_FAIL){
            printError();
            exit(SYSTEM_FAIL);
        }
    }else{
        for (int i=0;i<ARRAY_OF_COMMANDS-2;i++)
        {
            free(array[i]);
        }
        free(array);

        waitpid(pid, value, 0);
        if ( WIFEXITED((*value)) ) {
            const int es = WEXITSTATUS((*value));
            if (es == 1){
                gradeStudent(pStudents,i,"60","BAD_OUTPUT");
            }
            if (es == 2){
                gradeStudent(pStudents,i,"80","SIMILAR_OUTPUT");
            }
            if(es == 3){
                gradeStudent(pStudents,i,"100","GREAT_JOB");
            }
        }
        if( unlink(outputFileName) == SYSTEM_FAIL){
            printError();
            exit(SYSTEM_FAIL);
        }
    }
    return (*value);
}

/**
 * the function processes a request to grade a student.
 * @param pStudents - the array of studentInfo.
 * @param i - the students number in the array.
 * @param grade - the grade we want the student to have.
 * @param info - the reasoning behind the grade.
 */
void gradeStudent(studentInfo *pStudents,int i, char *grade, char *info) {
    pStudents[i].isGraded = 1;
    strCopy(pStudents[i].grade, grade);
    strCopy(pStudents[i].info,info);
}

/**
 * the function compiles all the c files submitted by the students.
 * @param submissionsCount - number of submissions.
 * @param myStudents - a pointer to an array holding all the students data.
 */
void compileAllCFiles(int submissionsCount, studentInfo *myStudents) {
    for (int i = 0; i < submissionsCount ; ++i) {
        char num[STRING_MAX_LENGTH];
        char array[STRING_MAX_LENGTH];
        if(myStudents[i].isGraded != 1){
            // defining the arrayu we are going to pass to the execv
            char *args[ARRAY_OF_COMMANDS];
            args[0] = "gcc";
            args[1] ="-o";
            itoa(i,num);
            strCopy(array,"temp");
            strConcatenate(array,num);
            strConcatenate(array,".out");
            strCopy(myStudents[i].compiledFileName,array);
            args[2]=array;
            args[3] = myStudents[i].cFilePath;
            args[4] = NULL;
            //compiling the file.
            executeCommand(args);

            if(checkCompileSuccess(array,myStudents[i].compiledFileName) == 0){
                gradeStudent(myStudents,i,"0","COMPILATION_ERROR");
            }

        }
    }
}

/**
 * the function checks that the c file was successfully compiled.
 * @param compiledFile - pointer to a string containing the c file location.
 * @return - 1 if true, else false.
 */
int checkCompileSuccess(char *compiledFile,char *compiledFilePath) {
    DIR* pDir;
    struct dirent*  entry;
    char path[STRING_MAX_LENGTH];
    getcwd(path, STRING_MAX_LENGTH);

    if ((pDir = opendir(path)) == NULL) {
        printf("Couldn't open current directory\n");
        exit(SYSTEM_FAIL);
    }
    // search for the STUDENT_EXECUTE_FILE_NAME file
    while ((entry = readdir(pDir)) != NULL) {
        if (!strCompare(entry->d_name, compiledFile)) {
            closedir(pDir);
            return 1;
        }
    }
    closedir(pDir);
    return 0;
}

/**
 * the function executes the provided commmand.
 * @param args - an array of strings containing the commands.
 */
void executeCommand(char **args) {

    int stat = 0;
    int retCode = 0;
    pid_t pid = 0;
    pid = fork();
    // if its the child process, execute the command.
    if (pid == 0) {
        retCode = execvp(args[0], &args[0]);
        if (retCode == SYSTEM_FAIL) {
            printError();
            exit(SYSTEM_FAIL);
        }
    } else {
        // if its the father process, wait for the child process to finish.
        waitpid(pid, NULL, WCONTINUED);

    }
}

/**
 * the function finds and returns all the paths to the submitted c files.
 * @param submissionsCount - amounts of submissions to go through.
 * @param myStudents - an array holding all the necessary data.
 */
void findStudentsCFiles(int submissionsCount, studentInfo *myStudents) {
    char *cFilePath = (char *) malloc(STRING_MAX_LENGTH);
    //find all the c files locations.
    for (int i = 0; i < submissionsCount; ++i) {
        strCopy(cFilePath,myStudents[i].dirPath);
        findCFilePath(cFilePath);
        strCopy(myStudents[i].cFilePath,cFilePath);
        if (!string_ends_with(myStudents[i].cFilePath,".c")){
            gradeStudent(myStudents,i,"0","NO_C_FILE");
        }
    }
    free(cFilePath);
}

/**
 * the function assigns the data to the array of students.
 * @param pStudents - a pointer holding an array of studentInfo.
 * @param folders - the path holding all the submissions folders.
 */
void assignFolderAndName(studentInfo *pStudents, char *folders) {
    DIR *pDir;
    int i = 0;
    struct dirent *pDirent;
    if ( (pDir = opendir(folders)) == NULL)
        exit(SYSTEM_FAIL);
    // looping through the directory, printing the directory entry name
    while ( (pDirent = readdir(pDir) ) != NULL ) {
        if(strCompare(pDirent->d_name, ".") && strCompare(pDirent->d_name, "..")){
            strCopy(pStudents[i].name,pDirent->d_name);
            char currentPath[STRING_MAX_LENGTH];
            strCopy(currentPath, folders);
            strConcatenate(currentPath, "/");
            strConcatenate(currentPath, pDirent->d_name);
            strCopy(pStudents[i].dirPath,currentPath);
            i++;
        }
    }
    closedir( pDir );

}

/**
 * the function checks how much submissions there are to process.
 * @param folders - a string holding the location of the submissions.
 * @return
 */
int countSubmittedFolders(char *folders) {
    DIR *pDir;
    int count = 0;
    struct dirent *pDirent;
    if ( (pDir = opendir(folders)) == NULL) {
        printError();
        exit(SYSTEM_FAIL);
    }
    // looping through the directory, printing the directory entry name
    while ( (pDirent = readdir(pDir) ) != NULL ) {
        if(strCompare(pDirent->d_name, ".") && strCompare(pDirent->d_name, "..")){
            count += 1;
        }
    }
    closedir( pDir );
    return count;
}

/**
 * the function reads and processes the configuration file.
 * @param argv - the argv array.
 * @param studentFolders - pointer to a char array that will hold
 * the students folder location.
 * @param testInput - a pointer to a char array that will hold
 * the location of input we would like to run.
 * @param correctOutPut - a pointer to a char array that will hold the correct output.
 */
void readConfigFile(char *const *argv,  char *studentFolders,
                     char *testInput,  char *correctOutPut) {
    //opening the configuration file.
    char *filePath = argv[1];
    int ConfigFile = openFile(filePath,READ_ONLY);

    //read from the config file and get the
    //line #1 - location of students folders.
    readLineFromFile(ConfigFile,studentFolders);
    //line #2 - location of the test input file.
    readLineFromFile(ConfigFile,testInput);
    //line #3 - location of the correct output.
    readLineFromFile(ConfigFile,correctOutPut);


    //closing the configuration file.
    closeFile(ConfigFile);
}

/**
 * the function closes the passed file.
 * @param file - the file we would like to close.
 */
void closeFile(int file) {
    //closing file One.
    if (close(file) == SYSTEM_FAIL) {
        printError();
        exit(SYSTEM_FAIL);
    }
}

/**
 * the function reads one line from the passed file.
 * @param file - the file we want to read from.
 * @param array - the array of chars we want to copy the line to.
 */
void readLineFromFile(int file, char *array) {
    int bytes_read;
    char buffer[CONFIG_MAX_LENGTH + 1];
    int k = 0;
    do {
        char t = 0;

        bytes_read = read(file, &t, 1);
        //checking if there was a problem reading from file.
        if (bytes_read < 0) {
            printError();
            exit(SYSTEM_FAIL);
        }

        buffer[k] = t;
        k++;

        if(t == '\n' || t == '\0') {
            buffer[k-1] = '\0';
            break;
        }
    }
    while (bytes_read != 0);
    strCopy(array,buffer);
    return;

}

/**
 * the function checks if the passed string ends with provided suffix.
 * @param str - the string we would like to check.
 * @param suffix - the suffix we are looking for.
 * @return - 1 it true, else false.
 */
int string_ends_with(char * str, char * suffix)
{
    int str_len = strLength(str);
    int suffix_len = strLength(suffix);

    return
            (str_len >= suffix_len) &&
            (0 == strCompare(str + (str_len - suffix_len), suffix));
}

/**
 * itoa function that processes numbers to strings.
 * @param i - the number we want to process.
 * @param b - the location we would like to save the string to.
 * @return - pointer to an array of chars holding the number.
 */
char* itoa(int i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return b;
}

/**
 * the function prints the error in sys-call message.
 */
void printError() {
    write(STDERR, ERROR, sizeof(ERROR));
}

/**
 * The function copies a char array to another.
 * @param dest - the char array we want to copy to.
 * @param src - the char array we want to copy from.
 * @return - a pointer to the dest array.
 */
char *strCopy(char *dest, const char *src)
{
    unsigned i;
    for (i=0; src[i] != '\0'; ++i)
        dest[i] = src[i];

    //Ensure trailing null byte is copied
    dest[i]= '\0';

    return dest;
}

/**
 * Implementation of strcat.
 * @param dest - the dest char array.
 * @param src - the string we want to append to dest.
 * @return .
 */
char *strConcatenate(char *dest, const char *src)
{
    int i,j;
    for (i = 0; dest[i] != '\0'; i++){
    }
    for (j = 0; src[j] != '\0'; j++){
        dest[i+j] = src[j];
    }
    dest[i+j] = '\0';
    return dest;
}


/**
 * The function compares between two strings.
 * @param string1
 * @param string2
 * @return returns 0 if both strings are equal, num else.
 */
int strCompare(char *string1, char *string2)
{
    for (int i = 0; ; i++)
    {
        if (string1[i] != string2[i])
        {
            return string1[i] < string2[i] ? -1 : 1;
        }

        if (string1[i] == '\0')
        {
            return 0;
        }
    }
}

/**
 * the function returns the length of a string.
 * @param s
 * @return
 */
unsigned int strLength(const char *s)
{
    int l;for(l=0;s[l]!='\0';l++);return l;
}

/**
 * The function checks if we got a config file to work with.
 * @param argc - number of command line arguments.
 * @return - 1 if config file is supplied | else - exit(-1).
 */
int insufArgs(int argc){
    if(argc > 2){
        fprintf(stderr, "%s", "Too many arguments supplied.\n");
        exit(SYSTEM_FAIL);
    }
    else if(argc < 2){
        fprintf(stderr, "%s", "Config File Expected.\n");
        exit(SYSTEM_FAIL);
    }
    return 1;
}


/**
 * The function opens a file according to the passed mode.
 * @param filePath - the file path.
 * @param selectedMode - mode.
 * @return - the file
 */
int openFile(const char *filePath, int selectedMode) {
    int file;
    switch(selectedMode) {
        case READ_ONLY :
            file = open(filePath, O_RDONLY);
            if (file < 0) {
                printf("File doesn't exist.\n");
                printError();
                //exiting the program.
                exit(SYSTEM_FAIL);
            }
            break;
        case CREATE_FILE :
            file = open(filePath, O_CREAT, O_WRONLY);
            if(file > 0) {
                break;
            }
        case WRITE_ONLY :
            file = open(filePath, O_WRONLY);
            break;

        case ADD_LINE :
            file = open(filePath, O_APPEND);
            break;
    }

    return file;
}

/**
 * the function runs through provided folders looking for a c file.
 * @param cpath - an array that will hold the path to the found c file.
 * @return - a pointer to a char array.
 */
char* findCFilePath(char *cpath) {
    DIR* dip;
    struct dirent* dit;
    if((dip=opendir(cpath))==NULL){
        printError();
        exit(SYSTEM_FAIL);
    }
    //read from the dir
    while ((dit=readdir(dip))!=NULL) {

        if (dit->d_type == DT_REG) {
            if (string_ends_with(dit->d_name,".c")) {
                strConcatenate(cpath,"/");
                strConcatenate(cpath,dit->d_name);
                return cpath;
            }

        } else if (strCompare(dit->d_name, ".") == 0 && strCompare(dit->d_name, "..") == 0) {
            //concat the subDir path

            strConcatenate(cpath,"/");
            strConcatenate(cpath,dit->d_name);
            return findCFilePath(cpath);
        }

    }
}

/**
 * the function writes the students name,grade,etc to a csv file.
 * @param pStudents - the array containing the grades/names/info of the students.
 * @param count - the amount of submissions we have to write to file.
 */
void writeToCSV(studentInfo *pStudents, int submissionsCount) {
    int resultsCsvFD;
    int checkWrite;
    int fileOpenCheck = resultsCsvFD=open("results.csv",O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if(fileOpenCheck == SYSTEM_FAIL)
    {
        printError();
        exit(SYSTEM_FAIL);
    }
    for (int i = 0; i < submissionsCount; ++i) {
        char stringToFile[STRING_MAX_LENGTH] = {0};
        char *string = studentToString(pStudents, i);
        strCopy(stringToFile,string);
        checkWrite = write(resultsCsvFD, stringToFile, STRING_MAX_LENGTH);
        if (checkWrite < 0){
            printError();
            closeFile(resultsCsvFD);
            exit(SYSTEM_FAIL);
        }
        free(string);
    }
    closeFile(resultsCsvFD);
}

char *studentToString(studentInfo *pStudents, int i) {
    char *string = malloc(STRING_MAX_LENGTH);
    strCopy(string,pStudents[i].name);
    strConcatenate(string,",");
    strConcatenate(string,pStudents[i].grade);
    strConcatenate(string,",");
    strConcatenate(string,pStudents[i].info);
    strConcatenate(string, "\n");
    strConcatenate(string,"\0");
    return string;
}
