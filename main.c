#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

//function prototypes
int randomInRange(int min, int max);
char * chooseRandomWord(char dictArray[DICT_SIZE][WORD_SIZE + 1]);
char * getPlayerGuess(char dictArray[DICT_SIZE][WORD_SIZE + 1], int hardMode, char guessedLetterArray[WORD_SIZE + 1], char wrongLetterArray[27], int comparisonArray[WORD_SIZE], char randWord[WORD_SIZE+1]);
void compareWords(int comparisonArray[WORD_SIZE], char guessedLetterArray[WORD_SIZE], char wrongLetterArray[27], char word1[WORD_SIZE + 1], char word2[WORD_SIZE + 1]);
int checkForWin(int comparisonArray[WORD_SIZE]);
void printGameState(int comparisonArray[WORD_SIZE], char guessedLetterArray[WORD_SIZE], char wrongLetterArray[WORD_SIZE], char randWord[WORD_SIZE+1], int guessCounter);
int checkStringInArray(char dictArray[DICT_SIZE][WORD_SIZE + 1], char guessedWord[WORD_SIZE + 1]);
void printHistogram(int intArray[11], int gameCount);
int getStringLength(const char *str);
float AvgLettersPerRound(int lettersPerRound[], int gameCount);

#define MAX_GAMES 100

//main function to control program flow
int main() {
    // load dictionary of words
    char dictionary[DICT_SIZE][WORD_SIZE + 1];
    load_word_list(dictionary);

    //initialize variables
    int gameCount = 0;
    int winCount = 0;
    int guessesPerRound[MAX_GAMES] = {0};
    int lettersPerRound[MAX_GAMES] = {0};
    int allowedGuesses = 6;
    int hardModeEnabled = 0;

    //main game loop
    while (1) {
        //allow stats checks from game 2 on
        if (gameCount > 1) {
            printf("\nWould you like to see your statistics? Type 'Y' for yes or 'N' for no: ");
            char Continue = getchar();
            getchar();

            //calculate stats
            if (Continue == 'y' || Continue == 'Y') {
                printf("\nYou have won %c%.2f of your games", 37, (((float)winCount/(float)gameCount)*(float)100));
                printf("\nThe average number of letters guessed per round is: %.2f", AvgLettersPerRound(lettersPerRound, gameCount));
                printf("\n\n");

                //print histogram graph
                printHistogram(guessesPerRound, gameCount);
            }
        }


        //enable or disable hard mode, which adds restrictions to the game
        printf("\nDo You want to play in Hard mode? Type 'Y' for yes or 'N' for no: ");
        char hardModeCheck = getchar();
        getchar();
        if (hardModeCheck == 'y' || hardModeCheck == 'Y') {
            printf("\nHard mode Enabled\n");
            hardModeEnabled = 1;
        }
        else{
            hardModeEnabled = 0;
            printf("\nNormal Mode Enabled\n");

        }


        //Select a random word

        // Seed the random number generator with the current time
        srand(time(NULL));
        char *randWord;
        randWord = chooseRandomWord(dictionary);

//        // test selection of random word by printing it
//        printf("\n");
//        for(int i = 0; i < WORD_SIZE; i++){
//            printf("%c", randWord[i]);
//        }
//        printf("\n");

        // create comparison array to determine which guesses are correct
        // this will be used to determine which characters to show when printing word
        // index filled with 0 are not guessed, index with 1 are correctly guessed
        int comparisonArray[WORD_SIZE] = {0};

        //initialize non-global variables/arrays within loop
        char guessedLetterArray[WORD_SIZE + 1] = {0};
        char wrongLetterArray[27] = {0};
        guessedLetterArray[0] = '\0';
        wrongLetterArray[0] = '\0';
        int guessCounter = allowedGuesses;
        int winState = 0;

        while (guessCounter > 0) {

            // call for user input
            char *playerGuess;
            int loopState = 0;
            while (loopState == 0) {
                //call player guess function to retrieve the word the user is guessing
                playerGuess = getPlayerGuess(dictionary, hardModeEnabled, guessedLetterArray, wrongLetterArray, comparisonArray, randWord);
                if (checkStringInArray(dictionary, playerGuess) == 1) {
                    loopState = 1;
                } else {
                    printf("\n%s is not a valid word. ", playerGuess);
                }
            }

            //call function to compare the words, and update arrays as appropriate
            compareWords(comparisonArray, guessedLetterArray, wrongLetterArray, playerGuess, randWord);

            //call check for win function to see if the game is in a winning state
            if (checkForWin(comparisonArray)) {
                printf("You win! the word was %s", randWord);
                winState = 1;
                winCount++;
                guessCounter--;
                break;
            } else {
                printf("Wrong: ");
                guessCounter--;

                //print the current state of the game
                printGameState(comparisonArray, guessedLetterArray, wrongLetterArray, randWord, guessCounter);
            }
            //free variable previously assigned with malloc
            free(playerGuess);
        }

        //if game ends with winstate 0, this means the player lost
        if (winState == 0) {
            printf("You lose! the word was %s", randWord);
        }

        //check if player wants another round
        printf("\nDo you want to continue? Type 'Y' for yes or 'N' for no: ");
        char Continue = getchar();
        getchar();

        //free variable previously assigned with malloc
        free(randWord);


        //collect data for statistics
        if (Continue == 'y' || Continue == 'Y') {
            guessesPerRound[gameCount] = (allowedGuesses-guessCounter);
            lettersPerRound[gameCount] = (getStringLength(guessedLetterArray) + getStringLength(wrongLetterArray));
            gameCount++;
            continue;
        }
        else {
            break;
        }


    }

    return 0;
}


int randomInRange(int min, int max) {
    // Generate a random number within the range and return it
    return (rand() % (max - min + 1)) + min;
}


char * chooseRandomWord(char dictArray[DICT_SIZE][WORD_SIZE + 1]) {
    //select a random word
    int randint = randomInRange(1, DICT_SIZE);

    char *randWord = (char*) malloc(sizeof(char) * (WORD_SIZE+1));

//    printf("%d", randint);
    for(int i = 0; i < WORD_SIZE +1; i++){
        randWord[i] = dictArray[randint][i];
    }
    return randWord;
}


char * getPlayerGuess(char dictArray[DICT_SIZE][WORD_SIZE + 1], int hardMode, char guessedLetterArray[WORD_SIZE + 1], char wrongLetterArray[27], int comparisonArray[WORD_SIZE], char randWord[WORD_SIZE+1]){
    //A function to retrieve and return a guess from the user's keyboard and ensure that it is valid against all rules
    char *strArray = (char*) malloc(sizeof(char) * (WORD_SIZE+1));
    printf("Enter a %d character word: \n", WORD_SIZE);
    char cursor = getchar();
    int correctCharCount = 0;
    char verifiedGuessedLetters[WORD_SIZE + 1] = {0};
    verifiedGuessedLetters[0] = '\0';


    for (int i = 0; i < WORD_SIZE; i++){
        //check if character is a letter
        if ((cursor >= 'A' && cursor <= 'Z') || (cursor >= 'a' && cursor <= 'z') || (cursor == '\n' || cursor == EOF)) {

            //if guess encounters end of frame before reaching word size, guess is too short, restart loop
            if (cursor == '\n' || cursor == EOF) {
                printf("\nThat guess was not long enough, please enter a %d character guess: \n", WORD_SIZE);
                i = -1;
            }

            //convert uppercase to lowercase
            else {
                if (cursor >= 'A' && cursor <= 'Z') {
                    cursor += 32;
                }

                //add letter to array
                strArray[i] = cursor;
            }
        }

        //if not a letter, it is not a valid guess, restart loop
        else {
            printf("\nThat guess contained a character that was not a letter, please enter a %d character guess using only letters: \n", WORD_SIZE);
            while ((cursor = getchar()) != '\n' && cursor != EOF) {}
            i = -1;
        }

        //variable used to measure the length of the string of guessed letters, used in hard mode checks
        int guessedLetterArrayLength = getStringLength(guessedLetterArray);
        //only apply the following checks when any characters have been guessed
        if (guessedLetterArrayLength > 0){

            //when hardmode is enabled
            if (hardMode == 1) {
                //iterate through array of guessed letters, to see if cursor is in string
                for (int j = 0; j < guessedLetterArrayLength; j++) {
                    if (cursor == guessedLetterArray[j]) {

                        //ensure each letter is only counted once on multiple loops,
                        // or if in the same letter appears more than once per word
                        int verifiedGLLength = getStringLength(verifiedGuessedLetters - 1);
                        int verfiedGLInString = 0;

                        for (int k = 0; k < verifiedGuessedLetters; k++) {
                            if (cursor == verifiedGuessedLetters[j]) {
                                verfiedGLInString = 1;
                            }
                        }
                        if (verfiedGLInString != 1) {
                            correctCharCount++;
                        }

                        verifiedGuessedLetters[verifiedGLLength - 1] = cursor;
                        verifiedGuessedLetters[verifiedGLLength] = '\0';

                    }
                }

                //if fewer correct letters were guessed than the total count of corretly guessed characters,
                //at least one must be missing
                //in hard mode this guess is invalid. Restart the loop.
                if ((guessedLetterArrayLength != correctCharCount) && (i == WORD_SIZE-1)) {
                    printf("\nThat guess did not contain a previously guessed correct letter. Please enter a word which contains all guessed letters: \n",
                           WORD_SIZE);
                    while ((cursor = getchar()) != '\n' && cursor != EOF) {}
                    i = -1;
                }

                //if a letter in a previous guess was ruled out, it cannot be used in future guesses in hard mode.
                //guess is invalid, restart loop.
                for (int l = 0; l<27; l++){
                    if (cursor == wrongLetterArray[l]) {
                        printf("\nThat guess contained a previously guessed incorrect letter. Please enter a word which contains all guessed letters: \n",
                               WORD_SIZE);
                        while ((cursor = getchar()) != '\n' && cursor != EOF) {}
                        i = -1;
                    }
                }

                // if letters were guessed in the correct position previously, it is invalid to guess them in the incorrect position.
                //restart loop
                if ((cursor != randWord[i]) && (comparisonArray[i] == 1)){
                    printf("\nThat guess contained incorrect letters in positions which were previously guessed correctly. Please enter a word which contains all guessed letters: \n",
                           WORD_SIZE);
                    while ((cursor = getchar()) != '\n' && cursor != EOF) {}
                    i = -1;
                }

            }
        }
        cursor = getchar();

    }
    strArray[WORD_SIZE] = '\0';
    return strArray;
}


void compareWords(int compArray[WORD_SIZE], char guessedLetterArray[WORD_SIZE+1], char wrongLetterArray[27], char word1[WORD_SIZE + 1], char word2[WORD_SIZE + 1]) {
    //compare for exact guesses
    for (int h = 0; h < WORD_SIZE; h++) {
        if (word1[h] == word2[h]) {
            compArray[h] = 1;
        }
    }

    //compare for non-exact guesses and add correct letters to array if not already existing
    for (int i = 0; i < WORD_SIZE; i++) {
        for (int j = 0; j < WORD_SIZE; j++) {
            if (word1[i] == word2[j]) {
                for (int k = 0; k < 27; k++) {
                    //exit loop if the letter already exists
                    if (word1[i] == guessedLetterArray[k]) {
                        break;
                    }
                        // if it wasn't in the array, loop to end of array, signified by '\0'
                    else if (guessedLetterArray[k] == '\0') {
                        guessedLetterArray[k] = word1[i];
                        guessedLetterArray[k + 1] = '\0';
                        break;
                    }
                }
            }
        }
    }
    // Check for unique wrong guesses and add them to wrongLetterArray.
    for (int i = 0; i < WORD_SIZE; i++) {
        int is_wrong_guess = 1;
        for (int j = 0; j < WORD_SIZE; j++) {
            if (word1[i] == word2[j]) {
                is_wrong_guess = 0;
                break;
            }
        }

        if (is_wrong_guess) {
            int letter_already_exists = 0;
            for (int k = 0; k < 27; k++) {
                if (word1[i] == wrongLetterArray[k]) {
                    letter_already_exists = 1;
                    break;
                }
            }

            if (!letter_already_exists) {
                for (int k = 0; k < 27; k++) {
                    if (wrongLetterArray[k] == '\0') {
                        wrongLetterArray[k] = word1[i];
                        wrongLetterArray[k + 1] = '\0';
                        break;
                    }
                }
            }
        }
    }
}


int checkForWin(int comparisonArray[WORD_SIZE]) {
    //check if any entry in comparison array is 0. If any 0's are present, player has not won
    for (int i = 0; i < WORD_SIZE; i++) {
        if (comparisonArray[i]==0){
            return 0;
        }
    }
    return 1;
}


void printGameState(int comparisonArray[WORD_SIZE], char guessedLetterArray[WORD_SIZE + 1], char wrongLetterArray[WORD_SIZE], char randWord[WORD_SIZE+1], int guessCounter){
    //a function to print the current state of the game
    printf("\n");

    //ensure only letters guessed in correct positions are printed, the remaining letters are printed as dashes
    for(int i = 0; i<WORD_SIZE; i++){
        if (comparisonArray[i] == 1){
            printf("%c", randWord[i]);
        }
        else {
            printf("-");
        }
    }

    printf("\nYou Have %d guesses remaining", guessCounter);
    printf("\nCorrectly guessed letters: %s", guessedLetterArray);
    printf("\nincorrectly guessed letters: %s\n\n", wrongLetterArray);
}


int checkStringInArray(char dictArray[DICT_SIZE][WORD_SIZE + 1], char guessedWord[WORD_SIZE + 1]){
    //determine if the string is in the dictionary, to determine guess validity
    int i, j;
    for (i = 0; i<DICT_SIZE; i++){
        for (j = 0; (dictArray[i][j] != '\0') || (guessedWord[j] != '\0'); j ++){
            if (dictArray[i][j] != guessedWord[j]){
                break;
            }
        }
        if ((dictArray[i][j] == '\0') && (guessedWord[j] == '\0')) {
            return 1;
        }
    }
    return 0;
}

void printHistogram(int intArray[], int gameCount){
    //find max value
    int maxValue = 0;
    for (int i = 0; i < gameCount; i++) {
        if (intArray[i] > maxValue) {
            maxValue = intArray[i];
        }
    }

    //print graph
    printf("\nNumber of Guesses: \n");
    for (int i = maxValue; i > 0; i--) {
        for (int j = 0; j < gameCount; j++) {
            if (intArray[j] >= i) {
                printf("    #    ");
            } else {
                printf("         ");
            }
        }
        printf("\n");
    }

    int lineNumber = (gameCount * 7);
    for(int i = 0; i < lineNumber; i++) putchar('_');

    printf("\n");
    for (int i = 0; i < gameCount; i++) {
        printf(" game %d  ", i+1);
    }

        printf("\n\n");
}

int getStringLength(const char *str) {
    //find the length of a string (array of characters ending in '\0'
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

float AvgLettersPerRound(int lettersPerRound[], int gameCount) {
    //determine the average number of letters guessed per round.
    int sum = 0;
    for (int i = 0; i < gameCount; i++) {
        sum += lettersPerRound[i];
    }
    return (float)sum / (float)gameCount;
}