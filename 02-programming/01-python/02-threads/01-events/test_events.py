"""
Simple test for the multithreading events concept.
It releases necessity to tick wait for values, as we can work with the event appearance and one or more queues.
Polling of the queue is not necessary - saves a bit of cpu power.
"""

from threading import Thread, Event
import time

def run(ev):
    if isinstance(ev, Event):
        ev.wait()
        ev.clear()
        print(f"Go Go Go: {ev.is_set()}")

def main():
    e = Event()
    e.clear()

    th = Thread(target=run, args=(e,))
    th.start()

    for i in range(10):
        print(f"{i}")
        time.sleep(0.5)
    e.set()
    th.join()
    print(th.is_alive(), th.ident)

if __name__ == '__main__':
    main()
