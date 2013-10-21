--TEST--
Test separating zval
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php

$first = 1;
$second = 1;

extsample_separate_zval ($first, $second);
var_dump ($first, $second);

?>
--EXPECT--
string(12) "Hello World!"
int(1)