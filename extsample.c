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

PHP_RINIT_FUNCTION(extsample)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(extsample)
{
	return SUCCESS;
}

PHP_MINIT_FUNCTION(extsample)
{
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(extsample)
{
	return SUCCESS;
}

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