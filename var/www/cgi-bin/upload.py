#!/usr/bin/env python3

import os
import sys
import cgi

def main():
    # Get CGI environment variables
    request_method = os.environ.get('REQUEST_METHOD', '')
    server_protocol = os.environ.get('SERVER_PROTOCOL', 'HTTP/1.1')
    
    # Validate request method
    if request_method != 'POST':
        error_msg = "Error: Only POST method is allowed"
        print(f"{server_protocol} 405 Method Not Allowed")
        print("Content-Type: text/plain")
        print(f"Content-Length: {len(error_msg)}")
        print()
        print(error_msg)
        sys.stdout.flush()
        return
    
    # Define upload directory
    upload_dir = "./var/www/uploads"
    
    try:
        os.makedirs(upload_dir, exist_ok=True)
    except Exception as e:
        error_msg = f"Error creating upload directory: {str(e)}"
        print(f"{server_protocol} 500 Internal Server Error")
        print("Content-Type: text/plain")
        print(f"Content-Length: {len(error_msg)}")
        print()
        print(error_msg)
        sys.stdout.flush()
        return
    
    # Parse POST data
    try:
        form = cgi.FieldStorage()
    except Exception as e:
        error_msg = f"Error parsing form data: {str(e)}"
        print(f"{server_protocol} 400 Bad Request")
        print("Content-Type: text/plain")
        print(f"Content-Length: {len(error_msg)}")
        print()
        print(error_msg)
        sys.stdout.flush()
        return
    
    # Find the first file field (accept any field name)
    fileitem = None
    field_name = None
    
    for key in form.keys():
        item = form[key]
        if hasattr(item, 'file') and item.file:
            fileitem = item
            field_name = key
            break
    
    if fileitem is None:
        error_msg = "Error: No file uploaded in request"
        print(f"{server_protocol} 400 Bad Request")
        print("Content-Type: text/plain")
        print(f"Content-Length: {len(error_msg)}")
        print()
        print(error_msg)
        sys.stdout.flush()
        return
    
    # Get filename
    if fileitem.filename:
        filename = os.path.basename(fileitem.filename)
    else:
        import time
        filename = f"upload_{int(time.time())}.dat"
    
    filepath = os.path.join(upload_dir, filename)
    
    # Write the file
    try:
        file_size = 0
        with open(filepath, 'wb') as f:
            while True:
                chunk = fileitem.file.read(8192)
                if not chunk:
                    break
                f.write(chunk)
                file_size += len(chunk)
        
        # Success response
        response_body = (
            f"<html><body>\n"
            f"<h1>Upload Successful</h1>\n"
            f"<p>File saved to: {filepath}</p>\n"
            f"<p>Filename: {filename}</p>\n"
            f"<p>Field name: {field_name}</p>\n"
            f"<p>Size: {file_size} bytes</p>\n"
            f"</body></html>\n"
        )
        
        print(f"{server_protocol} 200 OK")
        print("Content-Type: text/html")
        print(f"Content-Length: {len(response_body)}")
        print()
        print(response_body, end='')
        sys.stdout.flush()
        
    except Exception as e:
        error_msg = f"Error writing file: {str(e)}"
        print(f"{server_protocol} 500 Internal Server Error")
        print("Content-Type: text/plain")
        print(f"Content-Length: {len(error_msg)}")
        print()
        print(error_msg)
        sys.stdout.flush()
        return

if __name__ == "__main__":
    main()
