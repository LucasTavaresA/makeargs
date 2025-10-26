#ifdef MAKEARGS_IMPLEMENTATION
#ifndef MAKEARGS_SEPARATOR
#define MAKEARGS_SEPARATOR "--"
#endif

#ifndef MAKEARGS_DEFAULT_TARGET
#define MAKEARGS_DEFAULT_TARGET makeargs_help
#endif

#ifndef MAKEARGS_TARGETS
#error "MAKEARGS_TARGETS must be defined"
#endif

#ifndef MAKEARGS_STRLEN
#define MAKEARGS_STRLEN(s) strlen(s)
#endif

#ifndef MAKEARGS_STRCMP
#define MAKEARGS_STRCMP(s1, s2) strcmp(s1, s2)
#endif

#ifndef MAKEARGS_STRCHR
#define MAKEARGS_STRCHR(s, c) strchr(s, c)
#endif

#ifndef MAKEARGS_STRCPY
#define MAKEARGS_STRCPY(s1, s2) strcpy(s1, s2)
#endif

#ifndef MAKEARGS_ENVIRON
#ifdef _WIN32
extern char** _environ;
#define MAKEARGS_ENVIRON _environ
#else
extern char** environ;
#define MAKEARGS_ENVIRON environ
#endif
#endif

#ifndef MAKEARGS_MAX_VARS
#define MAKEARGS_MAX_VARS 256
#endif

#ifndef MAKEARGS_VAR_LENGTH
#define MAKEARGS_VAR_LENGTH 1024
#endif

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
	LOG_PRINTF("Usage: %s [", argv0);
	const char* sep = "";
#define MAKEARGS_TARGET(target, ...) \
	LOG_PRINTF("%s" #target, sep);     \
	sep = "|";
	MAKEARGS_TARGETS
#undef MAKEARGS_TARGET
	LOG_PRINTF("]\n");

#define MAKEARGS_TARGET(target, ...) \
	LOG_PRINTF("  " #target " \t " __VA_ARGS__ "\n");
	MAKEARGS_TARGETS
#undef MAKEARGS_TARGET
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
#define MAKEARGS_VAR_FMTSTRING "name = \"%s\"\nvalue = \"%s\"\n"
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
#undef MAKEARGS_VAR_FMTSTRING

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
		// temporary null-termination turns 'name=value\0' into 'name\0value\0'
		char eq_sign = *eq_pos;
		*eq_pos = '\0';

		makeargs_set(var, eq_pos + 1);

		*eq_pos = eq_sign;
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
	for (int i = 0; i < argc; i++)
	{
		makeargs_set_from_var(argv[i]);
	}
}

static int makeargs_run_targets(const int argc, const char** argv)
{
	if (argc == 1)
	{
		MAKEARGS_DEFAULT_TARGET(argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++)
	{
		char* eq_pos = MAKEARGS_STRCHR(argv[i], '=');

		if (eq_pos != NULL)
		{
			continue;
		}
		else if (MAKEARGS_STRCMP(argv[i], MAKEARGS_SEPARATOR) == 0)
		{
			return i + 1;
		}
#define MAKEARGS_TARGET(target, ...)               \
	else if (MAKEARGS_STRCMP(argv[i], #target) == 0) \
	{                                                \
		LOG_PRINTF("%s()\n", argv[i]);                 \
		target();                                      \
	}

		MAKEARGS_TARGETS
#undef MAKEARGS_TARGET
		else
		{
			LOG_ERROR("%s:%d: Unknown target %s()\n", __FILE__, __LINE__, argv[i]);
			MAKEARGS_DEFAULT_TARGET(argv[0]);
			LOG_EXIT(1);
		}
	}

	return 0;
}
#else
static char* makeargs_get(const char* name);
static void makeargs_append(const char* name, const char* suffix);
static void makeargs_set(const char* name, const char* value);
static void makeargs_set_from_var(const char* var);
static void makeargs_getenv();
static int makeargs_set_vars(const int argc, const char** argv);
static int makeargs_run_targets(const int argc, const char** argv);
#endif	// MAKEARGS_IMPLEMENTATION
// Licensed under the LGPL3 or later versions of the LGPL license.
// See the LICENSE file in the project root for more information.
