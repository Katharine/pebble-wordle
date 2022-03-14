#ifndef MODEL_H
#define MODEL_H

#define WORD_LENGTH 5

enum LetterStatus {
	LetterStatusNeutral,
	LetterStatusNotPresent,
	LetterStatusWrongPosition,
	LetterStatusCorrect,
	LetterStatusCount,
};
typedef enum LetterStatus LetterStatus;

bool is_valid_word(char word[WORD_LENGTH]);
bool score_word(char word[WORD_LENGTH], char expected[WORD_LENGTH], LetterStatus result[WORD_LENGTH]);
void word_of_the_day(char word[WORD_LENGTH]);
int wordle_number();

#endif