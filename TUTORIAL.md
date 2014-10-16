# Setting up your environment

My recommended development setup consists of Unix-like system, such as Linux, BSD or Mac OS X. At the time
of writing Linux is recommended due to optimal Valgrind support. Basic tools such as Autotools, pkg-config,
compilers etc should be installed as well. I will add a chapter later regarding compiling your extension on
Windows, but for now the focus shall be on systems like the ones mentioned above.

When doing extension development a debug build of PHP is recommended. The debug build is compiled
without optimisations, which is useful when the debugging with gdb or Valgrind. Debug build can be 
compiled by passing the following configure settings to PHP configure program:

    $ ./configure --enable-debug --disable-all --prefix=/opt/php-debug --enable-maintainer-zts

This will install a debug version of PHP to /opt/php-debug and does not interfere with your system-wide
installation. The enable-maintainer-zts flag enables Zend thread-safety and allows to catch some of the common
development errors early on.

# Basic extension layout

The minimal extension usually consists of three files: config.m4, a header file and an implementation file.
The config.m4 is used by phpize command-line tool to generate 'configure' -script. In a nutshell this file
defines how your extension is built, what libraries are linked etc.

The header file (.h) contains a few definitions (module_entry, version etc) and that's about it. In more complex
extensions there can be several header files, but even then the main header should be kept minimal.

The implementation file (.c) contains the extension implementation. This means module initialisation, per-request
initialisation, phpinfo-hooks, ini-entries and of course functions and classes. 

Naturally this is not the only way to lay out your extension but it's a good starting point. In more complex
extensions you might want to break the implementation into multiple files, for example one per class or one
file for the main extension and one file for helper functions. However, this is a matter of preference and
for the sake of this tutorial I shall not dive deeper into this.

There are a few additional files that will be introduced later, most importantly LICENSE, tests, package.xml and config.w32.

## Versioning

The versioning convention for PHP extensions is major.minor.patch and the initial release of your extension of
your extension should be 0.1.0, rather than for example 0.0.1. I don't know why this is, but it just happens to
be the convention followed in PECL.

## Setting up the boilerplate

Every extension contains certain amount of boilerplate code: the minimal header file, module entry, initialisation
of classes and resources etc.

A few words about naming your extension: If you read the PHP license carefully you will find the following section:

```
  4. Products derived from this software may not be called "PHP", nor
     may "PHP" appear in their name, without prior written permission
     from group@php.net.  You may indicate that your software works in
     conjunction with PHP by saying "Foo for PHP" instead of calling
     it "PHP Foo" or "phpfoo"
```

Hence, choose a name for your extension that does not contain "PHP" in it.

### config.m4

To get started with the development first thing to create is config.m4, the build-file. If your extension does not
link against external libraries this file will be very simple:

```
PHP_ARG_ENABLE(yourextension, whether to enable yourextension,
[  --enable-yourextension               Enable yourextension])

if test "$PHP_YOUREXTENSION" != "no"; then
  PHP_NEW_EXTENSION(yourextension, yourextension.c, $ext_shared)
fi
```

Ever wondered why some extensions use --with-xyz and some use --enable-xyz? The convention here (not enforced) is that
if you extension links against external libraries --with is used and if there are no external libraries involved then
--enable should be used. For example Imagick uses --with-imagick because it links against ImageMagick libraries.


Drafts
------

# Allocating memory

PHP provides it's own memory allocation methods that should be used instead of plain malloc, calloc, free etc.
These functions have "e" -prefix, for example emalloc, estrdup, ecalloc and efree. The memory allocated using
these functions will count towards the PHP memory_limit and it is automatically freed at the end of the request.
Even though the memory is automatically freed at the end of a request it is still good practice to maintain allocation
symmetry and _efree_ everything you _eallocate_.

As the memory is freed automatically at the end of the request these macros are not suitable for allocating memory
that needs to persist over multiple requests (for example persistent resources). There is a separate set of macros
to allocate memory for this purpose: pemalloc, pecalloc etc. These macros are defined as follows:

Zend/zend_alloc.h:
```C
    #define pemalloc(size, persistent) ((persistent)?__zend_malloc(size):emalloc(size))
    #define pefree(ptr, persistent)  ((persistent)?free(ptr):efree(ptr))
```

As you can see from the macro definition if the persistent flag is set to true the macro will use operating system's
allocation methods directly. This also means that all memory allocated using the persistent=1 needs to be freed using
persistent=1, as otherwise crashes might happen.


# zvals

When working with PHP extensions you often encounter a type called "zval". This is a very integral part of the PHP
engine and it represents any PHP variable type. The struct itself is defined in the following manner:

Zend/zend.h:
```C
struct _zval_struct {
    /* Variable information */
    zvalue_value value;     /* value */
    zend_uint refcount__gc;
    zend_uchar type;    /* active type */
    zend_uchar is_ref__gc;
};
```

The important parts here are "type" and "value". The type can be set to any of the "IS_" family values, such as
IS_STRING, IS_ARRAY, IS_LONG etc. Usually however you would access this struct using a plethora of macros available
for working with zvals, usually named Z_*. There are usually several variants of a single macro available for accessing
the zval directly, via a pointer or a pointer to pointer. For accessing the "type" member of the macro you could use
the following macros depending on the situation:

Zend/zend_operators.h:
```C
#define Z_TYPE(zval)        (zval).type
#define Z_TYPE_P(zval_p)    Z_TYPE(*zval_p)
#define Z_TYPE_PP(zval_pp)  Z_TYPE(**zval_pp)
```

As you can see from the definition the _P and _PP variants of the macro simply wrap the plain macro.
Next, let's look at the zvalue_value struct:

Zend/zend.h:
```C
typedef union _zvalue_value {
    long lval;                  /* long value */
    double dval;                /* double value */
    struct {
        char *val;
        int len;
    } str;
    HashTable *ht;              /* hash table value */
    zend_object_value obj;
    zend_ast *ast;
} zvalue_value;
```

The fields in this struct are modified and accessed via the macros depending on the type defined in the zval structure
described earlier. For for a string variable the zval->type would be set to IS_STRING and in this case zval->value->str
contains the value of the string. To access a string variable you would use code similar to:

```C
if (Z_TYPE_P(zval_ptr) == IS_STRING) {
    /*
        Do something with Z_STRVAL_P(zval_ptr) and/or Z_STRLEN_P(zval_ptr)
    */
}
```

## Reference counting

TODO

## Lifecycle

TODO





