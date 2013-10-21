extsample
---------

Contains practical examples of PHP extension writing. Things like handling arrays, returning values,
creating classes and methods, class constants etc.

This extension is not meant to be a full-blown documentation for writing PHP extensions but hopefully
provides useful snippets and patterns to get started.

Development
-----------

I'll be adding random examples from time to time. If you need a specific example, just open an issue.
Pull requests are welcome as well.


Environment
-----------

For the development environment you most likely want a Unix-like system like Linux, Solaris or Mac OS X.
At the moment I favour Linux because Valgrind support for Mac OS X 10.8 is pretty broken. I would also
recommend using debug builds of PHP (./configure --enable-debug) due to optimisations being disabled and
there are additional error messages regarding emalloc leaks.

Naturally if you are developing Windows specific extension this doesn't apply. To get started with building
on Windows see the following link: https://wiki.php.net/internals/windows/stepbystepbuild

Setting up the extension
------------------------

The minimal extension usually composes of three files: .m4, .h and .c file. The config.m4 is used by phpize
command to generate 'configure' script. The header file normally contains inclusion of several headers
like config.h and php.h. The implementation file (.c file) contains the actual code for the extension.

There isn't really enforced naming convention for the files (apart from config.m4) but I tend to favour
naming the header as 'php_extname.h' and the implementation file 'extname.c'. This however is a personal
preference, nothing more.

Naturally you can break the implementation in several header and implementation files but that's more of
personal preference. Usually in extensions composing several classes I tend to break each class into separate
.c file.

Debugging
---------

During debugging your best tools are gdb and valgrind. If you have a debug build of PHP it will tell
you when emalloced memory is leaked but for native allocations there won't be any warnings. The way
to debug memory leaks is following:

    USE_ZEND_ALLOC=0 valgrind php -d extension_dir=modules/ -d extension=extsample.so script.php

USE_ZEND_ALLOC will turn off Zend MM and you can see leaked emalloced memory in valgrind output.
There is an example function called extsample_leak, which can be used to
demonstrate a simple memory leak with emalloced and malloced memory.
