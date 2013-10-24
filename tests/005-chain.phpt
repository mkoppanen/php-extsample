--TEST--
Test chain method
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php

$sample = new ExtSample ();
$obj = $sample->chain ()->chain ()->chain ();

var_dump ($obj === $sample);

echo "OK" . PHP_EOL;
?>
--EXPECT--
bool(true)
OK
