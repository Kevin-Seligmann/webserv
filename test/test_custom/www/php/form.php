#!/usr/bin/env php
<?php
header("Content-Type: text/html; charset=utf-8");

$method = $_SERVER['REQUEST_METHOD'];
$form_data = array();

if ($method === 'GET') {
    $form_data = $_GET;
} elseif ($method === 'POST') {
    $contentType = isset($_SERVER['CONTENT_TYPE']) ? $_SERVER['CONTENT_TYPE'] : '';
    if (strpos($contentType, 'application/x-www-form-urlencoded') === false) {
        header("Content-Type: text/plain");
        echo "Error: Expected urlencoded, got " . $contentType . "\n";
        echo "Use /cgi-bin/upload.php for file uploads\n";
        exit(0);
    }
    $form_data = $_POST;
}
?>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Form Handler (PHP)</title>
</head>
<body>
    <h1>Form Data</h1>
    <p><strong>Method:</strong> <?php echo htmlspecialchars($method); ?></p>

    <?php if (!empty($form_data)): ?>
        <h2>Fields:</h2>
        <dl>
            <?php foreach ($form_data as $key => $value): ?>
                <dt><strong><?php echo htmlspecialchars($key); ?>:</strong></dt>
                <dd><?php echo htmlspecialchars($value); ?></dd>
            <?php endforeach; ?>
        </dl>
    <?php else: ?>
        <p>No data received</p>
    <?php endif; ?>

    <?php if ($method === 'GET' && isset($_SERVER['QUERY_STRING']) && $_SERVER['QUERY_STRING'] !== ''): ?>
        <p><small>Query: <?php echo htmlspecialchars($_SERVER['QUERY_STRING']); ?></small></p>
    <?php endif; ?>

    <hr>
    <h3>Test Form</h3>
    <form method="POST">
        <input name="name" placeholder="Name" required>
        <input name="email" type="email" placeholder="Email">
        <button type="submit">Submit</button>
    </form>
    <a href="/">Back</a>
</body>
</html>

