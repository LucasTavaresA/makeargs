#ifndef MAKEARGS_FREESTANDING
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
#	include "log.c"
#endif

/// prints the help message.
/// generated based on your MAKEARGS_TARGETS.
static inline void makeargs_help(const char* argv0);

/// returns the value of a variable.
/// halts if the variable is not found.
static char* makeargs_get(const char* name);

/// appends a to a variable.
/// halts if the variable is not found.
/// halts if the appended variable is too long.
static void makeargs_append(const char* name, const char* suffix);

/// sets a variable.
/// halts if you have too many variables.
/// halts if the variable name or value is too long.
static void makeargs_set(const char* name, const char* value);

/// sets a variable from a env variable, like "NAME=value".
/// halts if you have too many variables.
/// halts if the variable name or value is too long.
static void makeargs_set_from_var(const char* var);

/// sets all the variables based on the environment variables.
/// halts if you have too many variables.
/// halts if the variable name or value is too long.
static void makeargs_getenv();

/// sets all the variables based on the command line arguments.
/// halts if you have too many variables.
/// halts if the variable name or value is too long.
/// returns the last index or the index after MAKEARGS_SEPARATOR.
static int makeargs_set_vars(const int argc, const char** argv);

/// runs the targets based on the command line arguments.
/// prints the help message if no target is specified.
/// skips all arguments with '='.
/// returns the last index or the index after MAKEARGS_SEPARATOR.
static int makeargs_run_targets(const int argc, const char** argv);

#ifdef MAKEARGS_IMPLEMENTATION
/// makeargs will stop parsing at this separator
#	ifndef MAKEARGS_SEPARATOR
#		define MAKEARGS_SEPARATOR "--"
#	endif

/// function to call when no target is specified or when a unknown target is specified
/// by default prints a message based on your MAKEARGS_TARGETS
#	ifndef MAKEARGS_DEFAULT_TARGET
#		define MAKEARGS_DEFAULT_TARGET makeargs_help
#	endif

// an X macro list of all your targets, each target is defined like this:
// MAKEARGS_TARGET(name, description)
#	ifndef MAKEARGS_TARGETS
#		define MAKEARGS_TARGETS
#	endif

/// how the targets will be called
#	ifndef MAKEARGS_TARGET_CALL
#		define MAKEARGS_TARGET_CALL(target) \
			LOG_MSG("%s()\n", argv[i]);        \
			target();
#	endif

#	ifndef MAKEARGS_STRLEN
#		define MAKEARGS_STRLEN strlen
#	endif

#	ifndef MAKEARGS_STRCMP
#		define MAKEARGS_STRCMP strcmp
#	endif

#	ifndef MAKEARGS_STRCHR
#		define MAKEARGS_STRCHR strchr
#	endif

#	ifndef MAKEARGS_STRCPY
#		define MAKEARGS_STRCPY strcpy
#	endif

#	ifndef MAKEARGS_STRNCPY
#		define MAKEARGS_STRNCPY strncpy
#	endif

#	ifndef MAKEARGS_ENVIRON
#		ifdef _WIN32
extern char** _environ;
#			define MAKEARGS_ENVIRON _environ
#		else
extern char** environ;
#			define MAKEARGS_ENVIRON environ
#		endif
#	endif

/// maximum number of variables that can be set
#	ifndef MAKEARGS_MAX_VARS
#		define MAKEARGS_MAX_VARS 256
#	endif

/// maximum length of a variable name or value
#	ifndef MAKEARGS_VAR_LENGTH
#		define MAKEARGS_VAR_LENGTH 1024
#	endif

static struct
{
	char name[MAKEARGS_VAR_LENGTH];
	char value[MAKEARGS_VAR_LENGTH];
} makeargs_vars[MAKEARGS_MAX_VARS] = {0};
_Static_assert(sizeof(makeargs_vars) < 4 * 1024 * 1024,
							 "makeargs_vars too large!");

static int makeargs_vars_count = 0;

static inline void makeargs_help(const char* argv0)
{
	LOG_MSG("Usage: %s [", argv0);
	const char* sep = "";
#	define MAKEARGS_TARGET(target, ...) \
		LOG_MSG("%s" #target, sep);        \
		sep = "|";
	MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET
	LOG_MSG("]\n");

#	define MAKEARGS_TARGET(target, ...) \
		LOG_MSG("  " #target " \t " __VA_ARGS__ "\n");
	MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET
}

static char* makeargs_get(const char* name)
{
	for (int i = 0; i < makeargs_vars_count; i++)
	{
		if (MAKEARGS_STRCMP(makeargs_vars[i].name, name) == 0)
		{
			return makeargs_vars[i].value;
		}
	}

	LOG_HALT(LOG_ERROR_CODE, "%s:%d: Variable '%s' not found!", __FILE__,
					 __LINE__, name);
}

static void makeargs_append(const char* name, const char* suffix)
{
	char* value = makeargs_get(name);
	int value_len = MAKEARGS_STRLEN(value);

	LOG_ASSERT(
			value_len + MAKEARGS_STRLEN(suffix) < MAKEARGS_VAR_LENGTH,
			"name = \"%s\"\nvalue = \"%s%s\"\nmakeargs_append(): appended variable is too long, increase MAKEARGS_VAR_LENGTH!",
			name, value, suffix);

	MAKEARGS_STRCPY(value + value_len, suffix);
}

static void makeargs_set(const char* name, const char* value)
{
#	define MAKEARGS_VAR_FMTSTRING "name = \"%s\"\nvalue = \"%s\"\n"
	LOG_ASSERT(makeargs_vars_count < MAKEARGS_MAX_VARS,
						 "makeargs: too many variables, increase MAKEARGS_MAX_VARS!");
	LOG_ASSERT(
			MAKEARGS_STRLEN(name) < MAKEARGS_VAR_LENGTH,
			MAKEARGS_VAR_FMTSTRING
			"makeargs_set(): variable name too long, increase MAKEARGS_VAR_LENGTH!",
			name, value);
	LOG_ASSERT(
			MAKEARGS_STRLEN(value) < MAKEARGS_VAR_LENGTH,
			MAKEARGS_VAR_FMTSTRING
			"makeargs_set(): variable value too long, increase MAKEARGS_VAR_LENGTH!",
			name, value);
#	undef MAKEARGS_VAR_FMTSTRING

	for (int i = 0; i < makeargs_vars_count; i++)
	{
		if (MAKEARGS_STRCMP(makeargs_vars[i].name, name) == 0)
		{
			MAKEARGS_STRCPY(makeargs_vars[i].value, value);
			return;
		}
	}

	MAKEARGS_STRCPY(makeargs_vars[makeargs_vars_count].name, name);
	MAKEARGS_STRCPY(makeargs_vars[makeargs_vars_count].value, value);
	makeargs_vars_count++;
}

static void makeargs_set_from_var(const char* var)
{
	char* eq_pos = MAKEARGS_STRCHR(var, '=');

	if (eq_pos != NULL)
	{
		char name[MAKEARGS_VAR_LENGTH];
		size_t len = eq_pos - var;
		MAKEARGS_STRNCPY(name, var, len);
		name[len] = '\0';
		makeargs_set(name, eq_pos + 1);
	}
}

static void makeargs_getenv()
{
	for (char** env = MAKEARGS_ENVIRON; *env != NULL; env++)
	{
		makeargs_set_from_var((const char*)env);
	}
}

static int makeargs_set_vars(const int argc, const char** argv)
{
	int i = 1;

	while (i < argc)
	{
		if (MAKEARGS_STRCMP(argv[i], MAKEARGS_SEPARATOR) == 0)
		{
			return i + 1;
		}
		else
		{
			makeargs_set_from_var(argv[i]);
			i++;
		}
	}

	return i;
}

static int makeargs_run_targets(const int argc, const char** argv)
{
	if (argc == 1)
	{
		MAKEARGS_DEFAULT_TARGET(argv[0]);
		return 1;
	}

	int i = 1;

	while (i < argc)
	{
		char* eq_pos = MAKEARGS_STRCHR(argv[i], '=');

		if (eq_pos != NULL)
		{
			i++;
			continue;
		}
		else if (MAKEARGS_STRCMP(argv[i], MAKEARGS_SEPARATOR) == 0)
		{
			return i + 1;
		}
#	define MAKEARGS_TARGET(target, ...)               \
		else if (MAKEARGS_STRCMP(argv[i], #target) == 0) \
		{                                                \
			MAKEARGS_TARGET_CALL(target);                  \
			i++;                                           \
		}

		MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET
		else
		{
			LOG_FPRINTF(LOG_STDERR, "%s:%d: Unknown target %s()\n", __FILE__,
									__LINE__, argv[i]);
			MAKEARGS_DEFAULT_TARGET(argv[0]);
			LOG_EXIT(1);
		}
	}

	return i;
}
#endif	// MAKEARGS_IMPLEMENTATION
// Licensed under the LGPL3 or later versions of the LGPL license.
// See the LICENSE file in the project root for more information.
