#!/usr/bin/env python3

# 2. The Static Body
html_content = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Static Python CGI</title>
    <style>
        body { font-family: sans-serif; text-align: center; padding: 50px; background-color: #f4f4f9; }
        h1 { color: #2c3e50; }
        p { color: #555; }
    </style>
</head>
<body>
    <h1>Hello from a Static Python Script!</h1>
    <p>This page content is hardcoded and will not change.</p>
</body>
</html>
"""

print(html_content)