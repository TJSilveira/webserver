#!/usr/bin/env python3
import os
import sys

def main():
    # 1. Get the upload directory from the environment variable set by your C++ server
    upload_dir = os.environ.get("UPLOAD_DIR", "./var/www/uploads")
    request_method = os.environ.get('REQUEST_METHOD', '')
    server_protocol = os.environ.get('SERVER_PROTOCOL', 'HTTP/1.1')

    #2. Validate request method
    if request_method != 'GET':
        error_msg = "Error: Only GET method is allowed"
        print(f"{server_protocol} 405 Method Not Allowed")
        print("Content-Type: text/plain")
        print(f"Content-Length: {len(error_msg)}")
        print()
        print(error_msg)
        sys.stdout.flush()
        return

    # 3. Build the file list HTML
    file_list_html = ""
    try:
        if os.path.exists(upload_dir) and os.path.isdir(upload_dir):
            files = [f for f in os.listdir(upload_dir) if os.path.isfile(os.path.join(upload_dir, f))]
            for filename in files:
                file_list_html += f"""
                <li id="file-{filename}">
                    {filename} 
                    <button onclick="deleteFile('{filename}')">Delete</button>
                </li>"""
        else:
            file_list_html = "<li>Directory not found or inaccessible.</li>"
    except Exception as e:
        file_list_html = f"<li>Error listing files: {str(e)}</li>"

    # 4. Output the full HTML page
    body = f"""<!DOCTYPE html>
    <html>
    <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>About Webserv</title>
        <style>
            body {{
                font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
                line-height: 1.6;
                color: #333;
                max-width: 800px;
                margin: 0 auto;
                padding: 20px;
                background-color: #f9f9f9;
            }}
            header {{
                border-bottom: 2px solid #e1e1e1;
                padding-bottom: 20px;
                margin-bottom: 30px;
            }}
            /* Navbar matching index.html style if implied, or keeping generic clean style */
            nav {{
                background: white;
                padding: 15px;
                border-radius: 4px;
                box-shadow: 0 1px 3px rgba(0,0,0,0.1);
                margin-bottom: 30px;
                width: 100%;
                max-width: 800px; /* Match the body width */
                display: flex;
                justify-content: space-between;
                align-items: center;
                box-sizing: border-box; 
            }}
            nav a {{
                text-decoration: none;
                color: #555;
                margin: 0 10px;
                font-weight: 500;
            }}
            nav a:hover {{
                color: #2c3e50;
            }}
            h1 {{
                color: #2c3e50;
                margin: 0;
                font-size: 2.2rem;
            }}
            .subtitle {{
                color: #7f8c8d;
                font-size: 1.1rem;
                margin-top: 5px;
            }}
            section {{
                background: white;
                padding: 25px;
                margin-bottom: 20px;
                border-radius: 4px;
                box-shadow: 0 1px 3px rgba(0,0,0,0.05);
            }}
            h2 {{
                color: #2c3e50;
                border-bottom: 1px solid #eee;
                padding-bottom: 10px;
                margin-top: 0;
            }}
            ul {{
                padding-left: 20px;
            }}
            li {{
                margin-bottom: 8px;
            }}
            footer {{
                text-align: center;
                font-size: 0.9rem;
                color: #777;
                margin-top: 40px;
                border-top: 1px solid #e1e1e1;
                padding-top: 20px;
            }}
            .tag {{
                display: inline-block;
                background: #eee;
                padding: 2px 8px;
                border-radius: 3px;
                font-size: 0.85rem;
                font-family: monospace;
                color: #555;
            }}
        </style>
    </head>
    <body>
        <nav>
            <div style="font-weight: bold; font-size: 1.2rem;">webserv 42</div>
            <div>
                <a href="/">Home</a>
                <a href="/about/">About</a>
                <a href="/upload/">Upload</a>
                <a href="/files/"style="color: #2c3e50; font-weight: bold;">Files</a>
                <a href="/errors/">Errors</a>
            </div>
        </nav>

    <body>
        <h1>Uploaded Files</h1>
        <ul>
            {file_list_html}
        </ul>

        <script>
            function deleteFile(filename) {{
                if (!confirm('Are you sure you want to delete ' + filename + '?')) return;

                // This sends a DELETE request directly to your C++ server's endpoint
                fetch('/delete/' + filename, {{ 
                    method: 'DELETE' 
                }})
                .then(response => {{
                    if (response.ok || response.status === 204) {{
                        // Remove the element from the list upon success
                        const element = document.getElementById('file-' + filename);
                        if (element) element.remove();
                    }} else {{
                        alert('Failed to delete file. Status: ' + response.status);
                    }}
                }})
                .catch(error => {{
                    console.error('Error:', error);
                    alert('An error occurred while deleting the file.');
                }});
            }}
        </script>
    </body>
    </html>
    """

    # 5. Add Content-Length to the headers
    headers = "Content-Type: text/html\r\n"
    headers += f"Content-Length: {len(body)}\r\n"


    # 5. Print the full response
    print(f"{server_protocol} 200 OK")
    print(f"{headers}")
    print("\r\n")
    print(f"{body}")
    sys.stdout.flush()

if __name__ == "__main__":
    main()