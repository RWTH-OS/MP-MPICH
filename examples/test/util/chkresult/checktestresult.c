/*
	Usage:
	checktestresult <testname>

	Input Files:
	ignore.std
	<testname>.std
	<testname>.out (Linux, Unix)
	<testname>.txt (Windows)
	<testname>.neg

	Output:
	stdout

	the Program checks the following:
	is every line defined in <testname>.std present in <testname>.out
	   yes -> test success
	is any line defined in <testname>.neg present in <testname>.out
	no -> test failure

	if both occures test has failed...
    
	lines defined in ignore.std are ignored

	if test is successfull no output occures
	if test is not successfull output will tell why the test has failed

*/


#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <string.h>
#include <ctype.h>
#endif

#define SZ_LINEBUFFER 4096
#define SZ_SUBQUERYS 256
#define SZ_FILENAME 256

int verbose = 0;
int debug = 0;

/* #define MIN(a,b) ((a < b) ? a : b) */

typedef struct t_query_ {
    char match_orig[SZ_LINEBUFFER];
    char match[SZ_LINEBUFFER];
    char* submatch[SZ_SUBQUERYS];
    int sm_count;
    int matches;
    struct t_query_ *next;
} t_query;

typedef struct t_line_ {
    char line[SZ_LINEBUFFER];
    int show;
    struct t_line_ *next;
} t_line;


int isspace_safe(char* buf, int pos) {
    /* an undefined position is not a space */
    if (pos < 0)
	return 0;
    else
	return isspace(buf[pos]);
}

/* trims whitespace from the right */
void rtrim(char *string) {
    int pos = strlen(string) -1;
    while(isspace_safe(string,pos))
	pos --;
    string[pos +1] = '\0';
}

/* checks if a line matches a query (line from a query file) */
int match(char* p_line, t_query* p_query) {
    int RetVal = 0;
    int i, len;
    char *pos = p_line;
    
    if (p_line == NULL || p_query == NULL)
	return 0;
    
    for(i=0; i<p_query->sm_count; i++) {
	len = strlen(p_query->submatch[i]);
	if (len > 0) {
	    if (pos = strstr(pos, p_query->submatch[i])) {
		pos += len;
		RetVal = 1;
	    }
	    else {
		RetVal = 0;
		break;
	    }
	}
    }

    if (RetVal && debug) {
	printf("-- Line matched: %s\n", p_line);
	printf("-- Rule matched: (%x) %s\n",p_query, p_query->match_orig);
    }
    
    p_query->matches += RetVal;
    return RetVal;
}

/* checks if a line matches all/some/one/none of a list of queries */
int match_all(char* p_line, t_query* p_root) {
    int RetVal = 0;
    
    if (p_root == NULL)
	return 0;

    RetVal += match(p_line, p_root);
    
    if (p_root->next)
	RetVal += match_all(p_line, p_root->next);
    return RetVal;
}

/* reads a bunch of lines and buffers them in a linebased internal buffer format */
t_line *read_lines(char* filename) {
    FILE* fp_file;
    t_line* root = NULL;
    t_line* last = NULL;
    char buf[SZ_LINEBUFFER];

    fp_file = fopen(filename, "r");
    if (!fp_file) {
	fprintf(stderr, "File %s not found\n", filename);
	return(NULL);
    }
    
    while (fgets(buf, SZ_LINEBUFFER-1, fp_file)) {
	if (root == NULL) {
	    root =  malloc(sizeof(t_line));
	    last = root;
	}
	else {
	    last->next = malloc(sizeof(t_line));
	    last = last->next;
	}
	memset(last, 0, sizeof(t_line));
	buf[SZ_LINEBUFFER-1] = 0;
	rtrim(buf);
	strcpy(last->line, buf);
    }

    fclose(fp_file);
    return root;
}

/* reads a bunch of lines and transforms them to a list of queries */
t_query *read_queries(char* filename) {
    t_query* root = NULL;
    t_query* last = NULL;
    FILE* fp_file;
    int i, len;
    char buf[SZ_LINEBUFFER];
       
    fp_file = fopen(filename, "r");
    if (!fp_file) {
	/* fprintf(stderr, "File %s not found\n", filename); */
	return(NULL);
    }
    while (fgets(buf, SZ_LINEBUFFER-1, fp_file)) {
	buf[SZ_LINEBUFFER-1] = 0;
	rtrim(buf);
	if (strlen(buf) > 0) {	
	    if (root == NULL) {
		root =  malloc(sizeof(t_query));
		last = root;
	    }
	    else {
		last->next = malloc(sizeof(t_query));
		last = last->next;
	    }
	    memset(last, 0, sizeof(t_query));
	    
	    strcpy(last->match_orig, buf);
	    strcpy(last->match, buf);
	    len = strlen(last->match);
	    last->submatch[last->sm_count++] = last->match;
	    for (i=0; i<len; i++) {
		if (last->match[i] == '*') {
		    last->submatch[last->sm_count++] = last->match + i + 1;
		    last->match[i] = '\0';
		}
	    }
	}
    }
    fclose(fp_file);
    
    if (root && debug) {
	printf("- Queries loaded: (%x) %s\n",root, filename);
    }
    
    return root;
}

/* will return how many queries are not matched */
int unmatched(t_query* p_root) {
    int RetVal = 0;
    
    if (p_root == NULL)
	return 0;
    
    if (p_root->matches == 0)
	RetVal = 1;
    
    return RetVal + unmatched(p_root->next);
}

/* prints queries not matched */
void print_unmatched(t_query* p_root) {
    if (p_root == NULL)
	return;
    
    if (p_root->matches == 0)
	printf("-- '%s'\n", p_root->match_orig);
    print_unmatched(p_root->next);
}


int main(int argc, char* argv[]) {
    t_query* p_query_pos = NULL;
    t_query* p_query_neg = NULL;
    t_query* p_query_ignore = NULL;
    t_line* p_lines;
    t_line* p_act;
    char filename[SZ_FILENAME];
    char posname[SZ_FILENAME];
    char negname[SZ_FILENAME];
    char ignorename[SZ_FILENAME] = "ignore.std";
    int neg_match = 0;
    int missing_pos_match = 0;
	int i=0;

#ifdef WIN32
#ifdef _DEBUG
		printf("Command Line before parse:\n");
		for (i=0;i<argc;i++)
			printf("%s ",argv[i]);
		printf("\n");

#endif
#endif
    
	/* if whe have more than 3 parameters interpret and shift argument list */
    /* (add a loop later) */
	if (argc >= 3) {
	  if (strcmp("-v", argv[1]) == 0) {
	    verbose = 1;
	    argv[1] = argv[2];
	    argc--;
	  }
    }

	/* if less than 2 printout usage */
    if (argc < 2) {
	fprintf(stderr, "usage: %s <testname>\n", argv[0]);
	exit (-1);
    }
#ifdef WIN32
#ifdef _DEBUG
	if (verbose)
	{
		printf("Command Line after parse:\n");
		for (i=0;i<argc;i++)
			printf("%s ",argv[i]);
		printf("\n");
	}

#endif
#endif


	/* create filenames */
#ifdef WIN32
    sprintf(filename, "%s.txt", argv[1]);
#else
	sprintf(filename, "%s.out", argv[1]);
#endif
    sprintf(posname, "%s.std", argv[1]);
    sprintf(negname, "%s.neg", argv[1]);
    
	/* read queries and lines */
    p_query_pos = read_queries(posname);
    p_query_neg = read_queries(negname);
    p_query_ignore = read_queries(ignorename);
    p_lines = read_lines(filename);
    
	/* walk through all lines of testoutput */
    for (p_act = p_lines; p_act != NULL; p_act=p_act->next) {
	
	/* if a line is not defined as positive match, mark it for output */
	if (match_all(p_act->line, p_query_pos) < 1)
	    p_act->show = 1;
	
	/* ignore overrides lines not in pos. match */
	if (match_all(p_act->line, p_query_ignore)) {
	    p_act->show = 0;
	}

	/* if a line is defined as negative match, mark it for output */
	/* negatives overides ignore */
	if (match_all(p_act->line, p_query_neg)) {
	    p_act->show = 1;
	    neg_match++;
	}
    }

	/* are there unsatisfyed positive queries ? */
    missing_pos_match = unmatched(p_query_pos);
    
    if (neg_match || missing_pos_match) {
	printf("\n");
	printf("Test '%s' failed\n", argv[1]);
	if (verbose) {
	    if (neg_match)
		printf("- matched negative criteria: %d\n",neg_match);
	    if (missing_pos_match) {
		printf("- unmatched positive criteria: %d\n",missing_pos_match);
		print_unmatched(p_query_pos);
	    }
	}
	printf("\nOUTPUT:\n");
	for (p_act = p_lines; p_act != NULL; p_act=p_act->next) {
	    if (p_act->show)
		printf("%s\n", p_act->line);
	}
	printf("\n");
    }
    else {
	if (debug)
	    printf("Test '%s' was successfull\n", argv[1]);
    }

    return 0;
}
