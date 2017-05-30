// main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <math.h>


#define kNumNonStandardChars  6  // Number of Turkish-only letters in the Turkish alphabet 
#define kNumTotalChars       29  // Total number of letters in the Turkish alphabet

// Pauses the execution before exiting the program.
int exitProgram(int exitCode) {
	system("PAUSE");
	exit(exitCode);
}

// Struct that holds the read characters and character pairs.
typedef struct CharTable {
	// Array that contains the number of characters read for each character.
	int *letterCount;

	// 2-D array that contains the number of character pairs read for each character pair.
	int **jointLetterCount;

	// 3-D array that contains the number of character triplets read of each character triplet.
	int ***tripletLetterCount;
	
	// Number of total characters stored in the table.
	int size;

	// Number of total character pairs stored in the table.
	int pairSize;

	// Number of total character triplets stored in the table.
	int tripletSize;

	// Array that holds ISO-8859-9 character codes for Turkish-specific characters (both lowercase and uppercase).
	unsigned char isoCodes[kNumNonStandardChars][2];

	// Array that holds all letters in the Turkish alphabet.
	unsigned char letters[kNumTotalChars];

} CharTable;

// Calculates and returns the index assigned to the given character.
// e.g. a = 0, b = 1, c = 2, etc.
// If character is not valid, returns -1.
int letterIndex(unsigned char lowercaseLetter) {
	switch (lowercaseLetter) {
	case 0xe7:
		// c
		return 3;
	case 0xf0:
		// g
		return 8;
	case 0xfd:
		// i
		return 10;
	case 0xf6:
		// o
		return 18;
	case 0xfe:
		// s
		return 22;
	case 0xfc:
		// u
		return 25;
	}

	if (lowercaseLetter < 'a' || lowercaseLetter > 'z') {
		return -1;
	}

	int extraChars = 0;
	if (lowercaseLetter <= 'c') {
		extraChars = 0;
	} else if (lowercaseLetter <= 'g') {
		extraChars = 1;
	} else if (lowercaseLetter <= 'h') {
		extraChars = 2;
	} else if (lowercaseLetter <= 'o') {
		extraChars = 3;
	} else if (lowercaseLetter <= 's') {
		extraChars = 4;
	} else if (lowercaseLetter <= 'u') {
		extraChars = 5;
	} else if (lowercaseLetter <= 'z') {
		extraChars = 6;
	}

	if (lowercaseLetter > 'q') {
		extraChars--;
	}
	if (lowercaseLetter > 'x') {
		extraChars -= 2;
	}

	return (lowercaseLetter - 'a' + extraChars);
}

// Initializes and returns a table.
CharTable *tableInit() {
	CharTable *table = (CharTable *)malloc(sizeof(CharTable));

	const char isoCodes[6][2] = {
		{ '\xf0', '\xd0' },
		{ '\xfd', '\xdd' },
		{ '\xf6', '\xd6' },
		{ '\xfc', '\xdc' },
		{ '\xfe', '\xde' },
		{ '\xe7', '\xc7' },
	};
	for (int i = 0; i < kNumNonStandardChars; i++) {
		for (int j = 0; j < 2; j++) {
			table->isoCodes[i][j] = isoCodes[i][j];
		}
	}

	// All letters in the Turkish alphabet. This alphabet contains Turkish-only characters 
	// but excludes English-only characters (q, w, x).
	const char *alphabet = "abc\u00e7defg\u00f0h\u00fdijklmno\u00f6prs\u00fetu\u00fcvyz";
	int numChars = (int)strlen(alphabet);
	for (int i = 0; i < numChars; i++) {
		table->letters[i] = alphabet[i];
	}

	table->size = 0;
	table->pairSize = 0;
	table->tripletSize = 0;
	table->letterCount = (int *)malloc(kNumTotalChars * sizeof(int));
	for (int i = 0; i < kNumTotalChars; i++) {
		table->letterCount[i] = 0;
	}

	table->jointLetterCount = (int **)malloc(kNumTotalChars * sizeof(int *));
	for (int i = 0; i < kNumTotalChars; i++) {
		// calloc() will initialize all elements to 0.
		table->jointLetterCount[i] = (int *)calloc(kNumTotalChars, sizeof(int));
	}

	table->tripletLetterCount = (int ***)malloc(kNumTotalChars * sizeof(int **));
	for (int i = 0; i < kNumTotalChars; i++) {
		table->tripletLetterCount[i] = (int **)malloc(kNumTotalChars * sizeof(int *));
		for (int j = 0; j < kNumTotalChars; j++) {
			// calloc() will initialize all elements to 0.
			table->tripletLetterCount[i][j] = (int *)calloc(kNumTotalChars, sizeof(int));
		}
	}

	return table;
}

// Returns whether the given letter is a valid character in the Turkish alphabet.
// Letters like 'd', 'k', 'ÄŸ' are valid; all other characters (English-only letters,
// punctuation, whitespace, non-printable characters, etc) are invalid.
int isValidCharacter(CharTable *table, unsigned char letter) {
	if (!table) {
		fprintf(stderr, "Table is not initialized.\n");
		return 0;
	}

	if (letter >= 'a' && letter <= 'z') {
		if (letter != 'q' && letter != 'w' && letter != 'x') {
			return 1;
		}
	}
	else if (letter >= 'A' && letter <= 'Z') {
		if (letter != 'Q' && letter != 'W' && letter != 'X') {
			return 1;
		}
	}
	else {
		for (int i = 0; i < kNumNonStandardChars; i++) {
			if (letter == table->isoCodes[i][0] || letter == table->isoCodes[i][1]) {
				return 1;
			}
		}
	}

	return 0;
}

// If the given letter is a valid character in the Turkish alphabet, converts it to lowercase
// and returns it. Otherwise, returns 0x00.
unsigned char toLowercase(CharTable *table, unsigned char letter) {
	if (letter >= 'a' && letter <= 'z' && letter != 'q' && letter != 'w' && letter != 'x') {
		return letter;
	}
	else if (letter >= 'A' && letter <= 'Z' && letter != 'Q' && letter != 'W' && letter != 'X') {
		return letter + ('a' - 'A');
	}
	else {
		for (int i = 0; i < kNumNonStandardChars; i++) {
			if (letter == table->isoCodes[i][0] || letter == table->isoCodes[i][1]) {
				return table->isoCodes[i][0];
			}
		}
	}
	return 0x00;
}

// If the given character is valid, adds it to the table and returns 1.
// Otherwise, it doesn't modify the table and returns 0.
int tableAddChar(CharTable *table, unsigned char letter) {
	if (!table) {
		return 0;
	}

	unsigned char lowercaseLetter = toLowercase(table, letter);
	int index = letterIndex(lowercaseLetter);

	if (index >= 0) {
		table->letterCount[index]++;
		table->size++;
		return 1;
	}
	else {
		return 0;
	}
}

// Adds a pair of letters to the table, to be used in joint probability calculations.
void tableAddCharPair(CharTable *table, unsigned char letter1, unsigned char letter2) {
	if (!table) {
		return;
	}

	unsigned char lowercaseLetter1 = toLowercase(table, letter1);
	unsigned char lowercaseLetter2 = toLowercase(table, letter2);

	if (lowercaseLetter1 != 0x00 && lowercaseLetter2 != 0x00) {
		// Add the pair only if both letters are valid.
		int index1 = letterIndex(lowercaseLetter1);
		int index2 = letterIndex(lowercaseLetter2);
		table->jointLetterCount[index1][index2]++;
		table->pairSize++;
	}
}

// Adds a triplet of letters to the table, to be used in joint probability calculations.
void tableAddCharTriplet(CharTable *table, unsigned char letter1, unsigned char letter2, unsigned char letter3) {
	if (!table) {
		return;
	}

	unsigned char lowercaseLetter1 = toLowercase(table, letter1);
	unsigned char lowercaseLetter2 = toLowercase(table, letter2);
	unsigned char lowercaseLetter3 = toLowercase(table, letter3);

	if (lowercaseLetter1 != 0x00 && lowercaseLetter2 != 0x00 && lowercaseLetter3 != 0x00) {
		// Add the pair only if both letters are valid.
		int index1 = letterIndex(lowercaseLetter1);
		int index2 = letterIndex(lowercaseLetter2);
		int index3 = letterIndex(lowercaseLetter3);
		table->tripletLetterCount[index1][index2][index3]++;
		table->tripletSize++;
	}
}

// Calculates log-base-2 of the given number.
double Log2(double value) {
	return log(value) / log(2.0);
}

// Returns the number of characters needed to store the value as a string.
int widthOf(int value) {
	if (value < 0) {
		// This function should only be used for non-negative values.
		return 0;
	}
	else if (value == 0) {
		return 1;
	}
	else {
		int width = 0;
		for (; value > 0; width++) {
			value /= 10;
		}
		return width;
	}
}

// Returns a string containing only whitespace specified by the width.
// Used to vertically align fields in the output.
const char *whitespace(int width) {
	if (width < 0) {
		width = 0;
	}

	char *buffer = (char *)calloc(width + 1, sizeof(char));  // +1 for the terminating \0 character.
	for (int i = 0; i < width; i++) {
		buffer[i] = ' ';
	}
	return buffer;
}

// Calculates and returns the entropy of the text represented by the given char table.
double calculateEntropy(CharTable *table) {
	if (!table) {
		return -1;
	}

	double sum = 0;
	for (int i = 0; i < kNumTotalChars; i++) {
		double probability = (double)table->letterCount[i] / (double)table->size;
		if (probability > 0) {
			sum += probability * Log2(probability);
		}
	}
	return -sum;
}

// Calculates and returns the entropy over the pairs of characters in the given char table.
double calculateJointEntropy(CharTable *table) {
	if (!table) {
		return -1;
	}

	double sum = 0;
	for (int i = 0; i < kNumTotalChars; i++) {
		for (int j = 0; j < kNumTotalChars; j++) {
			double probability = (double)table->jointLetterCount[i][j] / (double)table->pairSize;
			if (probability > 0) {
				sum += probability * Log2(probability);
			}
		}
	}
	return -sum;
}

// Calculates and returns the entropy over the triplets of characters in the given char table.
double calculateTripleEntropy(CharTable *table) {
	if (!table) {
		return -1;
	}

	double sum = 0;
	for (int i = 0; i < kNumTotalChars; i++) {
		for (int j = 0; j < kNumTotalChars; j++) {
			for (int k = 0; k < kNumTotalChars; k++) {
				double probability = (double)table->tripletLetterCount[i][j][k] / (double)table->tripletSize;
				if (probability > 0) {
					sum += probability * Log2(probability);
				}
			}
		}
	}
	return -sum;
}

// Prints the given table to the given character streams (file or console).
void tablePrint(FILE **stream, CharTable *table) {
	if (!stream || !table) {
		return;
	}

	char *buffer = (char *)calloc(100, sizeof(char));

	// How wide should the "Occurence" field be?
	// +2 because we will have character triplets (e.g. 'abc')
	int fieldWidth = widthOf(table->size) + 2;

	for (int i = 0; i < kNumTotalChars; i++) {
		int count = table->letterCount[i];
		int countWidth = widthOf(count);
		int numWhitespace = fieldWidth - countWidth;  // Number of whitespace characters to be appended to the count value.
		const char *padding = whitespace(numWhitespace);
		double probability = (double)count / (double)table->size;

		sprintf(buffer, "%c : Occurence: %d%s Probability: %.6lf\r\n", table->letters[i], count, padding, probability);
		fwrite(buffer, sizeof(char), strlen(buffer), stream[0]);
	}

	const char *message = "Joint Probabilities:\r\n";
	fwrite(message, sizeof(char), strlen(message), stream[1]);

	fieldWidth--;
	for (int i = 0; i < kNumTotalChars; i++) {
		for (int j = 0; j < kNumTotalChars; j++) {
			int count = table->jointLetterCount[i][j];
			int countWidth = widthOf(count);
			int numWhitespace = fieldWidth - countWidth;  // Number of whitespace characters to be appended to the count value.
			const char *padding = whitespace(numWhitespace);
			double probability = (double)count / (double)table->pairSize;

			sprintf(buffer, "%c%c : Occurence: %d%s Probability: %.6lf\r\n",
				table->letters[i], table->letters[j], count, padding, probability);
			fwrite(buffer, sizeof(char), strlen(buffer), stream[1]);
		}

		const char *newline = "\r\n";
		fwrite(newline, sizeof(char), strlen(newline), stream[1]);
	}

	message = "Triple Probabilities:\r\n";
	fwrite(message, sizeof(char), strlen(message), stream[2]);

	fieldWidth--;
	for (int i = 0; i < kNumTotalChars; i++) {
		for (int j = 0; j < kNumTotalChars; j++) {
			for (int k = 0; k < kNumTotalChars; k++) {
				int count = table->tripletLetterCount[i][j][k];
				int countWidth = widthOf(count);
				int numWhitespace = fieldWidth - countWidth;  // Number of whitespace characters to be appended to the count value.
				const char *padding = whitespace(numWhitespace);
				double probability = (double)count / (double)table->tripletSize;

				sprintf(buffer, "%c%c%c : Occurence: %d%s Probability: %.6lf\r\n",
					table->letters[i], table->letters[j], table->letters[k], count, padding, probability);
				fwrite(buffer, sizeof(char), strlen(buffer), stream[2]);
			}

			const char *newline = "\r\n";
			fwrite(newline, sizeof(char), strlen(newline), stream[2]);
		}
	}

	free(buffer);
}

int main() {
	// This will change the active code page to 28599 
	// in order to display Turkish characters correctly in the console.
	system("chcp 28599");
	printf("\n");

	const char *kInputFileName = "..\\Debug\\input.txt";
	FILE *file = fopen(kInputFileName, "rb");
	if (!file) {
		fprintf(stderr, "Could not find input file: '%s'\n", kInputFileName);
		exitProgram(EXIT_FAILURE);
	}

	const char *outputFileNames[3] = { "mono.txt", "dual.txt", "triple.txt" };
	FILE *output[3];
	for (int i = 0; i < 3; i++) {
		const char *outputFileName = outputFileNames[i];
		output[i] = fopen(outputFileName, "wb");

		if (!output[i]) {
			fprintf(stderr, "Could not open output file: '%s'\n", outputFileName);
			exitProgram(EXIT_FAILURE);
		}
	}

	// Table that will store the character info.
	CharTable *table = tableInit();

	int totalCharsRead = 0;
	unsigned char charRead;
	unsigned char prevCharRead = 0x00;
	unsigned char prevPrevCharRead = 0x00;

	while (1) {
		int numCharsRead = (int)fread(&charRead, sizeof(unsigned char), 1, file);
		if (numCharsRead != 1) {
			// End-of-file reached. Stop reading here.
			break;
		}
		else {
			unsigned char lowercaseLetter = toLowercase(table, charRead);
			if (lowercaseLetter != 0x00) {
				tableAddChar(table, charRead);
			}
			totalCharsRead += numCharsRead;

			tableAddCharPair(table, prevCharRead, charRead);
			tableAddCharTriplet(table, prevPrevCharRead, prevCharRead, charRead);
			prevPrevCharRead = prevCharRead;
			prevCharRead = charRead;
		}
	}

	char *buffer = (char *)calloc(200, sizeof(char));
	sprintf(buffer, "Statistics for file '%s':\r\n\r\n", kInputFileName);
	fwrite(buffer, sizeof(char), strlen(buffer), output[0]);
	sprintf(buffer, "File size (bytes): %d\r\n", totalCharsRead);
	fwrite(buffer, sizeof(char), strlen(buffer), output[0]);
	sprintf(buffer, "Number of letters: %d\r\n", table->size);
	fwrite(buffer, sizeof(char), strlen(buffer), output[0]);
	sprintf(buffer, "\r\n");
	fwrite(buffer, sizeof(char), strlen(buffer), output[0]);

	double entropy = calculateEntropy(table);
	sprintf(buffer, "Entropy        = %.6lf\r\n\r\n", entropy);
	fwrite(buffer, sizeof(char), strlen(buffer), output[0]);

	double jointEntropy = calculateJointEntropy(table);
	sprintf(buffer, "Joint Entropy   = %.6lf\r\n\r\n", jointEntropy);
	fwrite(buffer, sizeof(char), strlen(buffer), output[1]);

	double tripleEntropy = calculateTripleEntropy(table);
	sprintf(buffer, "Triple Entropy  = %.6lf\r\n\r\n", tripleEntropy);
	fwrite(buffer, sizeof(char), strlen(buffer), output[2]);

	sprintf(buffer, "Character probability table:\r\n");
	fwrite(buffer, sizeof(char), strlen(buffer), output[0]);

	tablePrint(output, table);
	printf("Generated output files:\n");
	for (int i = 0; i < 3; i++) {
		printf("%s\n", outputFileNames[i]);
	}

	fclose(file);
	for (int i = 0; i < 3; i++) {
		fclose(output[i]);
	}

	exitProgram(EXIT_SUCCESS);
}