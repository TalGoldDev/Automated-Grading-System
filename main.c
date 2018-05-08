
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>

#define STDERR 2
#define CONFIG_MAX_LENGTH 160
#define ERROR "Error in system call\n"

#define WRITE_ONLY 1
#define READ_ONLY 2
#define CREATE_FILE 3
#define ADD_LINE 4

//grading
#define NO_C_FILE 0
#define COMPILATION_ERROR 1
#define TIME_OUT 2
#define BAD_INPUT 3
#define SIMILAR_OUTPUT 4
#define GREAT_JOB 5


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
char *strcpy(char *dest, const char *src)
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
char *strcat(char *dest, const char *src)
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
 * The function checks if we got a config file to work with.
 * @param argc - number of command line arguments.
 * @return - 1 if config file is supplied | else - exit(-1).
 */
int insufArgs(int argc){
    if(argc > 2){
        fprintf(stderr, "%s", "Too many arguments supplied.\n");
        exit(-1);
    }
    else if(argc < 2){
        fprintf(stderr, "%s", "Config File Expected.\n");
        exit(-1);
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
                exit(-1);
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

int cd(char *path) {
    return chdir(path);
}

void rankStudent(int grade,char* name[]);


void readLineFromFile(int file, char *array);

void closeFile(int file);

int main(int argc, char *argv[]) {
    //checking for a config file as a command line input.
    insufArgs(argc);

    //creating the results CSV file.
    char resultCSV[] = "./results.csv";
    int resultCSVFile = openFile(resultCSV,CREATE_FILE);

    //opening the configuration file.
    char *filePath = argv[1];
    int ConfigFile = openFile(filePath,READ_ONLY);

    //the location of the folders containing the c files.
    char studentFolders[CONFIG_MAX_LENGTH + 1];
    //the location of the inputs that we would like to run.
    char testInput[CONFIG_MAX_LENGTH + 1];
    //the location of the text file containing the correct output.
    char correctOutPut[CONFIG_MAX_LENGTH + 1];

    //read from the config file and get the
    //line #1 - location of students folders.
    readLineFromFile(ConfigFile,studentFolders);
    //line #2 - location of the test input file.
    readLineFromFile(ConfigFile,testInput);
    //line #3 - location of the correct output.
    readLineFromFile(ConfigFile,correctOutPut);


    //go over all the folders in the studentFolder.

    // code template listing all the files in a folder :
    //    DIR *pDir;
    //    struct dirent *pDirent;
    //    if ( (pDir = opendir(studentFolders)) == NULL)
    //        exit(1);
    //    // looping through the directory, printing the directory entry name
    //    while ( (pDirent = readdir(pDir) ) != NULL )
    //        printf( "%s\n", pDirent ->d_name );
    //    closedir( pDir );

    //find all c files in a student folder.

    //compile c files -- using execvp.

    //run c file using given inputs and check the results.

    //comapre results to result file using ex31

    //write the score according to result.




    //closing the configuration file.
    closeFile(ConfigFile);
    closeFile(resultCSVFile);
    return 0;
}

void closeFile(int file) {
    //closing file One.
    if (close(file) == -1) {
        printError();
        exit(-1);
    }
}

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
            exit(-1);
        }

        buffer[k] = t;
        k++;

        if(t == '\n' || t == '\0') {
            buffer[k-1] = '\0';
            break;
        }
    }
    while (bytes_read != 0);
    strcpy(array,buffer);
    return;

}


/**
 * The function takes a grade and the students name
 * and writes it down in the results.csv file.
 * @param grade - the students grade.
 * @param name - the students name.
 */
void rankStudent(int grade,char* name[]) {
    switch(grade) {
        case NO_C_FILE :

            break;
        case COMPILATION_ERROR :

            break;
        case TIME_OUT :

            break;
        case BAD_INPUT :

            break;
        case SIMILAR_OUTPUT :

            break;
        case GREAT_JOB :

            break;
        default :
            printf("Invalid grade\n" );
    }
}
