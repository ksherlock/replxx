#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "replxx.h"

void completionHook(char const* prefix, int bp, replxx_completions* lc, void* ud) {
	char** examples = (char**)( ud );
	size_t i;
	for (i = 0;	examples[i] != NULL; ++i) {
		if (strncmp(prefix + bp, examples[i], strlen(prefix) - bp) == 0) {
			replxx_add_completion(lc, examples[i]);
		}
	}
}

void hintHook(char const* prefix, int bp, replxx_hints* lc, ReplxxColor* c, void* ud) {
	char** examples = (char**)( ud );
	int i;
	int len = strlen( prefix );
	if ( len > bp ) {
		for (i = 0;	examples[i] != NULL; ++i) {
			if (strncmp(prefix + bp, examples[i], strlen(prefix) - bp) == 0) {
				replxx_add_hint(lc, examples[i] + len - bp);
			}
		}
	}
}

void colorHook( char const* str_, ReplxxColor* colors_, int size_, void* ud ) {
	int i = 0;
	for ( ; i < size_; ++ i ) {
		if ( isdigit( str_[i] ) ) {
			colors_[i] = BRIGHTMAGENTA;
		}
	}
}

int main( int argc, char** argv ) {
	char* examples[] = {
		"db", "hello", "hallo", "hans", "hansekogge", "seamann", "quetzalcoatl", "quit", "power", NULL
	};
	Replxx* replxx = replxx_init();
	replxx_install_window_change_handler( replxx );

	int quiet = 0;
	while ( argc > 1 ) {
		-- argc;
		++ argv;
#ifdef __REPLXX_DEBUG__
		if ( !strcmp( *argv, "--keycodes" ) ) {
			replxx_debug_dump_print_codes();
			exit(0);
		}
#endif
		switch ( (*argv)[0] ) {
			case 'b': replxx_set_beep_on_ambiguous_completion( replxx, (*argv)[1] - '0' ); break;
			case 'c': replxx_set_completion_count_cutoff( replxx, atoi( (*argv) + 1 ) );   break;
			case 'e': replxx_set_complete_on_empty( replxx, (*argv)[1] - '0' );            break;
			case 'd': replxx_set_double_tab_completion( replxx, (*argv)[1] - '0' );        break;
			case 'h': replxx_set_max_hint_rows( replxx, atoi( (*argv) + 1 ) );             break;
			case 's': replxx_set_max_history_size( replxx, atoi( (*argv) + 1 ) );          break;
			case 'q': quiet = atoi( (*argv) + 1 );                                         break;
		}

	}

	const char* file = "./replxx_history.txt";

	replxx_history_load( replxx, file );
	replxx_set_completion_callback( replxx, completionHook, examples );
	replxx_set_highlighter_callback( replxx, colorHook, NULL );
	replxx_set_hint_callback( replxx, hintHook, examples );

	printf("starting...\n");

	char const* prompt = "\x1b[1;32mreplxx\x1b[0m> ";

	while (1) {
		char const* result = NULL;
		do {
			result = replxx_input( replxx, prompt );
		} while ( ( result == NULL ) && ( errno == EAGAIN ) );

		if (result == NULL) {
			printf("\n");
			break;
		} else if (!strncmp(result, "/history", 8)) {
			/* Display the current history. */
			int index = 0;
			int size = replxx_history_size( replxx );
			for ( ; index < size; ++index) {
				char const* hist = replxx_history_line( replxx, index );
				if (hist == NULL) break;
				printf("%4d: %s\n", index, hist);
			}
		}
		if (*result != '\0') {
			printf( quiet ? "%s\r\n" : "thanks for the input: %s\n", result );
			replxx_history_add( replxx, result );
		}
	}
	replxx_history_save( replxx, file );
	printf( "Exiting Replxx\n" );
	replxx_end( replxx );
}

