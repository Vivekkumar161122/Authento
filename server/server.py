import http.server
import socketserver
import json
import csv
import os
from datetime import datetime
import threading
import time

PORT = 3000
PUBLIC_DIR = os.path.join(os.path.dirname(__file__), 'public')
CSV_FILE = os.path.join(os.path.dirname(__file__), 'attendance_log.csv')

# In-memory mock database of known users
KNOWN_USERS = {
    "A1B2C3D4": "Alice Smith",
    "E5F6G7H8": "Bob Jones"
}

# Global list to keep track of SSE clients
sse_clients = []

# Ensure CSV exists
if not os.path.exists(CSV_FILE):
    with open(CSV_FILE, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Timestamp', 'UID', 'Name'])

class AttendanceHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=PUBLIC_DIR, **kwargs)

    def do_POST(self):
        if self.path == '/api/scan':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            
            try:
                data = json.loads(post_data.decode('utf-8'))
                uid = data.get('uid', 'UNKNOWN')
                name = KNOWN_USERS.get(uid, "Unknown User")
                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

                # Log to CSV
                with open(CSV_FILE, mode='a', newline='') as file:
                    writer = csv.writer(file)
                    writer.writerow([timestamp, uid, name])
                
                print(f"Scanned: {name} ({uid}) at {timestamp}")

                # Notify all SSE clients
                scan_event = json.dumps({"timestamp": timestamp, "uid": uid, "name": name})
                for client in list(sse_clients):
                    try:
                        client.wfile.write(f"data: {scan_event}\n\n".encode('utf-8'))
                        client.wfile.flush()
                    except Exception:
                        sse_clients.remove(client)

                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(b'{"status":"success"}')

            except Exception as e:
                print("Error processing POST:", e)
                self.send_response(400)
                self.end_headers()
        else:
            self.send_response(404)
            self.end_headers()

    def do_GET(self):
        if self.path == '/api/events':
            self.send_response(200)
            self.send_header('Content-type', 'text/event-stream')
            self.send_header('Cache-Control', 'no-cache')
            self.send_header('Connection', 'keep-alive')
            self.end_headers()
            
            sse_clients.append(self)
            
            # Send initial history
            try:
                history = []
                with open(CSV_FILE, mode='r') as file:
                    reader = csv.DictReader(file)
                    for row in reader:
                        history.append(row)
                
                # Send the history as a custom event or just a normal data block
                init_data = json.dumps({"history": history})
                self.wfile.write(f"event: init\ndata: {init_data}\n\n".encode('utf-8'))
                self.wfile.flush()
                
                # Keep connection alive
                while True:
                    time.sleep(1)
            except Exception as e:
                if self in sse_clients:
                    sse_clients.remove(self)
        else:
            # Serve files from PUBLIC_DIR
            super().do_GET()

class ThreadedHTTPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    daemon_threads = True
    allow_reuse_address = True

if __name__ == '__main__':
    if not os.path.exists(PUBLIC_DIR):
        os.makedirs(PUBLIC_DIR)
        
    print(f"Starting server at http://localhost:{PORT}")
    print(f"Make sure to update your Arduino code with your laptop's IP address!")
    with ThreadedHTTPServer(("", PORT), AttendanceHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server.")
            pass
