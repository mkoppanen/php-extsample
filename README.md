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

Debugging
---------

During debugging your best tools are gdb and valgrind. If you have a debug build of PHP it will tell
you when emalloced memory is leaked but for native allocations there won't be any warnings. The way
to debug memory leaks is following:

    USE_ZEND_ALLOC=0 valgrind php -d extension_dir=modules/ -d extension=extsample.so script.php

USE_ZEND_ALLOC will turn off Zend MM and you can see leaked emalloced memory in valgrind output.
There is an example function called extsample_leak, which can be used to
demonstrate a simple memory leak with emalloced and malloced memory.
