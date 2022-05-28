/**
Homework #5 - Grep - By Yazen Aljamal

Test Cases:

  Case 1: Searching my name using a file name override
  	Input: -l yazen file_one.txt file_two.txt file_three.txt
  	Expected Output: file_one.txt
  	            	   file_three.txt
  
  Case 2: Searching my name using count override
    Input: -c yazen file_one.txt file_two.txt file_three.txt
    Expected Output: file_one.txt contains 1 match
                     file_three.txt contains 1 match
  
  Case 3: Searching my name using count override while inverting output
    Input: -c -v yazen file_one.txt file_two.txt file_three.txt
    Expected Output: file_one.txt contains 2 non matches
                     file_two.txt contains 3 non matches
                     file_three.txt contains 3 non matches
  
  Case 4: Searching a random letter while ignoring case with line number enabled
    Input: -i -n a file_one.txt file_two.txt file_three.txt
    Expected Output: [file_one.txt] (LINE#1) this is a test
                     [file_one.txt] (LINE#2) my name is yazen
                     [file_one.txt] (LINE#3) i like programming
                     [file_two.txt] (LINE#1) Professor Bruce Mckenzie is Awesome!
                     [file_two.txt] (LINE#2) I learned a lot this class.
                     [file_two.txt] (LINE#3) I finally got a job!
                     [file_three.txt] (LINE#1) my name is yazen aljamal
                     [file_three.txt] (LINE#2) and I'm an electrical engineer
                     [file_three.txt] (LINE#3) but I like programming more than
                     [file_three.txt] (LINE#4) electrical circuits
  
  Case 5: Searching 'programming' with no file name enabled
    Input: -i -h programming file_one.txt file_two.txt file_three.txt
    Expected Output: i like programming
                     but I like programming more than
  
  Case 6: Searching 'programming' with file name and line number enabled
    Input: -i -H -n programming file_one.txt file_two.txt file_three.txt
    Expected Output: [file_one.txt] (LINE#3) i like programming
                     [file_three.txt] (LINE#3) but I like programming more than
  
  Case 7: Find all lines that don't contain 'programming' using count override
    Input: -i -c -v programming file_one.txt file_two.txt file_three.txt
    Expected Output: file_one.txt contains 2 matches
                     file_two.txt contains 3 matches
                     file_three.txt contains 3 matches
  
  Case 8: Match an exact line while ignoring case and line number enabled
    Input: -i -x -n "professor bruce mckenzie is awesome!" file_one.txt file_two.txt file_three.txt
    Expected Output: [file_two.txt] (LINE#1) Professor Bruce Mckenzie is Awesome!
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_BUF 2048

/** Pattern pointer */
char *pattern           = NULL;

/** Matching Control -i -v -x */
bool ignore_case        = false;
bool inverse_match      = false;
bool match_full_line    = false;

/** Output Line Prefix Control -H -h -n */
bool show_file_name     = false;
bool show_line_num      = false;
bool no_file_name       = false;

/** General Output Control -l -c */
bool file_name_override = false;
bool count_override     = false;

/** Print and return the number of matches found.*/
size_t find_matches(char *file_name);

int main(int argc, char *argv[]) 
{
  if (argc < 2)
  {
    fprintf(stderr, "You must enter atleast a pattern and filename.\n");
    return 1;
  }
  
  size_t matches_found    = 0;
  size_t first_file_index = 0;

  for (size_t i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
        case 'i':
          /*
            --ignore-case
            Ignore case distinctions in both the PATTERN and the input files.
          */
          ignore_case = true;
          break;
        case 'v':
          /*
            --invert-match
            Invert the sense of matching, to select non-matching lines.
          */
          inverse_match = true;
          break;
        case 'x':
          /*
            --line-regexp
            Select only those matches that exactly match the whole line.
          */
          match_full_line = true;
          break;
        case 'H':
          /*
            -with-filename
            Print the file name for each match.
            This is the default when there is more than one file to search.
          */
          show_file_name = true;
          no_file_name = false;
          break;
        case 'h':
          /*
            --no-filename
            Suppress the prefixing of file names on output.
            This is the default when there is only one file (or only standard input) to search.
          */
          show_file_name = false;
          no_file_name = true;
          break;
        case 'l':
          /*
            --files-with-matches
            Suppress normal output;
            instead print the name of each input file from which output would normally have been printed.
            The scanning will stop on the first match. 
          */
          file_name_override = true;
          break;
        case 'c':
          /*
            --count
            Suppress normal output;
            instead print a count of matching lines for each input file.
            With the -v, --invert-match option (see below), count non-matching lines. 
          */
          count_override = true;
          break;
        case 'n':
          /*
            --line-number
            Prefix each line of output with the 1-based line number within its input file.
          */
          show_line_num = true;
          break;
        default:
          fprintf(stderr, "ERROR: Unknown flag encountered '%s'. Skipping...\n", argv[i]);
         break;
      }
    }
    else
    {
      if(!pattern)
      {
        pattern = argv[i];
        first_file_index = i + 1;
        break;
      }
    }
  }

  if (!pattern)
  {
    fprintf(stderr, "ERROR: No pattern found in your input.\n");
    return 1;
  }

  if (!first_file_index || first_file_index >= argc)
  {
    fprintf(stderr, "ERROR: No files found in your input.\n");
    return 1;
  }

  // Set default behavior if no file name info given and there's more than 1 file.
  if (!no_file_name && argc - first_file_index > 1)
    show_file_name = true;

  for (size_t i = first_file_index; i < argc; i++)
    matches_found += find_matches(argv[i]);

  if (!matches_found)
    printf("No matches found.\n");
  
  return 0;
}

size_t find_matches(char *file_name)
{
  FILE *fp;
  bool match_found;
  char line[MAX_BUF];
  size_t matches_found = 0;
  size_t line_num = 1;
  
  fp = fopen(file_name, "r");
  
  if (fp == NULL)
  {
    fprintf(stderr, "ERROR: File '%s' could not be opened. Skipping...\n", file_name);
    return 0;
  }

  while (fgets(line, sizeof line, fp))
  {
    if(line[strlen(line) - 1] == '\n')
      line[strlen(line) - 1] = '\0';
    
    match_found = false;

    if (match_full_line)
    {
      match_found = ignore_case ? !strcasecmp(line, pattern) : !strcmp(line, pattern);
    }
    else
    {
      match_found = ignore_case ? strcasestr(line, pattern) : strstr(line, pattern);
    }

    if (inverse_match)
      match_found = !match_found;

    if (match_found)
    {
      if (!count_override)
      {
        if (file_name_override)
        {
          printf("%s\n", file_name);
          return 1;
        }
        
        if (show_file_name)
          printf("[%s] ", file_name);
  
        if (show_line_num)
          printf("(LINE#%ld) ", line_num);

        printf("%s\n", line);
      }
      
      matches_found++;
    }

    line_num++;
  }

  if (count_override && matches_found > 0)
    printf("%s contains %ld match(es)\n",
      file_name,
      matches_found);
  
  fclose(fp);
  
  return matches_found;
}