#!/usr/bin/env php
<?php
header("Content-Type: text/html; charset=utf-8");

$method = $_SERVER['REQUEST_METHOD'];
$form_data = [];

if ($method === 'GET') {
    $form_data = $_GET;
} elseif ($method === 'POST') {
    $contentType = $_SERVER['CONTENT_TYPE'] ?? '';
    if (strpos($contentType, 'application/x-www-form-urlencoded') === false) {
        header("Content-Type: text/plain");
        echo "Error: Expected urlencoded, got " . $contentType . "\n";
        echo "Use /cgi-bin/upload.php for file uploads\n";
        exit;
    }
    $form_data = $_POST;
}

$pathInfo = $_SERVER['PATH_INFO'] ?? '';
if ($pathInfo === '') {
    $uriPath = parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH);
    $scriptPath = $_SERVER['SCRIPT_NAME'];
    if (strpos($uriPath, $scriptPath) === 0) {
        $pathInfo = substr($uriPath, strlen($scriptPath));
    }
}
if ($pathInfo === '') $pathInfo = '/';

$segments = array_values(array_filter(explode('/', $pathInfo), 'strlen'));

$actionResult = '';
$action = null;

// 1️⃣ Priority 1 — PATH_INFO
if (!empty($segments)) {
    $action = $segments[0];
    if ($action === 'hello' && isset($segments[1])) {
        $name = htmlspecialchars($segments[1]);
        $actionResult = "Hello (via PATH_INFO), {$name}!";
    } elseif ($action === 'sum' && isset($segments[1], $segments[2])) {
        $a = (int)$segments[1];
        $b = (int)$segments[2];
        $actionResult = "Sum (via PATH_INFO): {$a} + {$b} = " . ($a + $b);
    } else {
        $actionResult = "Unknown PATH_INFO: " . htmlspecialchars($pathInfo);
    }

} elseif (isset($_GET['action'])) {
    $action = $_GET['action'];
    if ($action === 'hello' && isset($_GET['name'])) {
        $name = htmlspecialchars($_GET['name']);
        $actionResult = "Hello (via query), {$name}!";
    } elseif ($action === 'sum' && isset($_GET['a'], $_GET['b'])) {
        $a = (int)$_GET['a'];
        $b = (int)$_GET['b'];
        $actionResult = "Sum (via query): {$a} + {$b} = " . ($a + $b);
    } else {
        $actionResult = "Unknown query action: " . htmlspecialchars($action);
    }

} else {
    $actionResult = "Welcome to PHP test page!";
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

    <?php if ($method === 'GET' && !empty($_SERVER['QUERY_STRING'])): ?>
        <p><small>Query: <?php echo htmlspecialchars($_SERVER['QUERY_STRING']); ?></small></p>
    <?php endif; ?>

    <?php if ($pathInfo !== '/'): ?>
        <p><small>Path info: <?php echo htmlspecialchars($pathInfo); ?></small></p>
    <?php endif; ?>

    <p><strong>Action result:</strong> <?php echo $actionResult; ?></p>

    <hr>
    <h3>Test Form</h3>
    <form method="POST">
        <input name="name" placeholder="Name" required>
        <input name="email" type="email" placeholder="Email">
        <button type="submit">Submit</button>
    </form>

    <hr>
    <h3>Examples (PATH_INFO)</h3>
    <ul>
        <li><a href="/php/form.php/hello/Alex">/hello/Alex</a></li>
        <li><a href="/php/form.php/sum/5/7">/sum/5/7</a></li>
    </ul>

    <h3>Examples (Query string)</h3>
    <ul>
        <li><a href="/php/form.php?action=hello&name=Sheila">?action=hello&name=Sheila</a></li>
        <li><a href="/php/form.php?action=sum&a=10&b=9">?action=sum&a=10&b=9</a></li>
    </ul>

    <a href="/">Back</a>
</body>
</html>

