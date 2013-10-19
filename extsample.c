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

/* This is the internal structure for out extsample class.
   Effectively it can be used to store data inside the object
   without having to expose the members to userland.

   This structure would commonly hold file handles, library handles
   etc etc.
*/

typedef struct _php_extsample_object  {
	zend_object zo;
} php_extsample_object;

/* Class entry for our extsample class. Each class needs one of these */
static
	zend_class_entry *php_extsample_sc_entry;

/* Object handlers for extsample class. Things like what to do during clone, new etc */
static
	zend_object_handlers extsample_object_handlers;

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
  Declare methods for our extsample class.
*/
static
zend_function_entry php_extsample_class_methods[] =
{
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

	/* Free anything here that you allocate in php_extsample_object_new */

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

PHP_MINIT_FUNCTION(extsample)
{
	/* Here is where we register classes and resources. For now we are going to register
	   extsample class */
	zend_class_entry ce;

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

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(extsample)
{
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
}

/*
  Functions that the extension provides, class methods are separately
*/
zend_function_entry extsample_functions[] = {
	PHP_FE(extsample_version, NULL)
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