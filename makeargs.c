#ifndef MAKEARGS_FREESTANDING
#	include <stdio.h>
#	include <stdlib.h>
#	include <stdbool.h>
#	include <string.h>
#	include <errno.h>
#	include <sys/stat.h>
#	include <sys/types.h>
#	include <time.h>

#	ifdef _WIN32
#		include <io.h>
#		include <shlwapi.h>
#		ifdef _MSC_VER
#			pragma comment(lib, "Shlwapi.lib")
#			define _Static_assert static_assert
#		endif
#		define STAT_STRUCT __stat64
#		define STAT_FUNC _stat64
#	else
#		include <fnmatch.h>
#		include <glob.h>
#		define STAT_STRUCT stat
#		define STAT_FUNC stat
#	endif
#endif

#ifndef MAKEARGS_DEF
#	define MAKEARGS_DEF static inline
#endif

/// prints the help message.
/// generated based on your MAKEARGS_TARGETS.
MAKEARGS_DEF void makeargs_help(const char* argv0);

/// returns the value of a variable.
/// halts if the variable is not found.
MAKEARGS_DEF char* makeargs_get(const char* name);

/// appends a to a variable, also returns its pointer.
/// halts if the variable is not found.
/// halts if the appended variable is too long.
MAKEARGS_DEF char* makeargs_append(const char* name, const char* suffix);

/// sets a variable.
/// halts if you have too many variables.
/// halts if the variable name or value is too long.
MAKEARGS_DEF void makeargs_set(const char* name, const char* value);

/// sets a variable from a env variable, like "NAME=value".
/// halts if you have too many variables.
/// halts if the variable name or value is too long.
MAKEARGS_DEF void makeargs_set_from_var(const char* var);

/// sets all the variables based on the environment variables.
/// halts if you have too many variables.
/// halts if the variable name or value is too long.
MAKEARGS_DEF void makeargs_getenv(void);

/// returns true if the output is newer than the dependencies.
/// returns true if the output is "" or the file doesnt exist.
/// halts on unexpected stat() fails.
MAKEARGS_DEF bool makeargs_needs_rebuild(const char* output,
																				 const size_t deps_count,
																				 const char* const deps[]);

/// sets custom and builtin flags based on the command line arguments.
/// halts with DEFAULT_TARGET() if -h or --help is specified.
/// returns the last index or the index after MAKEARGS_SEPARATOR.
MAKEARGS_DEF size_t makeargs_set_flags(const size_t argc, const char** argv);

/// sets all the variables based on the command line arguments.
/// halts if you have too many variables.
/// halts if the variable name or value is too long.
/// returns the last index or the index after MAKEARGS_SEPARATOR.
MAKEARGS_DEF size_t makeargs_set_vars(const size_t argc, const char** argv);

/// runs the targets based on the command line arguments.
/// prints the help message if no target is specified.
/// skips all arguments with '='.
/// returns the last index or the index after MAKEARGS_SEPARATOR.
MAKEARGS_DEF size_t makeargs_run_targets(const size_t argc, const char** argv);

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
/// MAKEARGS_FLAG_BOOL(var, description, usage, flags...)
/// MAKEARGS_FLAG_LIST(list, description, usage, flags...)
#	ifndef MAKEARGS_FLAGS
#		define MAKEARGS_FLAGS                                                     \
			MAKEARGS_FLAG_BOOL(makeargs_dry_run, "print without running anything",   \
												 "-n|--dry-run", "-n", "--dry-run")                    \
			MAKEARGS_FLAG_BOOL(makeargs_always_run, "Unconditionally run targets",   \
												 "-B|--always-run", "-B", "--always-run")              \
			MAKEARGS_FLAG_LIST(makeargs_assumed_old, "never remake <target>",        \
												 "-o|--assume-old <target>", "-o", "--assume-old")     \
			MAKEARGS_FLAG_BOOL(makeargs_silent, "Silent mode, don't print anything", \
												 "-s|--silent|--quiet", "-s", "--silent", "--quiet")   \
			MAKEARGS_FLAG_LIST(makeargs_assumed_new, "Act like <target> is new",     \
												 "-W|--assume-new <target>", "-W", "--assume-new")     \
			MAKEARGS_FLAG_BOOL(makeargs_nocolor, "Removes colors from output",       \
												 "--nocolor", "--nocolor")
#	endif

/// an X macro list for additional flags
/// MAKEARGS_FLAG_BOOL(var, description, usage, flags...)
/// MAKEARGS_FLAG_LIST(list, description, usage, flags...)
#	ifndef MAKEARGS_CUSTOM_FLAGS
#		define MAKEARGS_CUSTOM_FLAGS
#	endif

// clang-format off
#define MAKEARGS_EXIT_OK           0
#define MAKEARGS_EXIT_USAGE        64   /* command line usage error */
#define MAKEARGS_EXIT_NOINPUT      66   /* cannot open input */
#define MAKEARGS_EXIT_SOFTWARE     70   /* internal software error */
#define MAKEARGS_EXIT_IOERR        74   /* input/output error */
#define MAKEARGS_EXIT_CONFIG       78   /* configuration error */
// clang-format on

#	ifndef MAKEARGS_STDOUT
#		define MAKEARGS_STDOUT stdout
#	endif

#	ifndef MAKEARGS_STDERR
#		define MAKEARGS_STDERR stderr
#	endif

#	ifndef MAKEARGS_FPRINTF
#		define MAKEARGS_FPRINTF fprintf
#	endif

#	ifndef MAKEARGS_EXIT
#		define MAKEARGS_EXIT exit
#	endif

#	define MAKEARGS_TERM_RESET "\x1b[0m"
#	define MAKEARGS_TERM_RED "\x1b[31m"
#	define MAKEARGS_TERM_GREEN "\x1b[32m"
#	define MAKEARGS_TERM_YELLOW "\x1b[33m"

#	ifdef MAKEARGS_NOCOLOR
static bool makeargs_nocolor = true;
#		define MAKEARGS_COLOR_STR(color, str) str
#	else
static bool makeargs_nocolor = false;

#		define MAKEARGS_COLOR_STR(color, str) \
			makeargs_nocolor ? str : color str MAKEARGS_TERM_RESET
#	endif

#	define MAKEARGS_PRINT(...)          \
		do                                 \
		{                                  \
			if (!makeargs_silent)            \
				MAKEARGS_FPRINTF(__VA_ARGS__); \
		} while (0)

#	define MAKEARGS_MSG(format, ...) \
		MAKEARGS_PRINT(MAKEARGS_STDOUT, format, ##__VA_ARGS__)

#	define MAKEARGS_HALT(status, format, ...)                             \
		do                                                                   \
		{                                                                    \
			MAKEARGS_PRINT(MAKEARGS_STDERR,                                    \
										 MAKEARGS_COLOR_STR(MAKEARGS_TERM_RED, format "\n"), \
										 ##__VA_ARGS__);                                     \
			MAKEARGS_EXIT(status);                                             \
		} while (0)

#	define MAKEARGS_ASSERT(status, cond, format, ...)  \
		do                                                \
		{                                                 \
			if (!(cond))                                    \
			{                                               \
				MAKEARGS_HALT(status, format, ##__VA_ARGS__); \
			}                                               \
		} while (0)

#	define MAKEARGS_FIRST(x, ...) x
#	define MAKEARGS_REST(x, ...) __VA_ARGS__

#	define MAKEARGS_ARGS_ARRAY(type, ...) \
		(type const[])                       \
		{                                    \
			__VA_ARGS__                        \
		}

#	define MAKEARGS_ARGS_ARRAY_COUNTOF(type, ...) \
		MAKEARGS_COUNTOF(((type const[]){__VA_ARGS__}))

#	define MAKEARGS_COUNTOF(x) (sizeof(x) / sizeof(*(x)))

#	define MAKEARGS_PARAM_STR_ARRAY(...)                    \
		MAKEARGS_ARGS_ARRAY_COUNTOF(const char*, __VA_ARGS__), \
				MAKEARGS_ARGS_ARRAY(const char*, __VA_ARGS__)

/// how the targets will be called
#	ifndef MAKEARGS_TARGET_CALL
#		define MAKEARGS_TARGET_CALL(target) \
			MAKEARGS_MSG("%s()\n", #target);   \
			if (!makeargs_dry_run)             \
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

#	ifndef MAKEARGS_GLOBMATCH
#		if _WIN32
#			define MAKEARGS_GLOBMATCH(pattern, file, flags) \
				PathMatchSpecA(file, pattern)
#		else
#			define MAKEARGS_GLOBMATCH(pattern, file, flags) \
				fnmatch(pattern, file, flags)
#		endif
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
#		define MAKEARGS_VAR_LENGTH 4096
#	endif

static struct
{
	char name[MAKEARGS_VAR_LENGTH];
	char value[MAKEARGS_VAR_LENGTH];
} makeargs_vars[MAKEARGS_MAX_VARS] = {0};
_Static_assert(sizeof(makeargs_vars) < 4 * 1024 * 1024,
							 "makeargs_vars too large!");

static const char* makeargs_assumed_old[MAKEARGS_MAX_VARS];
static size_t makeargs_assumed_old_count = 0;
static const char* makeargs_assumed_new[MAKEARGS_MAX_VARS];
static size_t makeargs_assumed_new_count = 0;

static size_t makeargs_vars_count = 0;
static bool makeargs_dry_run = false;
static bool makeargs_always_run = false;
static bool makeargs_silent = false;

MAKEARGS_DEF void makeargs_help(const char* argv0)
{
	MAKEARGS_MSG("Usage: %s [", argv0);
	const char* sep = "";
#	define MAKEARGS_TARGET(target, ...) \
		MAKEARGS_MSG("%s" #target, sep);   \
		sep = "|";
	MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET
	MAKEARGS_MSG("]\n");

#	define MAKEARGS_TARGET(target, ...) \
		MAKEARGS_MSG("  %-30s %s\n", #target, MAKEARGS_FIRST(__VA_ARGS__) "");

	MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET

	MAKEARGS_MSG("\nFlags:\n");
#	define MAKEARGS_FLAG_DESCRIBE(description, usage) \
		MAKEARGS_MSG("  %-30s %s\n", usage, description);

#	define MAKEARGS_FLAG_LIST(_, description, usage, ...) \
		MAKEARGS_FLAG_DESCRIBE(description, usage)
#	define MAKEARGS_FLAG_BOOL(_, description, usage, ...) \
		MAKEARGS_FLAG_DESCRIBE(description, usage)

	MAKEARGS_FLAGS
	MAKEARGS_CUSTOM_FLAGS
#	undef MAKEARGS_FLAG_BOOL
#	undef MAKEARGS_FLAG_LIST
#	undef MAKEARGS_FLAG_DESCRIBE
}

MAKEARGS_DEF char* makeargs_get(const char* name)
{
	for (size_t i = 0; i < makeargs_vars_count; i++)
	{
		if (MAKEARGS_STRCMP(makeargs_vars[i].name, name) == 0)
		{
			return makeargs_vars[i].value;
		}
	}

	MAKEARGS_HALT(MAKEARGS_EXIT_CONFIG, "%s: Variable '%s' not found!", __func__,
								name);
}

MAKEARGS_DEF char* makeargs_append(const char* name, const char* suffix)
{
	char* value = makeargs_get(name);
	size_t value_len = MAKEARGS_STRLEN(value);

	MAKEARGS_ASSERT(
			MAKEARGS_EXIT_CONFIG,
			value_len + MAKEARGS_STRLEN(suffix) < MAKEARGS_VAR_LENGTH,
			"name = \"%s\"\nvalue = \"%s%s\"\n%s: appended variable is too long, increase MAKEARGS_VAR_LENGTH!",
			name, value, suffix, __func__);

	MAKEARGS_STRCPY(value + value_len, suffix);

	return value;
}

MAKEARGS_DEF void makeargs_set(const char* name, const char* value)
{
#	define MAKEARGS_VAR_FMTSTRING "name = \"%s\"\nvalue = \"%s\"\n"
	MAKEARGS_ASSERT(MAKEARGS_EXIT_CONFIG, name[0] != '\0',
									MAKEARGS_VAR_FMTSTRING "%s: variable has no name!", name,
									value, __func__);
	MAKEARGS_ASSERT(MAKEARGS_EXIT_CONFIG, makeargs_vars_count < MAKEARGS_MAX_VARS,
									"%s: too many variables, increase MAKEARGS_MAX_VARS!",
									__func__);
	MAKEARGS_ASSERT(MAKEARGS_EXIT_CONFIG,
									MAKEARGS_STRLEN(name) < MAKEARGS_VAR_LENGTH,
									MAKEARGS_VAR_FMTSTRING
									"%s: variable name too long, increase MAKEARGS_VAR_LENGTH!",
									name, value, __func__);
	MAKEARGS_ASSERT(MAKEARGS_EXIT_CONFIG,
									MAKEARGS_STRLEN(value) < MAKEARGS_VAR_LENGTH,
									MAKEARGS_VAR_FMTSTRING
									"%s: variable value too long, increase MAKEARGS_VAR_LENGTH!",
									name, value, __func__);
#	undef MAKEARGS_VAR_FMTSTRING

	for (size_t i = 0; i < makeargs_vars_count; i++)
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

MAKEARGS_DEF void makeargs_set_from_var(const char* var)
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

MAKEARGS_DEF void makeargs_getenv(void)
{
	for (char** env = MAKEARGS_ENVIRON; *env != NULL; env++)
	{
		makeargs_set_from_var(*env);

		if (MAKEARGS_STRCMP(*env, "NO_COLOR=1") == 0)
		{
			makeargs_nocolor = true;
		}
	}
}

MAKEARGS_DEF bool _stack_contains(const char* str,
																	const char** strs,
																	const size_t amount)
{
	for (size_t i = 0; i < amount; i++)
	{
		if (MAKEARGS_STRCMP(str, strs[i]) == 0)
		{
			return true;
		}
	}

	return false;
}

MAKEARGS_DEF bool _array_contains(const char* str,
																	const size_t count,
																	const char* const array[])
{
	for (size_t i = 0; i < count; i++)
	{
		if (MAKEARGS_STRCMP(str, array[i]) == 0)
		{
			return true;
		}
	}

	return false;
}

#	if _WIN32
// TODO(LucasTA): glob support on windows
typedef struct
{
	size_t gl_pathc;
	char** gl_pathv;
} glob_t;

MAKEARGS_DEF glob_t _glob_from_strs(const size_t amount,
																		const char* const* strs)
{
	for (size_t i = 0; i < amount; i++)
	{
		if (MAKEARGS_STRCHR(strs[i], '*') != NULL ||
				MAKEARGS_STRCHR(strs[i], '?') != NULL ||
				MAKEARGS_STRCHR(strs[i], '[') != NULL)
		{
			MAKEARGS_MSG(
					MAKEARGS_COLOR_STR(
							MAKEARGS_TERM_YELLOW,
							"glob patterns not supported on Windows (found '%s'), always rebuilding the targets!\n"),
					strs[i]);
			makeargs_always_run = true;
			break;
		}
	}

	return (glob_t){.gl_pathc = amount, .gl_pathv = (char**)strs};
}

#		define globfree(...)
#	else
MAKEARGS_DEF glob_t _glob_from_strs(const size_t amount,
																		const char* const* strs)
{
	glob_t buf = {0};

	for (size_t i = 0; i < amount; i++)
	{
		int flags = (i == 0) ? 0 : GLOB_APPEND;
		glob(strs[i], flags | GLOB_TILDE | GLOB_NOCHECK | GLOB_NOSORT, NULL, &buf);
	}

	return buf;
}
#	endif

MAKEARGS_DEF bool _makeargs_build_out(const char* output,
																			const size_t deps_count,
																			const char* const deps[])
{
	if (output[0] == '\0' ||
			_stack_contains(output, makeargs_assumed_new, makeargs_assumed_new_count))
	{
		return true;
	}

	if (_stack_contains(output, makeargs_assumed_old, makeargs_assumed_old_count))
	{
		return false;
	}

	glob_t glob = _glob_from_strs(deps_count, deps);

	bool result = (makeargs_always_run ||
								 makeargs_needs_rebuild(output, glob.gl_pathc,
																				(const char* const*)glob.gl_pathv));

	globfree(&glob);
	return result;
}

MAKEARGS_DEF void _makeargs_build_deps(const size_t deps_count,
																			 const char* const deps[])
{
	static const char* makeargs_deps_stack[MAKEARGS_MAX_VARS];
	static size_t makeargs_deps_depth = 0;
#	define MAKEARGS_TARGET(target, ...) static bool _first_##target = false;
	MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET

	for (size_t i = 0; i < deps_count; ++i)
	{
// NOTE(LucasTA): checked against '\0' to prevent "" output
#	define MAKEARGS_OUTPUTS(target, description, ...)                         \
		__VA_OPT__(if (MAKEARGS_FIRST(__VA_ARGS__)[0] != '\0' &&                 \
									 MAKEARGS_GLOBMATCH(deps[i], MAKEARGS_FIRST(__VA_ARGS__),  \
																			0) == 0) {                             \
			if (_makeargs_build_out(                                               \
							MAKEARGS_FIRST(__VA_ARGS__),                                   \
							MAKEARGS_PARAM_STR_ARRAY(MAKEARGS_REST(__VA_ARGS__))))         \
			{                                                                      \
				if (_first_##target)                                                 \
				{                                                                    \
					MAKEARGS_PRINT(MAKEARGS_STDERR,                                    \
												 MAKEARGS_COLOR_STR(                                 \
														 MAKEARGS_TERM_RED,                              \
														 "%s: Attempt to build circular dependency!\n"), \
												 __func__);                                          \
					for (size_t j = 0; j < makeargs_deps_depth; j++)                   \
						MAKEARGS_PRINT(MAKEARGS_STDERR,                                  \
													 MAKEARGS_COLOR_STR(MAKEARGS_TERM_RED, "%s -> "),  \
													 makeargs_deps_stack[j]);                          \
					MAKEARGS_HALT(MAKEARGS_EXIT_CONFIG, "%s",                          \
												MAKEARGS_FIRST(__VA_ARGS__));                        \
				}                                                                    \
                                                                             \
				_first_##target = true;                                              \
				MAKEARGS_ASSERT(                                                     \
						MAKEARGS_EXIT_CONFIG, makeargs_deps_depth < MAKEARGS_MAX_VARS,   \
						"%s: dependency stack overflow - too many nested dependencies!", \
						__func__);                                                       \
				makeargs_deps_stack[makeargs_deps_depth++] =                         \
						MAKEARGS_FIRST(__VA_ARGS__);                                     \
				_makeargs_build_deps(                                                \
						MAKEARGS_PARAM_STR_ARRAY(MAKEARGS_REST(__VA_ARGS__)));           \
				MAKEARGS_TARGET_CALL(target)                                         \
				makeargs_deps_depth--;                                               \
				_first_##target = false;                                             \
			}                                                                      \
		})

#	define MAKEARGS_TARGET(target, ...) \
		__VA_OPT__(MAKEARGS_OUTPUTS(target, __VA_ARGS__))

		MAKEARGS_TARGETS
#	undef MAKEARGS_TARGET
#	undef MAKEARGS_OUTPUTS
	}
}

MAKEARGS_DEF bool makeargs_needs_rebuild(const char* output,
																				 const size_t deps_count,
																				 const char* const deps[])
{
	struct STAT_STRUCT st = {0};

	if (STAT_FUNC(output, &st) < 0)
	{
		if (errno == ENOENT)
		{
			// theres no output, empty "" or file doesnt exist
			return true;
		}

		MAKEARGS_HALT(MAKEARGS_EXIT_IOERR, "could not stat(%s): %s!", output,
									strerror(errno));
	}

	time_t output_time = st.st_mtime;

	for (size_t i = 0; i < deps_count; ++i)
	{
		if (STAT_FUNC(deps[i], &st) < 0)
		{
			MAKEARGS_HALT(MAKEARGS_EXIT_NOINPUT, "could not stat(%s): %s!", deps[i],
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

// NOTE(LucasTA): annoying macro to skip the next argument on list flags
MAKEARGS_DEF void _makeargs_skip_if_list_flag(const char** argv, size_t* i)
{
#	define MAKEARGS_FLAG_BOOL(...)
#	define MAKEARGS_FLAG_LIST(list, description, usage, ...)                    \
		else if (_array_contains(argv[*i], MAKEARGS_PARAM_STR_ARRAY(__VA_ARGS__))) \
		{                                                                          \
			(*i)++;                                                                  \
		}

	if (0)
	{
	}
	MAKEARGS_FLAGS
	MAKEARGS_CUSTOM_FLAGS
#	undef MAKEARGS_FLAG_BOOL
#	undef MAKEARGS_FLAG_LIST
}

MAKEARGS_DEF size_t makeargs_set_flags(const size_t argc, const char** argv)
{
	size_t i = 1;

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
			MAKEARGS_EXIT(MAKEARGS_EXIT_OK);
		}
#	define MAKEARGS_FLAG_BOOL(var, description, _, ...)                        \
		else if (_array_contains(argv[i], MAKEARGS_PARAM_STR_ARRAY(__VA_ARGS__))) \
		{                                                                         \
			var = true;                                                             \
		}
#	define MAKEARGS_FLAG_LIST(list, description, usage, ...)                   \
		else if (_array_contains(argv[i], MAKEARGS_PARAM_STR_ARRAY(__VA_ARGS__))) \
		{                                                                         \
			MAKEARGS_ASSERT(MAKEARGS_EXIT_CONFIG, list##_count < MAKEARGS_MAX_VARS, \
											"%s: too many files %s, increase MAKEARGS_MAX_VARS!",   \
											__func__, #list);                                       \
			MAKEARGS_ASSERT(MAKEARGS_EXIT_USAGE, argc > i + 1,                      \
											"%s: missing argument for %s", __func__, argv[i]);      \
			i++;                                                                    \
			list[list##_count] = argv[i];                                           \
			list##_count++;                                                         \
		}

		MAKEARGS_FLAGS
		MAKEARGS_CUSTOM_FLAGS
		else if (argv[i][0] == '-')
		{
			MAKEARGS_PRINT(
					MAKEARGS_STDERR,
					MAKEARGS_COLOR_STR(MAKEARGS_TERM_YELLOW, "%s: Unknown flag %s\n"),
					__func__, argv[i]);
			MAKEARGS_DEFAULT_TARGET(argv[0]);
			MAKEARGS_EXIT(MAKEARGS_EXIT_USAGE);
		}
#	undef MAKEARGS_FLAG_BOOL
#	undef MAKEARGS_FLAG_LIST
		i++;
	}

	return i;
}

MAKEARGS_DEF size_t makeargs_set_vars(const size_t argc, const char** argv)
{
	size_t i = 1;

	while (i < argc)
	{
		if (MAKEARGS_STRCMP(argv[i], MAKEARGS_SEPARATOR) == 0)
		{
			return i + 1;
		}
		else if (argv[i][0] == '-')
		{
			_makeargs_skip_if_list_flag(argv, &i);
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

MAKEARGS_DEF size_t makeargs_run_targets(const size_t argc, const char** argv)
{
	if (argc == 1)
	{
		MAKEARGS_DEFAULT_TARGET(argv[0]);
		return 1;
	}

	size_t i = 1;

	while (i < argc)
	{
		char* eq_pos = MAKEARGS_STRCHR(argv[i], '=');

		if (MAKEARGS_STRCMP(argv[i], MAKEARGS_SEPARATOR) == 0)
		{
			return i + 1;
		}
		else if (eq_pos != NULL || argv[i][0] == '-')
		{
			_makeargs_skip_if_list_flag(argv, &i);
			i++;
			continue;
		}
#	define MAKEARGS_NO_REBUILD(target, ...) MAKEARGS_TARGET_CALL(target)

#	define MAKEARGS_HAS_REBUILD(target, desc, output, ...)                   \
		_makeargs_build_deps(MAKEARGS_PARAM_STR_ARRAY(__VA_ARGS__));            \
                                                                            \
		if (_makeargs_build_out(output, MAKEARGS_PARAM_STR_ARRAY(__VA_ARGS__))) \
		{                                                                       \
			MAKEARGS_TARGET_CALL(target)                                          \
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
			MAKEARGS_PRINT(
					MAKEARGS_STDERR,
					MAKEARGS_COLOR_STR(MAKEARGS_TERM_YELLOW, "%s: Unknown target %s()\n"),
					__func__, argv[i]);
			MAKEARGS_DEFAULT_TARGET(argv[0]);
			MAKEARGS_EXIT(MAKEARGS_EXIT_USAGE);
		}
	}

	return i;
}
#endif	// MAKEARGS_IMPLEMENTATION
// Licensed under the LGPL3 or later versions of the LGPL license.
// See the LICENSE file in the project root for more information.
