#pragma once

const char* INDEX_HTML = R"(
<!DOCTYPE html>
<html>
<head>
    <title>workin'</title>
</head>
<body>
    <h1>workin'</h1>
    <canvas id="canvas" width="800" height="600"></canvas>
    <script>
        const ws = new WebServer("http://localhost:9000");
        const cv = document.getElementById('canvas');
    </script>
</body>
</html>
)";
