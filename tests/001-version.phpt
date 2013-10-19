--TEST--
Test version function
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php

var_dump (extsample_version ());

/* I like to end tests with OK because if there is a segfault before it might not be visible */
echo "OK" . PHP_EOL;
?>
--EXPECTF--
string(%d) "%d.%d.%d"
OK
