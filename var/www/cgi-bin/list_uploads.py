#!/usr/bin/env python3
import os
import sys

# Configuration: Path to your server's upload directory
# Make sure this matches your 'upload_store' directive in the config
upload_dir = os.environ.get('UPLOAD_DIR','')

def generate_html():
    print("Content-Type: text/html\r\n\r\n")
    print("""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>Uploaded Files Management</title>
        <style>
            body { font-family: sans-serif; padding: 20px; max-width: 800px; margin: 0 auto; }
            table { border-collapse: collapse; width: 100%; margin-top: 20px; }
            th, td { text-align: left; padding: 10px; border-bottom: 1px solid #ddd; }
            th { background-color: #f4f4f4; }
            tr:hover { background-color: #f9f9f9; }
            .delete-btn { 
                background-color: #ff4d4d; color: white; border: none; 
                padding: 5px 10px; cursor: pointer; border-radius: 3px; 
            }
            .delete-btn:hover { background-color: #cc0000; }
            .header { display: flex; justify-content: space-between; align-items: center; }
        </style>
    </head>
    <body>
        <div class="header">
            <h1>Uploaded Files</h1>
            <a href="/">Back to Home</a>
        </div>
        <table>
            <thead>
                <tr>
                    <th>Filename</th>
                    <th>Size (Bytes)</th>
                    <th>Action</th>
                </tr>
            </thead>
            <tbody>
    """)

    try:
        # Check if directory exists
        if not os.path.exists(upload_dir) or not os.path.isdir(upload_dir):
            print(f"<tr><td colspan='3'>Upload directory '{upload_dir}' not found.</td></tr>")
        else:
            files = os.listdir(upload_dir)
            if not files:
                print("<tr><td colspan='3'>No files uploaded yet.</td></tr>")
            else:
                for filename in sorted(files):
                    # Skip hidden files or navigation dots
                    if filename.startswith('.'):
                        continue
                    
                    filepath = os.path.join(upload_dir, filename)
                    try:
                        size = os.path.getsize(filepath)
                    except OSError:
                        size = "Unknown"

                    # The Delete form: sends a POST request to delete_file.py
                    # with the filename passed as a form parameter
                    print(f"""
                    <tr>
                        <td>{filename}</td>
                        <td>{size}</td>
                        <td>
                            <form action="/cgi-bin/delete_file.py" method="POST" style="margin:0;">
                                <input type="hidden" name="filename" value="{filename}">
                                <button type="submit" class="delete-btn" onclick="return confirm('Are you sure you want to delete {filename}?');">
                                    Delete
                                </button>
                            </form>
                        </td>
                    </tr>
                    """)
    except Exception as e:
        print(f"<tr><td colspan='3'>Error reading directory: {e}</td></tr>")

    print("""
            </tbody>
        </table>
    </body>
    </html>
    """)

if __name__ == "__main__":
    generate_html()
