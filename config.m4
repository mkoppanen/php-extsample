PHP_ARG_ENABLE(extsample, whether to enable extsample,
[  --enable-extsample               Enable extsample])

if test "$PHP_EXTSAMPLE" != "no"; then
  #
  # Add libraries and other stuff here (for libexample)
  #  
  #  PHP_ADD_LIBRARY_WITH_PATH(example, $PHP_LIBEXAMPLE_DIR/$PHP_LIBDIR, EXTSAMPLE_SHARED_LIBADD)
  #  PHP_ADD_INCLUDE($PHP_LIBEXAMPLE_INCDIR)
  #  PHP_SUBST(EXTSAMPLE_SHARED_LIBADD)
  #
  #  $PHP_LIBEXAMPLE_DIR and $PHP_LIBEXAMPLE_INCDIR need to be resolved somehow, for example pkg-config
  #

  PHP_NEW_EXTENSION(extsample, extsample.c, $ext_shared)
fi

