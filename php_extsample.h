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

#ifndef PHP_EXTSAMPLE_H
# define PHP_EXTSAMPLE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"

#define PHP_EXTSAMPLE_EXTVER "0.0.1"

/*
  Define an entry-point to the extension
*/
extern zend_module_entry extsample_module_entry;
#define phpext_extsample_ptr &extsample_module_entry

#endif /* PHP_EXTSAMPLE_H */