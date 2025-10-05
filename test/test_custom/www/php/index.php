#!/usr/bin/env php-cgi
<?php
header("Content-Type: text/html; charset=utf-8");
?>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>PHP CGI Test</title>
</head>
<body>
    <h1>PHP CGI is Working!</h1>
    <p>If you can see this, CGI execution is functioning correctly.</p>
    <ul>
        <li><a href="cgi_demo_php.php">GET request demo</a></li>
        <li><a href="form.php">Form PHP Test</a></li>
	</ul>
    <p>GCI error</p>
    <ul>
        <li><a href="forbidden.php">CGI can't execute script (doesn't have permission - Error 403</a></li>


    </ul>
</body>
</html>

