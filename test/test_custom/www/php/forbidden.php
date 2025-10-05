#!/usr/bin/env php
<?php
header("Content-Type: text/html");

echo "<html><body>";
echo "<h1>Hello from CGI (PHP)!</h1>";

echo "<p><strong>Query String:</strong> " . (getenv("QUERY_STRING") ?: "(empty)") . "</p>";
echo "<p><strong>Path Info:</strong> " . (getenv("PATH_INFO") ?: "(empty)") . "</p>";
echo "<p><strong>Request Method:</strong> " . (getenv("REQUEST_METHOD") ?: "(empty)") . "</p>";
echo "<p><strong>Script Name:</strong> " . (getenv("SCRIPT_NAME") ?: "(empty)") . "</p>";

echo "</body></html>";
?>

