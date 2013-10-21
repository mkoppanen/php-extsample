/*
  +----------------------------------------------------------------------+
  | Copyright (c) 2013 The PHP Group                                     |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Mikko Koppanen <mkoppanen@php.net>                          |
  +----------------------------------------------------------------------+
*/

#include "php_extsample.h"

/* For stuff used in PHP_MINFO_FUNCTION */
#include "ext/standard/info.h"

/* For smart strings */
#include "ext/standard/php_smart_str.h"

/*
	Define our globals. These are used for ini-settings.
	Also, if you need global variables do not use static C globals,
	rather put the variables in this structure because that way they
	place nice with ZTS

	NEEDED_FOR_INI
*/
ZEND_BEGIN_MODULE_GLOBALS(extsample)
	char *default_name; /* This is our ini-setting, extsample.default_name */
ZEND_END_MODULE_GLOBALS(extsample)

/*
	Declare the globals defined above
*/
ZEND_DECLARE_MODULE_GLOBALS(extsample)

#ifdef ZTS
#define EXTSAMPLE_G(v) TSRMG(extsample_globals_id, zend_extsample_globals *, v)
#else
#define EXTSAMPLE_G(v) (extsample_globals.v)
#endif

/* This is the internal structure for out extsample class.
   Effectively it can be used to store data inside the object
   without having to expose the members to userland.

   This structure would commonly hold file handles, library handles
   etc etc.
*/

typedef struct _php_extsample_object  {
	zend_object zo;
	char *name;
} php_extsample_object;

/* Class entry for our extsample class. Each class needs one of these */
static
	zend_class_entry *php_extsample_sc_entry;

/* Object handlers for extsample class. Things like what to do during clone, new etc */
static
	zend_object_handlers extsample_object_handlers;

/* {{{ proto ExtSample ExtSample::__construct(string $name)
    Construct a new extsample object
*/
PHP_METHOD(extsample, __construct)
{
	php_extsample_object *intern;

	/*
		Name is optional parameter so we need to initialise manually
	*/
	char *name = NULL;
	int name_len = 0;

	/*
		Parse one string parameter, name. Notice that name_len has to be int, not long 
		otherwise it causes problems on platforms where long and int are different size
		| makes parameters after it optional
	*/
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &name, &name_len) == FAILURE) {
		/*
			Notice that if parameter parsing has failed in constructor then we return here
			any possible initialisation that happens after this would not be executed and
			you might end up with half built object. In some cases it's best to either avoid
			constructor parameters, make them optional or throw exception on failure
		*/
		return;
	}

	/* 
		This is how you get access to the internal structure allocated in php_extsample_object_new
	*/
	intern = (php_extsample_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	/*
		We store the name into the struct using estrdup if it's set
		It's essentially same as strdup but uses PHP memory management
	*/
	if (name_len) {
		intern->name = estrdup (name);
	} else {
		/*
			If user did not provide a name, use the ini-setting extsample.default_name
		*/
		intern->name = estrdup (EXTSAMPLE_G(default_name));
	}
}
/* }}} */

/* {{{ proto string ExtSample::getName()
    Get name stored into extsample object
*/
PHP_METHOD(extsample, getname)
{
	php_extsample_object *intern;

	/* This method takes no parameters */
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	/* Get our object structure */
	intern = (php_extsample_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	/*
		If no name is set we return PHP null. Note that you cannot just "return NULL;" because
		the C function return void. We use PHP macros to set the return value correctly
	*/
	if (!intern->name) {
		/* This expands to:
		
		{ RETVAL_NULL(); return;}
		
		*/
		RETURN_NULL();
	}

	/* If we have a name, then return a copy of it. We have to copy because otherwise the 
	   returned string might be freed when the return value goes out of scope and that would
	   a dangling pointer inside the object
	*/
	RETURN_STRING(intern->name, 1);
}
/* }}} */

/* Get the type of a zval * in a character array */
static
char *s_get_zval_type (zval *value)
{
	/*
		Z_TYPE_P returns the type of a zval pointer
		There are macros for Z_TYPE and Z_TYPE_PP, the former is for zval
		latter is for zval **
	*/
	switch (Z_TYPE_P(value)) {
		case IS_NULL:
		return estrdup ("IS_NULL");

		case IS_LONG:
		return estrdup ("IS_LONG");

		case IS_DOUBLE:
		return estrdup ("IS_DOUBLE");

		case IS_BOOL:
		return estrdup ("IS_BOOL");

		case IS_ARRAY:
		return estrdup ("IS_ARRAY");

		case IS_OBJECT:
		return estrdup ("IS_OBJECT");

		case IS_STRING:
		return estrdup ("IS_STRING");

		case IS_RESOURCE:
		return estrdup ("IS_RESOURCE");

		case IS_CONSTANT:
		return estrdup ("IS_CONSTANT");

		case IS_CONSTANT_ARRAY:
		return estrdup ("IS_CONSTANT_ARRAY");

#if defined(IS_CALLABLE)
		case IS_CALLABLE:
		return estrdup ("IS_CALLABLE");
#endif

		default:
			return estrdup ("IS_UNKNOWN");
	}
}

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3)
static int s_get_element_type(zval **ppzval, int num_args, va_list args, zend_hash_key *hash_key)
{
	/* The older version doesn't pass TSRMLS_DC in arguments so we need to call this macro*/
	TSRMLS_FETCH();
#else
static int s_get_element_type(zval **ppzval TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
#endif
	zval *return_value;
	char *type_name;

	/* Pick up the return_value from the va_list */
	return_value = va_arg(args, zval *);

	/*
		Next, ppzval contains our current array item. What we want to do is to check the type
		and add it to return_value with same key. So for example:
		array (1 => 'hello', 'foo' => 2) would return array (1 => 'IS_STRING', 'foo' => IS_LONG)
	*/
	type_name = s_get_zval_type (*ppzval);

	/* If key length is 0 it means that the index is integer rather than associative */
	if (!hash_key->nKeyLength) {
		/* If it's an integer key, this is how you can access it
			The last parameter is whether we copy the string to array.
			In this case we don't copy, because it's coming from estrdup
			above and we have no use for it later. Hence we give the ownership
			away and it will be freed when return value goes out of scope
		*/
		add_index_string(return_value, hash_key->h, type_name, 0);

	} else {
		/*
			In this case we have a string key, let's add it to the array
			using add_assoc_string instead of add_index_string. Also, like
			with integer key don't copy string
		*/
		add_assoc_string(return_value, hash_key->arKey, type_name, 0);
	}

	/* This return value continues iteration. We could also return:
		ZEND_HASH_APPLY_REMOVE - Continues iteration but removes current element
		ZEND_HASH_APPLY_STOP - Stop iteration
	*/
	return ZEND_HASH_APPLY_KEEP;
}

/* {{{ proto string ExtSample::arrayValueTypes(array $arr)
    Handling array input parameter. Takes array as parameter and returns an array containing 
	type for each key
*/
PHP_METHOD(extsample, arrayvaluetypes)
{
	/* The array will be given as zval ptr */
	zval *array;

	/* "a" to receive array parameter */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &array) == FAILURE) {
		return;
	}

	/*
		There are two main ways to iterate over an array: 
		Callback based iteration and iterating by getting the HashTable inside the zval * and using a for loop
	*/

	/*
		This method returns an array so we need to initialise the return value to array
		'return_value' is a zval pointer, which contains the value returned. 
		Normally you would use things RETVAL_STRING, RETURN_STRING etc but for array 
		it's a bit different
	*/
	array_init(return_value);

	/*
		The signature changed between PHP 5.2 and PHP 5.3.
		If you are not inclined to support PHP 5.2 just use the newer version.
	*/
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3)
	zend_hash_apply_with_arguments(Z_ARRVAL_P(array), (apply_func_args_t) s_get_element_type, 1, return_value);
#else
	/*
		The arguments are straight-forward:
		First pass the array zval * to Z_ARRVAL_P which gives the HashTable member
		Next is TSRMLS_CC, I think this stands for Thread-Safe Resource Manager Local Storage Comma Call. This is for handling 
		ZTS. The TSRMLS_CC version expands to a comma before it.
		Next comes our callback function and indication that we pass one extra argument and that argument is the return_value
	*/
	zend_hash_apply_with_arguments(Z_ARRVAL_P(array) TSRMLS_CC, (apply_func_args_t) s_get_element_type, 1, return_value);
#endif
}
/* }}} */

/* {{{ proto string extsample_version()
	Returns the extsample version
*/
PHP_FUNCTION(extsample_version)
{
	/* The function takes no arguments */
	if(zend_parse_parameters_none() == FAILURE) {
		return;
	}

	/* Return a string, the second parameter implicates whether to copy the 
		string or not. In general you would copy everything that is not allocated
		with emalloc or similar because the value is later efreed
	*/
	RETURN_STRING(PHP_EXTSAMPLE_EXTVER, 1);
}
/* }}} */

/* {{{ proto string extsample_stream_fetch(string $dsn[, integer $timeout_sec])
	Simple demonstration of using streams. Connects to an endpoint, sends GET / HTTP/1.0\r\n\r\n and waits response.
	Call it like $contents = extsample_stream_fetch ("tcp://valokuva.org:80");
*/
PHP_FUNCTION(extsample_stream_fetch)
{
	/*
		I guess mainly due to Windows, PHP uses C89 standard so you have
		to declare variables at the beginning of a scope
	*/
	size_t written, write_size;
	php_stream *stream;
	char *err_msg = NULL;
	int err_code = 0;
	struct timeval tv;
	char *dsn, *line;
	int dsn_len;

	/*
		We need to initialise timeout because it's an optional parameter
		so zend_parse_parameters will not touch it
	*/
	long timeout = 10;
	char buffer [1024];

	/*
		Take one string parameter
	*/
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &dsn, &dsn_len, &timeout) == FAILURE) {
		return;
	}

	/*
		Setup timeout for the fetch
	*/
	tv.tv_sec  = timeout;
	tv.tv_usec = 0;

	/* Create a new PHP stream transport */
	stream = php_stream_xport_create (dsn, dsn_len, 0, STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, NULL, &tv, NULL, &err_msg, &err_code);

	/*
		If we failed to open a stream, print out a warning and return false. You could also throw an exception here
		RETURN_FALSE will set return_value to false and return. RETVAL_FALSE would just set return_value but not return
	*/
	if (!stream) {
		/* This is how you do a PHP warning, could also throw exception here */
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", err_msg);
		RETURN_FALSE;
	}

	/*
		First write our request data
	*/
	write_size = sizeof("GET / HTTP/1.0\r\n\r\n") - 1;
	written = php_stream_write(stream, "GET / HTTP/1.0\r\n\r\n", write_size);

	/*
		Failed to write our data, fail here
	*/
	if (write_size < written) {
		/*
			Could show a warning here as well
		*/
		php_stream_close(stream);
		RETURN_FALSE;
	}

	/*
		smart_str is a php string type, which allows creating automatically
		expanding strings. For more information see the ext/standard/php_smart_str.h
		header
	*/
	smart_str str = {0};

	/*
		Read the stream until we meet eof
	*/
	while(!php_stream_eof(stream)) {
		/*
			Read a block from the stream into our buffer
		*/
		size_t bytes_read = php_stream_read(stream, buffer, sizeof (buffer));
		if (bytes_read <= 0) {
			break;
		}
		/*
			Append the received buffer into the smart_str. The smart_str
			takes care of allocating space for added strings
		*/
		smart_str_appendl(&str, buffer, bytes_read);
	}
	/*
		After everything has been read, close the stream
	*/
	php_stream_close(stream);

	/*
		We append our string to return_value using this macro
	*/
	RETVAL_STRINGL(str.c, str.len, 1);

	/*
		And free the buffer as we copied the string above
	*/
	smart_str_free (&str);
}
/* }}} */

/* {{{ proto void extsample_leak()
	This function will leak memory emalloced and malloced
*/
PHP_FUNCTION(extsample_leak)
{
	char *eptr, *ptr;

	/* The function takes no arguments */
	if(zend_parse_parameters_none() == FAILURE) {
		return;
	}

	/*
		emalloc 5 bytes of memory
	*/
	eptr = emalloc (5);

	/*
		malloc 5 bytes of memory
	*/
	ptr = malloc (5);

	/*
		We exit here so the memory will leak. You should see
		this in valgrind.
	*/
}
/* }}} */

PHP_RINIT_FUNCTION(extsample)
{
	/*
		Do any per request initialisation here. Not used commonly
		and also note that doing heavy processing here will affect
		every request. If possible it's always better to use MINIT
	*/
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(extsample)
{
	/*
		Executed during request shutdown
	*/
	return SUCCESS;
}

/*
	Argument info for the constructor. I think this is used by reflection
	The last number defines how many mandatory arguments the function takes
*/
ZEND_BEGIN_ARG_INFO_EX(extsample_construct_args, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

/*
	Argument info for getname
*/
ZEND_BEGIN_ARG_INFO_EX(extsample_getname_args, 0, 0, 0)
ZEND_END_ARG_INFO()

/*
	Argument info for getname
*/
ZEND_BEGIN_ARG_INFO_EX(extsample_arrayvaluetypes_args, 0, 0, 1)
	ZEND_ARG_INFO(0, array)
ZEND_END_ARG_INFO()

/*
  Declare methods for our extsample class.
*/
static
zend_function_entry php_extsample_class_methods[] =
{
	/* Constructor entry in the class methods, otherwise identical to others except has ZEND_ACC_CTOR */
	PHP_ME(extsample, __construct, extsample_construct_args,   ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	/* Other methods below */
	PHP_ME(extsample, getname, extsample_getname_args,   ZEND_ACC_PUBLIC)
	/* For this method we don't need the internal structure so mark it as static */
	PHP_ME(extsample, arrayvaluetypes, extsample_arrayvaluetypes_args,   ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{ NULL, NULL, NULL }
};

/* This function is called while the object is being destroyed but 
   after destructors have been called */
static
void php_extsample_object_free_storage(void *object TSRMLS_DC)
{
	php_extsample_object *intern = (php_extsample_object *)object;

	if (!intern) {
		return;
	}

	/*
		During destruction we check if name is set and efree it.
	*/
	if (intern->name) {
		efree(intern->name);
	}

	zend_object_std_dtor(&intern->zo TSRMLS_CC);
	efree(intern);
}

/* Compatibility macro for pre PHP 5.4 versions to initialise properties */
#if PHP_VERSION_ID < 50399 && !defined(object_properties_init)
# define object_properties_init(zo, class_type) { \
			zval *tmp; \
			zend_hash_copy((*zo).properties, \
							&class_type->default_properties, \
							(copy_ctor_func_t) zval_add_ref, \
							(void *) &tmp, \
							sizeof(zval *)); \
		 }
#endif

/*
  This function gets called when a new instance of ExtSample class is created
  but before calling constructor. Any internal initialisation of structures that
  are per object can be done here
*/
static
zend_object_value php_extsample_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	/* Allocate space for our object using PHP emalloc */
	php_extsample_object *intern = emalloc (sizeof (php_extsample_object));

	/* Init the 'name' member to NULL */
	intern->name = NULL;

	/* Initialise zend object member */
	memset(&intern->zo, 0, sizeof(zend_object));
	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);

	/* Initialise object properties, there is a macro above for older PHP versions */
	object_properties_init(&intern->zo, class_type);

	/* Put this object into Zend Object Store and assign a destructor (what gets called when the object is destroyed) */
	retval.handle = zend_objects_store_put(intern, NULL, (zend_objects_free_object_storage_t) php_extsample_object_free_storage, NULL TSRMLS_CC);

	/* Assign the object handlers initialised in MINIT */
	retval.handlers = &extsample_object_handlers;

	/* Return the object */
	return retval;
}

/* 
	Define ini entries here. You can define the name of the ini-setting, default value. The possible settings for where the
	configuration can be changed are: PHP_INI_USER, PHP_INI_PERDIR, PHP_INI_SYSTEM and PHP_INI_ALL

	Usually you would choose where the setting can be changed based on how it is used. For example if you want to access
	the setting during RINIT stage then you would want PHP_INI_PERDIR because the setting would have no use after RINIT.
	
	The change callback function OnUpdate* depends on the type of your setting, for example OnUpdateBool, OnUpdateString, OnUpdateStringUnempty etc
	In outr case OnUpdateStringUnempty updates the value but disallows empty values

	The fifth parameter defines the struct member name in the globals that will be changed when this setting is changed.
	
	I'll mark all things needed for ini-settings to work with NEEDED_FOR_INI
*/
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("extsample.default_name",	"Default Name",	PHP_INI_ALL, OnUpdateStringUnempty,	default_name,	zend_extsample_globals,	extsample_globals)
PHP_INI_END()

/*
	Initialise the globals. Called from MINIT
	NEEDED_FOR_INI
*/
static
void s_extsample_init_globals(zend_extsample_globals *extsample_globals)
{
	extsample_globals->default_name = NULL;
}

PHP_MINIT_FUNCTION(extsample)
{
	/* Here is where we register classes and resources. For now we are going to register
	   extsample class */
	zend_class_entry ce;

	/*
		Initialize globals. The two last parameters are ctor and dtor in that order. If you have
		globals that you need to destroy manually you can register a destructor here. In this case
		we just register NULL.
		NEEDED_FOR_INI
	*/
	ZEND_INIT_MODULE_GLOBALS(extsample, s_extsample_init_globals, NULL);

	/* Initialise the object handlers to standard object handlers (i.e sane defaults) */
	memcpy (&extsample_object_handlers, zend_get_std_object_handlers (), sizeof (zend_object_handlers));

	/* Initialise the class entry and assign methods to the class entry */
	INIT_CLASS_ENTRY(ce, "ExtSample", php_extsample_class_methods);

	/* This is what gets called when a new object of our type is created, allows to initialise
	   members in the 'php_extsample_object' before constructor gets invoked */
	ce.create_object = php_extsample_object_new;

	/* Do not allow cloning for now, results to "tried to clone uncloneable object" */
	extsample_object_handlers.clone_obj = NULL;

	/* Register our class entry with the engine */
	php_extsample_sc_entry = zend_register_internal_class(&ce TSRMLS_CC);

	/*
		Register our ini-settings
		NEEDED_FOR_INI
	*/
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(extsample)
{
	/*
		Unregister the ini entries during module shutdown
		NEEDED_FOR_INI
	*/
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/*
	This function gets executed during phpinfo to display information about the extension.
	There is a correspending PHP_INFO(extsample) entry in the extsample_module_entry
*/
PHP_MINFO_FUNCTION(extsample)
{
	php_info_print_table_start();

		php_info_print_table_header(2, "extsample extension", "enabled");
		php_info_print_table_row(2, "extsample extension version", PHP_EXTSAMPLE_EXTVER);
		/* Add more rows here */

	php_info_print_table_end();

	/*
		This is not actually needed for ini-settings but it's handy. This macro
		call shows all extension ini-settings automatically ini phpinfo()
		NEEDED_FOR_INI
	*/
	DISPLAY_INI_ENTRIES();
}

ZEND_BEGIN_ARG_INFO_EX(extsample_version_args, 0, 0, 0)
ZEND_END_ARG_INFO()

/*
	One mandatory argument and one optional
*/
ZEND_BEGIN_ARG_INFO_EX(extsample_stream_fetch_args, 0, 0, 1)
	ZEND_ARG_INFO(0, dsn)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(extsample_leak_args, 0, 0, 0)
ZEND_END_ARG_INFO()

/*
  Functions that the extension provides, class methods are separately
*/
zend_function_entry extsample_functions[] = {
	PHP_FE(extsample_version, extsample_version_args)
	PHP_FE(extsample_stream_fetch, extsample_stream_fetch_args)
	PHP_FE(extsample_leak, extsample_leak_args)
	/* Add more PHP_FE entries here, the last entry needs to be NULL, NULL, NULL */
	{NULL, NULL, NULL}
};

/**
 The standard module structure for PHP extension
 RINIT and RSHUTDOWN can be NULL if there is nothing to do during request startup / shutdown
*/
zend_module_entry extsample_module_entry =
{
	STANDARD_MODULE_HEADER,
	"extsample"					/* Name of the extension */,
	extsample_functions,		/* This is where you would functions that are not part of classes */
	PHP_MINIT(extsample),		/* Executed once during module initialisation, not per request */
	PHP_MSHUTDOWN(extsample),	/* Executed once during module shutdown, not per request */
	PHP_RINIT(extsample),		/* Executed every request, before script is executed */
	PHP_RSHUTDOWN(extsample),	/* Executed every request, after script has executed */
	PHP_MINFO(extsample),		/* Hook for displaying info in phpinfo() */
	PHP_EXTSAMPLE_EXTVER,		/* Extension version, defined in php_extsample.h */
	STANDARD_MODULE_PROPERTIES
};

/*
  See definition of ZEND_GET_MODULE in zend_API.h around line 133
  Expands into get_module call which returns the zend_module_entry
*/
#ifdef COMPILE_DL_EXTSAMPLE
ZEND_GET_MODULE(extsample)
#endif /* COMPILE_DL_EXTSAMPLE */