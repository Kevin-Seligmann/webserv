#!/usr/bin/env php
<?php
header("Content-Type: text/html; charset=utf-8");
?>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>PHP CGI Demo</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        table { border-collapse: collapse; margin-top: 15px; }
        td, th { border: 1px solid #ccc; padding: 6px 10px; }
        th { background: #eee; }
    </style>
</head>
<body>
    <h1>PHP CGI Demo</h1>
    <p><strong>Time now:</strong> <?php echo date("Y-m-d H:i:s"); ?></p>

    <h2>GET Parameters</h2>
    <?php if (!empty($_GET)): ?>
        <table>
            <tr><th>Key</th><th>Value</th></tr>
            <?php foreach ($_GET as $key => $value): ?>
                <tr>
                    <td><?php echo htmlspecialchars($key); ?></td>
                    <td><?php echo htmlspecialchars($value); ?></td>
                </tr>
            <?php endforeach; ?>
        </table>
    <?php else: ?>
        <p>No GET parameters received.</p>
    <?php endif; ?>

    <h2>CGI Environment</h2>
    <table>
        <tr><th>Variable</th><th>Value</th></tr>
        <tr><td>REQUEST_METHOD</td><td><?php echo getenv("REQUEST_METHOD"); ?></td></tr>
        <tr><td>QUERY_STRING</td><td><?php echo getenv("QUERY_STRING"); ?></td></tr>
        <tr><td>SERVER_NAME</td><td><?php echo getenv("SERVER_NAME"); ?></td></t

