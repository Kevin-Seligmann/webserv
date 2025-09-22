#!/usr/bin/php
<?php
header("Content-Type: text/html");
echo "<h1>PHP CGI Works!</h1>\n";
echo "<p>Process ID: " . getmypid() . "</p>\n";
echo "<p>Current Time: " . date('Y-m-d H:i:s') . "</p>\n";