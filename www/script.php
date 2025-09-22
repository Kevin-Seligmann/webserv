#!/usr/bin/php
<?php
header("Content-Type: text/plain");

echo "CGI Environment Debug\n";
echo "=====================\n";
echo "PID: " . getmypid() . "\n";
echo "PPID: " . posix_getppid() . "\n";
echo "Script: " . $_SERVER['SCRIPT_FILENAME'] . "\n";
echo "Method: " . $_SERVER['REQUEST_METHOD'] . "\n";
echo "Time: " . date('Y-m-d H:i:s') . "\n";

// Simular trabajo
sleep(2);
echo "Work completed after 2 seconds\n";

// Forzar flush
fflush(STDOUT);
?>