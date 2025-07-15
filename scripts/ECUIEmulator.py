import socket
import threading
import json


HOST = '127.0.0.1'  # localhost
PORT = 8080        # Port to listen on

# Store connected clients
clients = []
MAX_MSG_LENGTH = 65536-1

# Function to handle client connections
def handle_client(conn, addr):
    print(f"Connected by {addr}")
    clients.append(conn)
    try:
        while True:
            data = conn.recv(1024)
            if not data:
                break
            print(f"Received from client: {data}")
    except Exception as e:
        print(f"Connection error: {e}")
    finally:
        conn.close()
        clients.remove(conn)
        print(f"Connection with {addr} closed.")

def start_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen(5)
    print(f"Server listening on {HOST}:{PORT}")

    while True:
        conn, addr = server.accept()
        threading.Thread(target=handle_client, args=(conn, addr),daemon=True).start()

# Function to send JSON messages to all clients, with a custom header
def send_message_to_all_clients(type,message):
    if type == "sequence-start":
        print("Starting sequence with message: %s", message)
        msg = {
            "type": type,
            "content": [message,"",""]
        }
    else:
        msg = {
            "type": type,
            "content": message
        }

    str_msg = json.dumps(msg) + '\n'
    str_msg_len = len(str_msg)

    if str_msg_len <= MAX_MSG_LENGTH:
        lsb = str_msg_len & 0x00FF
        msb = str_msg_len >> 8

        header = bytes([msb, lsb])
        msg_buffer = header + str_msg.encode('ascii')

        for client in clients:
            try:
                client.sendall(msg_buffer)
            except Exception as e:
                print(f"Error sending message to a client: {e}")

if __name__ == "__main__":
    print("Starting JSON socket server.")
    threading.Thread(target=start_server, daemon=True).start()
    while True:
        try:
            print("Enter your message type(one of sequence-start, send-postseq-comment, abort, auto-abort-change, states-load, states-get, states-set, states-start, states-stop, gui-mapping-load, commands-load, commands-set):")
            type = input()
            print("Enter your JSON message (send with `END` in a new line):")
            lines = []
            while True:
                line = input()
                if line.strip() == "END":
                    break
                lines.append(line)

            # Join the lines and parse as JSON
            json_data = json.loads("\n".join(lines))

            # Send the JSON data to all connected clients
            send_message_to_all_clients(type,json_data)
            print("Message sent.")
        except json.JSONDecodeError:
            print("Invalid JSON. Please try again.")
        except KeyboardInterrupt:
            print("Server shutting down.")
            break
        except Exception as e:
            print(f"Unexpected error: {e}")
