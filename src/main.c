/*
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

	Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.
	The name of the author may not be used to endorse or promote products
	derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parsemidi.h"
#include "midimerge.h"
#include "convert.h"
#include "smalloc.h"



int main(int argc, char** argv)
{
	midi_t* midi;
	FILE* f;

	char* buff = smalloc(BUFSIZ);
	size_t buffsize = BUFSIZ;

	size_t len = 0;

	size_t nread;

	printf("MIDI3MUS\n"
		"\n");

	if (argc != 5 && argc != 4 && argc != 3 && argc != 2)
	{
		fprintf(stderr, "Usage: midi3mus [-options value] infile [outfile]\n"
			"  Default action is to convert with a time division of 140 Hz\n"
			"options:\n"
			"  -t set time divison\n"
			"value:\n"
			"  Set time divison value in Hz\n"
			"  For example, 140 is correct for games based on the Doom engine and\n"
			"  70 is the correct value for Raptor: Call of the Shadows\n");
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "-") == 0)
	{
		f = stdin;
	}
	else if (strcmp(argv[1], "-t") == 0)
	{
		if (argv[2] == NULL)
		{
			fprintf(stderr, "time division value not specified\n");
			return EXIT_FAILURE;
		}

		if (argv[3] == NULL)
		{
			fprintf(stderr, "Inputfile not specified\n");
			return EXIT_FAILURE;
		}

		size_t stringlen = strlen(argv[2]);

		for (int i = 0; i < stringlen; i++)
		{
			if (!isdigit(argv[2][i]))
			{
				fprintf(stderr, "time division value not digit %s\n", argv[2]);
				return EXIT_FAILURE;
			}
		}

		u16 tpsval = atoi(argv[2]);

		if (!tpsval)
		{
			fprintf(stderr, "time division value out of range %s\n", argv[2]);
			return EXIT_FAILURE;
		}

		set_tps_value(tpsval);

		f = fopen(argv[3], "rb");
		if (!f)
		{
			fprintf(stderr, "failed to load %s\n", argv[3]);
			return EXIT_FAILURE;
		}
	}
	else
	{
		f = fopen(argv[1], "rb");
		if (!f)
		{
			fprintf(stderr, "failed to load %s\n", argv[1]);
			return EXIT_FAILURE;
		}
	}

	while ((nread = fread(buff + len, 1, BUFSIZ, f)) == BUFSIZ)
	{
		len += nread;

		if (len == buffsize)
		{
			buffsize *= 2;
			buff = srealloc(buff, buffsize);
		}
	}
	len += nread;
	fclose(f);



	midi = parsemidi(buff, len);

	if (!midi)
	{
		fprintf(stderr, "some failure in parsemidi\n");
		return EXIT_FAILURE;
	}

	if (midi->mode == 1)
	{
		//print_midi_debug (midi);

		fprintf(stderr, "merging tracks on type 1 file\n");
		midi_t* newmidi = mergemode1(midi);
		if (!newmidi)
		{
			fprintf(stderr, "fail in merging mode 1 to mode 0\n");
			return EXIT_FAILURE;
		}
		freemidi(midi);
		midi = newmidi;
	}
	free(buff);

	if (!argv[2] || (strcmp(argv[1], "-t") == 0 && !argv[4]))
	{ // default outfile name
		if (strcmp(argv[1], "-t") == 0 && !argv[4])
		{
			strcpy(argv[1], argv[3]);
		}

		char* fnameout;
		int i = strlen(argv[1]);
		fnameout = smalloc(i + 5);

		strcpy(fnameout, argv[1]);

		if (fnameout[i - 4] == '.')
		{ // replace extension
			strcpy(fnameout + i - 3, "mus");
		}
		else
		{ // append extension
			strcpy(fnameout + i, ".mus");
		}
		f = fopen(fnameout, "wb");
		if (!f)
		{
			fprintf(stderr, "failed to write to %s\n", fnameout);
			return EXIT_FAILURE;
		}
		free(fnameout);
	}
	else if (strcmp(argv[2], "-") == 0)
	{
		f = stdout;
	}
	else if (argv[4] && argc == 5)
	{
		f = fopen(argv[4], "wb");
		if (!f)
		{
			fprintf(stderr, "failed to write to %s\n", argv[4]);
			return EXIT_FAILURE;
		}
	}
	else
	{
		f = fopen(argv[2], "wb");
		if (!f)
		{
			fprintf(stderr, "failed to write to %s\n", argv[2]);
			return EXIT_FAILURE;
		}
	}

	//print_midi_debug (midi);

	convert_midi(midi, f);

	fclose(f);


	freemidi(midi);

	return 0;
}











