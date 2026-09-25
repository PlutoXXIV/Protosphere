#!/usr/bin/env python3
"""
Wireless quiz server (PC side)
-------------------------------
Talks to the gateway ESP32 over USB serial. Broadcasts a question,
listens for true/false answers from each node (esp1..esp9), and
prints a live, labeled results table. You move to the next question
by pressing Enter.

Setup:
    pip install pyserial

Usage:
    python quiz_server.py --port /dev/ttyUSB0
    python quiz_server.py --port COM5 --window 10
    python quiz_server.py --port COM5 --questions questions.txt

If --questions is omitted, you'll be prompted to type each question
interactively.
"""

import argparse
import json
import time
import threading

import serial

ALL_NODES = [f"esp{i}" for i in range(1, 10)]  # esp1..esp9


class QuizServer:
    def __init__(self, port, baud, window):
        self.ser = serial.Serial(port, baud, timeout=1)
        self.window = window
        self.lock = threading.Lock()
        self.answers = {}  # node -> answer, for the CURRENT question only
        self.current_qid = None
        self.stop_flag = False
        self.reader_thread = threading.Thread(target=self._read_loop, daemon=True)

    def start(self):
        time.sleep(2)  # let the ESP32 finish its reset-on-serial-open
        self.reader_thread.start()

    def _read_loop(self):
        while not self.stop_flag:
            try:
                raw = self.ser.readline().decode(errors="ignore").strip()
            except Exception:
                continue
            if not raw:
                continue
            try:
                msg = json.loads(raw)
            except json.JSONDecodeError:
                continue

            if "node" in msg and "answer" in msg:
                with self.lock:
                    if msg.get("qid") == self.current_qid:
                        self.answers[msg["node"]] = msg["answer"]
            elif "status" in msg or "error" in msg:
                print(f"[gateway] {raw}")

    def ask(self, qid, text, window=None):
        window = window if window is not None else self.window
        with self.lock:
            self.current_qid = qid
            self.answers = {}

        payload = {"qid": qid, "text": text, "window": window}
        self.ser.write((json.dumps(payload) + "\n").encode())

        print(f"\nQ{qid}: {text}   (answer window: {window}s)")
        deadline = time.time() + window + 1  # small buffer for radio/serial delay
        last_shown = -1
        while time.time() < deadline:
            with self.lock:
                n = len(self.answers)
            if n != last_shown:
                self._print_table()
                last_shown = n
            time.sleep(0.25)
        self._print_table()
        print()  # newline after the final table

    def _print_table(self):
        with self.lock:
            snapshot = dict(self.answers)
        line = "  ".join(f"{node} - {snapshot.get(node, '...')}" for node in ALL_NODES)
        print("\r" + line, end="", flush=True)

    def close(self):
        self.stop_flag = True
        self.ser.close()


def load_questions(path):
    with open(path, encoding="utf-8") as f:
        return [line.strip() for line in f if line.strip()]


def main():
    parser = argparse.ArgumentParser(description="Wireless quiz server")
    parser.add_argument("--port", required=True, help="Serial port of the gateway ESP32, e.g. COM5 or /dev/ttyUSB0")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--window", type=int, default=8, help="Default answer window in seconds")
    parser.add_argument("--questions", help="Optional text file, one question per line")
    args = parser.parse_args()

    server = QuizServer(args.port, args.baud, args.window)
    server.start()

    questions = load_questions(args.questions) if args.questions else None
    qid = 0

    try:
        if questions:
            for q in questions:
                qid += 1
                server.ask(qid, q)
                input("Press Enter for the next question...")
        else:
            print("Type a question and press Enter to broadcast it. Ctrl+C to quit.")
            while True:
                text = input("\nQuestion: ").strip()
                if not text:
                    continue
                qid += 1
                server.ask(qid, text)
    except KeyboardInterrupt:
        print("\nQuiz ended.")
    finally:
        server.close()


if __name__ == "__main__":
    main()
