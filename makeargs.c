#ifndef MAKEARGS_FREESTANDING
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
#	include <errno.h>
#	include <sys/stat.h>
#	include <sys/types.h>
#	include <time.h>

#	ifdef _WIN32
#		include <io.h>
#		define STAT_STRUCT _stat
#		define STAT_FUNC _stat
#	else
#		define STAT_STRUCT stat
#		define STAT_FUNC stat
#	endif

#	include "log.c"
#	include "span/span.c"
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
static void makeargs_getenv(void);

/// returns true if the output is newer than the dependencies.
/// returns true if the output is "" or the file doesnt exist.
/// halts on unexpected stat() fails.
static bool makeargs_needs_rebuild(char* output, string_span deps);

/// sets custom and builtin flags to true based on the command line arguments.
/// halts with DEFAULT_TARGET() if -h or --help is specified.
/// returns the last index or the index after MAKEARGS_SEPARATOR.
static int makeargs_set_flags(const int argc, const char** argv);

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

/// an X macro list of all your targets
/// name is required, the rest is optional, you can have 10 dependencies
/// MAKEARGS_TARGET(name, description, output, dependencies...)
#	ifndef MAKEARGS_TARGETS
#		define MAKEARGS_TARGETS
#	endif

/// an X macro list of flags
/// the variables are always booleans, for passing other values use variables instead
/// MAKEARGS_FLAG(value, description, flag)
#	ifndef MAKEARGS_FLAGS
#		define MAKEARGS_FLAGS                                                     \
			MAKEARGS_FLAG(_makeargs_dry_run, "print without running anything", "-n") \
			MAKEARGS_FLAG(_makeargs_dry_run, "print without running anything",       \
										"--dry-run")                                               \
			MAKEARGS_FLAG(_makeargs_always_run, "Unconditionally run targets", "-B") \
			MAKEARGS_FLAG(_makeargs_always_run, "Unconditionally run targets",       \
										"--always-run")
#	endif

/// an X macro list of flags
/// the variables are always booleans, for passing other values use variables instead
/// MAKEARGS_FLAG(value, description, flag)
#	ifndef MAKEARGS_CUSTOM_FLAGS
#		define MAKEARGS_CUSTOM_FLAGS
#	endif

#	define MAKEARGS_FIRST(x, ...) x
#	define MAKEARGS_REST(x, ...) __VA_ARGS__

/// how the targets will be called
#	ifndef MAKEARGS_TARGET_CALL
#		define MAKEARGS_TARGET_CALL(target) \
			LOG_MSG("%s()\n", #target);        \
			if (!_makeargs_dry_run)            \
				target();
#	endif

#	ifndef MAKEARGS
#		define MAKEARGS(argc, argv)      \
			makeargs_getenv();              \
			makeargs_set_flags(argc, argv); \
			makeargs_set_vars(argc, argv);  \
			makeargs_run_targets(argc, argv);
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
static bool _makeargs_dry_run = false;
static bool _makeargs_always_run = false;

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
		LOG_MSG("  %-18s %s\n", #target, MAKEARGS_FIRST(__VA_ARGS__) "");

	MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET

	LOG_MSG("\nFlags:\n");
#	define MAKEARGS_FLAG(_, description, flag) \
		LOG_MSG("  %-18s  %s\n", flag, description);

	MAKEARGS_FLAGS
	MAKEARGS_CUSTOM_FLAGS
#	undef MAKEARGS_FLAG
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

static void makeargs_getenv(void)
{
	for (char** env = MAKEARGS_ENVIRON; *env != NULL; env++)
	{
		makeargs_set_from_var((const char*)env);
	}
}

static void _makeargs_build_deps(string_span deps)
{
	static const char* _makeargs_stack[256];
	static int _makeargs_depth = 0;
#	define MAKEARGS_TARGET(target, ...) static bool _first_##target = false;
	MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET

	for (size_t i = 0; i < deps.size; ++i)
	{
#	define MAKEARGS_OUTPUTS(target, description, ...)                         \
		__VA_OPT__(else if (MAKEARGS_FIRST(__VA_ARGS__)[0] != '\0' &&            \
												MAKEARGS_STRCMP(deps.data[i],                        \
																				MAKEARGS_FIRST(__VA_ARGS__)) == 0) { \
			if (_makeargs_always_run ||                                            \
					makeargs_needs_rebuild(MAKEARGS_FIRST(__VA_ARGS__),                \
																 STRING_SPAN(MAKEARGS_REST(__VA_ARGS__))))   \
			{                                                                      \
				if (_first_##target)                                                 \
				{                                                                    \
					LOG_FPRINTF(LOG_STDERR,                                            \
											"%s:%d: Attempt to build circular dependency!\n",      \
											__FILE__, __LINE__);                                   \
					for (int j = 0; j < _makeargs_depth; j++)                          \
						LOG_FPRINTF(LOG_STDERR, "%s -> ", _makeargs_stack[j]);           \
					LOG_HALT(LOG_ERROR_CODE, "%s", MAKEARGS_FIRST(__VA_ARGS__));       \
				}                                                                    \
                                                                             \
				_first_##target = true;                                              \
				LOG_ASSERT(                                                          \
						_makeargs_depth < 256,                                           \
						"dependency stack overflow - too many nested dependencies!");    \
				_makeargs_stack[_makeargs_depth++] = MAKEARGS_FIRST(__VA_ARGS__);    \
				_makeargs_build_deps(STRING_SPAN(MAKEARGS_REST(__VA_ARGS__)));       \
				MAKEARGS_TARGET_CALL(target)                                         \
				_makeargs_depth--;                                                   \
				_first_##target = false;                                             \
			}                                                                      \
		})

#	define MAKEARGS_TARGET(target, ...) \
		__VA_OPT__(MAKEARGS_OUTPUTS(target, __VA_ARGS__))

		if (0)
		{
		}
		MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET
#	undef MAKEARGS_OUTPUTS
	}
}

static bool makeargs_needs_rebuild(char* output, string_span deps)
{
	struct STAT_STRUCT st = {0};

	if (STAT_FUNC(output, &st) < 0)
	{
		if (errno == ENOENT)
		{
			// theres no output, empty "" or file doesnt exist
			return true;
		}

		LOG_HALT(LOG_ERROR_CODE, "could not stat(%s): %s!", output,
						 strerror(errno));
	}

	time_t output_time = st.st_mtime;

	for (size_t i = 0; i < deps.size; ++i)
	{
		if (STAT_FUNC(deps.data[i], &st) < 0)
		{
			LOG_HALT(LOG_ERROR_CODE, "could not stat(%s): %s!", deps.data[i],
							 strerror(errno));
		}

		time_t dep_time = st.st_mtime;

		if (difftime(dep_time, output_time) > 0)
		{
			// a dependency is newer than the output
			return true;
		}
	}

	return false;
}

static int makeargs_set_flags(const int argc, const char** argv)
{
	int i = 1;

	while (i < argc)
	{
		if (MAKEARGS_STRCMP(argv[i], MAKEARGS_SEPARATOR) == 0)
		{
			return i + 1;
		}
		else if ((MAKEARGS_STRCMP(argv[i], "-h") == 0) ||
						 (MAKEARGS_STRCMP(argv[i], "--help") == 0))
		{
			MAKEARGS_DEFAULT_TARGET(argv[0]);
			LOG_EXIT(0);
		}
#	define MAKEARGS_FLAG(var, description, flag)   \
		else if (MAKEARGS_STRCMP(argv[i], flag) == 0) \
		{                                             \
			var = true;                                 \
		}

		MAKEARGS_FLAGS
		MAKEARGS_CUSTOM_FLAGS
#	undef MAKEARGS_FLAG
		i++;
	}

	return i;
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
		else if (argv[i][0] == '-')
		{
			i++;
			continue;
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

		if (MAKEARGS_STRCMP(argv[i], MAKEARGS_SEPARATOR) == 0)
		{
			return i + 1;
		}
		else if (eq_pos != NULL || argv[i][0] == '-')
		{
			i++;
			continue;
		}
#	define MAKEARGS_NO_REBUILD(target, ...) MAKEARGS_TARGET_CALL(target)

#	define MAKEARGS_HAS_REBUILD(target, desc, output, ...)             \
		string_span deps = STRING_SPAN(__VA_ARGS__);                      \
		_makeargs_build_deps(deps);                                       \
                                                                      \
		if (_makeargs_always_run || makeargs_needs_rebuild(output, deps)) \
		{                                                                 \
			MAKEARGS_TARGET_CALL(target)                                    \
		}

#	define MAKEARGS_DISPATCH_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, \
																 _12, N, ...)                                  \
		MAKEARGS_##N##_REBUILD
#	define MAKEARGS_DISPATCH(...)                                           \
		MAKEARGS_DISPATCH_IMPL(__VA_ARGS__, HAS, HAS, HAS, HAS, HAS, HAS, HAS, \
													 HAS, HAS, HAS, NO, NO)

#	define MAKEARGS_TARGET(target, ...)                         \
		else if (MAKEARGS_STRCMP(argv[i], #target) == 0)           \
		{                                                          \
			MAKEARGS_DISPATCH(__VA_ARGS__)(target, __VA_ARGS__) i++; \
		}

		MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET
#	undef MAKEARGS_DISPATCH
#	undef MAKEARGS_DISPATCH_IMPL
#	undef MAKEARGS_HAS_REBUILD
#	undef MAKEARGS_NO_REBUILD
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
