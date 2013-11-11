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

There are a few additional files that will be introduced later, most importantly tests, package.xml and config.w32.


## Setting up the boilerplate



