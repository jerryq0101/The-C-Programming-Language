/*
 * Exercise 5-14. Modify the sort program to handle a -r flag, which indicates
 * sorting in reverse (decreasing) order. be sure that -r works with -n.
 * By Faisal Saadatmand
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINES   5000      /* max #lines to be sorted */
#define MAXLEN     1000      /* max length of any input line */
#define MEMORYSIZE 10000     /* storage for memory[] */

/* functions */
int  getLine(char *s, int lim);
int  readlines(char *lineptr[], int nlines, char *p);
void witelines(char *lineptr[], int nlines);
void qSort(void *lineptr[], int left, int right,
		int (*comp)(void *, void *));
int  numcmp(char *, char *);
int  strCmp(char *s, char *t);
int  rnumcmp(char *, char *);
int  rstrCmp(char *s, char *t);

/* Globals */
char *lineptr[MAXLINES];     /* pointers to text lines */

/* getLine: get line into s, return length of s -- pointer version */
int getLine(char *s, int lim)
{
	int c, len;

	len = 0;
	while (--lim > 0 && (c = getchar()) != EOF && c != '\n') {
		*s++ = c;
		++len;
	}
	if ( c == '\n') {
		*s++ = c;
		++len;
	}
	*s = '\0';
	return len;
}

/* readlines: read input lines */
int readlines(char *lineptr[], int maxlines, char *p)
{
	int len, nlines, usedMemory;
	char line[MAXLEN];

	usedMemory = 0;
	nlines = 0;
	while ((len = getLine(line, MAXLEN)) > 0)
		if (nlines >= maxlines || MEMORYSIZE - usedMemory < len)
			return -1;
		else {
			line[len - 1] = '\0';   /* delete newline character */
			strcpy(p, line);
			lineptr[nlines++] = p;
			p += len;               /* increment pointer */
			usedMemory += len;      /* track memory usage */
		}
	return nlines;
}

/* writelines: write output lines */
void writelines(char *lineptr[], int nlines)
{
	while (nlines-- > 0)
		printf("%s\n", *lineptr++);
}

/* qsort: sort v[left]...V[right] into increasing order */
void qSort(void *v[], int left, int right,
		int (*comp)(void *, void *))
{
	int i, last;

	void swap(void *v[], int, int);

	if (left >= right)             /* do nothing if array contains */
		return;                    /* fewer than two elements */

	swap(v, left, (left + right) / 2);
	last = left;
	for (i = left + 1; i <= right; i++)
		if ((*comp)(v[i], v[left]) < 0)
			swap(v, ++last, i);
	swap(v, left, last);
	qSort(v, left, last - 1, comp);
	qSort(v, last + 1, right, comp);
}

/* numcmp: compare s1 and s2 numerically */
int numcmp(char *s1, char *s2)
{
	double v1, v2;

	v1 = atof(s1);
	v2 = atof(s2);

	if (v1 < v2)
		return -1;
	else if (v1 > v2)
		return 1;
	else
		return 0;
}

/* strCmp: return < 0 if s < t, 0 if s == t, > 0 if s > t */
int strCmp(char *s, char *t)
{
	for ( ; *s == *t; s++, t++)
		if (*s == '\0')
			return 0;
	return *s - *t;
}

/* rnumcmp: compare s1 and s2 numerically - reversed order */
int rnumcmp(char *s1, char *s2)
{
	double v1, v2;

	v1 = atof(s1);
	v2 = atof(s2);

	if (v1 > v2)
		return -1;
	else if (v1 < v2)
		return 1;
	else
		return 0;
}

/* rstrCmp: same as strCmp but in reverse order */
int rstrCmp(char *s, char *t)
{
	for ( ; *s == *t; s++, t++)
		if (*s == '\0')
			return 0;
	return *t - *s;
}

void swap(void *v[], int i, int j)
{
	void *temp;

	temp = v[i];
	v[i] = v[j];
	v[j] = temp;
}

/* sort input lines */
int main(int argc, char *argv[])
{
	int nlines;                    /* number of input lines read */     
	int numeric = 0;               /* 1 if numeric sort */
	int decreasing = 0;               /* 1 if reverse order sort */
	char memory[MEMORYSIZE];       /* stored lines */

	++argv;
	while (--argc > 0) {
		if (strCmp(argv[0], "-n") == 0)
			numeric = 1;
		if (strCmp(argv[0], "-r") == 0)
			decreasing = 1;
		++argv;
	}

	if ((nlines = readlines(lineptr, MAXLINES, memory)) >= 0) {
		if (decreasing)
			qSort((void**) lineptr, 0, nlines - 1,
					(int (*)(void*, void*))(numeric ? rnumcmp : rstrCmp));
		else
			qSort((void**) lineptr, 0, nlines - 1,
					(int (*)(void*, void*))(numeric ? numcmp : strCmp));
		writelines(lineptr, nlines);
		return 0;
	} else {
		printf("input too big to sort\n");
		return 1;
	}
}
