--TEST--
Test version function
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php

var_dump (extsample_version ());
?>
--EXPECTF--
string(%d) "%d.%d.%d"
