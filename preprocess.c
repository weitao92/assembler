#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "preprocess.h"

/**
This function is responsible for preprocess the input file. I will delete the comment and blank line.
Then delete all the tab and white space as well except those in the ascii part.
**/
void preProcess(FILE* in, FILE* out)
{
	char* line = calloc(100, sizeof(char));
	while(fgets(line, 100, in) != NULL) 
	{
		if(line[0] != '\n')
		{
			int first = 0;
			while(line[first] == ' ' || line[first] == '\t')//ignore the blank line
			{
				first++;
			}

			if(line[first] == '#')
			{
				continue;
			}
			char* comment = strchr(line, '#'); //get the comment part
			char* dest = calloc(100, sizeof(char));
			if(comment != NULL)
			{
				int length = strlen(line) - strlen(comment);
				strncpy(dest, line, length);
				char newLine[1];
				newLine[0] = '\n';
				strcat(dest, newLine);
			}
			else
			{
				strncpy(dest, line, strlen(line));
			}
			if(strchr(dest, '\"') == NULL)//remove the tab and space.
			{
				char* token = strtok(dest, " \t");
				while (token != NULL)
				{
					fprintf(out, "%s", token);
					token = strtok(NULL, " \t");
				}
			}
			else//special case for the ascii part, which i need to reserve the tab and space between the " ".
			{
				char* dest1 = calloc(100, sizeof(char));
				strncpy(dest1, dest, strlen(dest));
				char* quote = strchr(dest, '\"');

		        int quotePos = (quote - dest);

				int strLength = strlen(dest);
			    char ascii[strLength - quotePos];
			    int count = 0;
			    for(int i = quotePos; i < strLength; i++)
			    {
			    	ascii[count++] = dest[i];
			    }

			    char* token = strtok(dest1, " \t");
				while (token != NULL && token[0] != '\"')
				{
					fprintf(out, "%s", token);
					token = strtok(NULL, " \t");
				}
				fprintf(out, "%s", ascii);
				free(dest1);

			}
			free(dest);
		}
	}
	free(line);
}