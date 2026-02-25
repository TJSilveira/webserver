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
        <title>File Manager</title>
        <style>
            li {{ margin-bottom: 10px; list-style: none; }}
            button {{ margin-left: 10px; color: red; cursor: pointer; }}
        </style>
    </head>
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