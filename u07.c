#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// count lines in file
int countLines(FILE *file) {
    int count = 0;
    char buffer[1000];
    while (fgets(buffer, sizeof(buffer), file)) {
        count++;
    }
    rewind(file); // reset file pointer to beginning of file
    return count;
}

// CSV files are read and a new file is created with the differences between the temperature values
void processCSV(const char *filename, int pos, int begin) { // pos = column of temperature values, begin = start the data
    char buffer[1000];
    char *token;
    int limit;
    int column;

    limit = 0; 

    // open file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }

    int numLines = countLines(file); // count lines in file

    // generate array for temperature values
    float *temp = malloc(numLines * sizeof(float)); // allocate memory for temperature values
    if (temp == NULL) {
        printf("Fehler bei der Speicherzuweisung\n");
        fclose(file);
        return;
    }

    // generate array for temperature differences
    float *tempd = malloc(numLines * sizeof(float));
    if (tempd == NULL) {
        printf("Fehler bei der Speicherzuweisung\n");
        free(temp);
        fclose(file);
        return;
    }

    printf("I was able to open the file\n");

    // read file
    #pragma omp parallel for // parallelize reading of file
    for (int i = 0; i < numLines; i++) {
        fgets(buffer, sizeof(buffer), file);
        column = 0; 

        // get first token
        token = strtok(buffer, "\t");

        while (token != NULL) {
            if ((column == pos) && (begin >= 0)) { 
                temp[begin] = atof(token);
                break;
            }
            token = strtok(NULL, "\t"); // get next token
            column++;
        }

        begin++;
        limit++;
    }

    fclose(file);

    printf("I was able to read the file\n");

    // calculate difference
    for (int i = 1; i < numLines; i++) {
        tempd[i] = temp[i] - temp[i-1];
    }

    // write to file
    FILE *file1 = fopen(filename, "w");
    if (file1 == NULL) {
        printf("Error opening file\n");
        free(temp);
        free(tempd);
        return;
    }

    // write differences to file
    for (int i = 1; i < numLines - 26; i++) {
        fprintf(file1, "%.1f\n", tempd[i]);
    }

    // unlock memory
    free(temp);
    free(tempd);
    fclose(file1);

    printf("I was able to write the differences\n");
}

int main() {
    // hier könneten die anderen Datein noch eingefügt werden, mit den entsprechenden Parametern
    #pragma omp parallel sections // parallelize processing of files
    {
    
        #pragma omp section
        processCSV("02_Gatow-1_2022.csv", 8, -25);
        #pragma omp section
        processCSV("04_Botanischer-Garten-1_2022.csv", 8, -49);
    }
    
    return 0;
}
