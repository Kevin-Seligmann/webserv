#!/usr/bin/php-cgi
<?php
echo "Content-Type: text/html\r\n\r\n";
echo "<h1>PHP CGI Test</h1>\n";
echo "<p>REQUEST_METHOD: " . $_SERVER['REQUEST_METHOD'] . "</p>\n";
echo "<p>SCRIPT_NAME: " . $_SERVER['SCRIPT_NAME'] . "</p>\n";
echo "<p>PATH_INFO: " . ($_SERVER['PATH_INFO'] ?? 'not set') . "</p>\n";
echo "<p>QUERY_STRING: " . ($_SERVER['QUERY_STRING'] ?? '') . "</p>\n";
echo "<p>CONTENT_TYPE: " . ($_SERVER['CONTENT_TYPE'] ?? '') . "</p>\n";
echo "<p>CONTENT_LENGTH: " . ($_SERVER['CONTENT_LENGTH'] ?? '') . "</p>\n";
echo "<p>Server working correctly!</p>\n";
?>