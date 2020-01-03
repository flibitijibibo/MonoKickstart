#include <mono/metadata/mono-config.h>
#include <mono/metadata/assembly.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int mono_main (int argc, char* argv[]);

#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

static char **mono_options = NULL;

static int count_mono_options_args (void)
{
	const char *e = getenv ("MONO_BUNDLED_OPTIONS");
	const char *p, *q;
	int i, n;

	if (e == NULL)
		return 0;

	/* Don't bother with any quoting here. It is unlikely one would
	 * want to pass options containing spaces anyway.
	 */

	p = e;
	n = 1;
	while ((q = strchr (p, ' ')) != NULL) {
		n++;
		p = q + 1;
	}

	mono_options = malloc (sizeof (char *) * (n + 1));

	p = e;
	i = 0;
	while ((q = strchr (p, ' ')) != NULL) {
		mono_options[i] = malloc ((q - p) + 1);
		memcpy (mono_options[i], p, q - p);
		mono_options[i][q - p] = '\0';
		i++;
		p = q + 1;
	}
	mono_options[i++] = strdup (p);
	mono_options[i] = NULL;

	return n;
}

#include "binreloc.h"

int main (int argc, char* argv[])
{
	char **newargs;
	int i, k = 0;

#ifdef _WIN32
	/* CommandLineToArgvW() might return a different argc than the
	 * one passed to main(), so let it overwrite that, as we won't
	 * use argv[] on Windows anyway.
	 */
	wchar_t **wargv = CommandLineToArgvW (GetCommandLineW (), &argc);
#endif

	newargs = (char **) malloc (sizeof (char *) * (argc + 2) + count_mono_options_args ());

#ifdef _WIN32
	newargs [k++] = g_utf16_to_utf8 (wargv [0], -1, NULL, NULL, NULL);
#else
	newargs [k++] = argv [0];
#endif

	if (mono_options != NULL) {
		i = 0;
		while (mono_options[i] != NULL)
			newargs[k++] = mono_options[i++];
	}

	BrInitError err = 0;
	if (br_init(&err) == 1) {
#ifdef __APPLE__
		char *exedir = br_find_data_dir(NULL);
#else
		char *exedir = br_find_exe_dir(NULL);
#endif
		if (exedir) {
			setenv("MONO_PATH",exedir,1);
			mono_set_dirs(exedir, exedir);
			chdir(exedir);
			free(exedir);
		}
	} else {
		switch (err) {
		case BR_INIT_ERROR_NOMEM:
			printf("Could not allocate enough memory\n");
			break;
		case BR_INIT_ERROR_OPEN_MAPS:
		case BR_INIT_ERROR_READ_MAPS:
		case BR_INIT_ERROR_INVALID_MAPS:
			printf("Couldn't access /proc/self/maps!\n");
			break;
		case BR_INIT_ERROR_DISABLED:
			printf("BinReloc disabled!!\n");
			break;
		}
		return 1;
	}
	
	// Calculate image_name
	char *image_name;
	char *image_suffix;
	char *exe = br_find_exe(NULL);
	char *pos = strrchr(exe, '/');
	if (pos != NULL) {
		image_name = pos + 1;
		image_suffix = strstr(image_name, ".bin.");
		if (image_suffix != NULL)
		{
			image_name = strdup(pos + 1);
			strcpy(strstr(image_name, ".bin."), ".exe");
		}
		else
		{
			image_suffix = strstr(image_name, ".");
			if (image_suffix == NULL)
			{
				/* This is most likely a *nix executable, so
				 * just append ".exe" to the end of it
				 */
				const char *exe_suffix = ".exe";
				image_name = (char*) malloc(
					strlen(image_name) +
					strlen(exe_suffix) +
					1
				);
				strcpy(image_name, pos + 1);
				strcat(image_name, exe_suffix);
			}
			else
			{
				printf("Failed to get exe name!\n");
				return 1;
			}
		}
	}
	free(exe);

	newargs [k++] = image_name;

	for (i = 1; i < argc; i++) {
#ifdef _WIN32
		newargs [k++] = g_utf16_to_utf8 (wargv [i], -1, NULL, NULL, NULL);
#else
		newargs [k++] = argv [i];
#endif
	}
#ifdef _WIN32
	LocalFree (wargv);
#endif
	newargs [k] = NULL;

	/* config */
	FILE *fileIn = fopen("monoconfig", "r");
	if (fileIn == NULL)
	{
		printf("monoconfig not found!\n");
		return 0;
	}
	fclose(fileIn);
	setenv("MONO_CONFIG", "monoconfig", 0);

	/* machine.config */
	fileIn = fopen("monomachineconfig", "r");
	if (fileIn == NULL)
	{
		printf("monomachineconfig not found!\n");
		return 0;
	}
	fseek(fileIn, 0, SEEK_END);
	long len = ftell(fileIn);
	char *machineconfig = (char*) malloc(len); /* DO NOT FREE! -flibit */
	fseek(fileIn, 0, SEEK_SET);
	fread(machineconfig, len, 1, fileIn);
	fclose(fileIn);
	mono_register_machine_config(machineconfig);

	/* Main(string[] args) */
	return mono_main (k, newargs);
}
