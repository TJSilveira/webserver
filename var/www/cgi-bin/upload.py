#!/usr/bin/env python3
import os
import sys
import cgi

def main():
    # Required CGI header
    print("Content-Type: text/plain")
    print()

    # Parse form fields from stdin
    form = cgi.FieldStorage()

    if "file" not in form:
        print("No file field named 'file' in form")
        return

    file_item = form["file"]

    if not file_item.file or not file_item.filename:
        print("No file uploaded")
        return

    upload_dir = "/var/www/uploads"   # adjust to your setup
    os.makedirs(upload_dir, exist_ok=True)

    dst_path = os.path.join(upload_dir, os.path.basename(file_item.filename))

    try:
        with open(dst_path, "wb") as f:
            while True:
                chunk = file_item.file.read(4096)
                if not chunk:
                    break
                f.write(chunk)
        print(f"File uploaded to {dst_path}")
    except Exception as e:
        print("Upload failed:", str(e))

if __name__ == "__main__":
    main()
