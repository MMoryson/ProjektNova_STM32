import csv
import serial
import re
import time
import matplotlib.pyplot as plt
from collections import deque
import tkinter as tk
from tkinter import messagebox

# Flaga globalna do przerwania pętli
stop_plotting = False

# Funkcja do odczytu danych z UART
def odczyt_uart(csv_plik, port='COM3', baudrate=115200, timeout=1):
    start = time.time()
    ser = serial.Serial(port, baudrate, timeout=timeout)

    with open(csv_plik, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Czas', 'Dystans', 'Enkoder'])

    last_time = -1  # Zmienna do sprawdzania unikalności czasu

    try:
        while True:
            if ser.in_waiting > 0:
                a = round(time.time() - start, 3)  # Zaokrąglanie czasu do 3 miejsc po przecinku
                if a == last_time:
                    continue  # Pomijanie duplikatów czasu
                last_time = a

                line = ser.readline().decode('utf-8', errors='ignore').strip()
                temp = re.findall(r'\d+', line)
                res = list(map(int, temp))
                if temp:
                    dystans = res[0]
                    enkoder = res[1]
                    czas = a
                    with open(csv_plik, mode='a', newline='') as file:
                        writer = csv.writer(file)
                        writer.writerow([czas, dystans, enkoder])
                    yield (czas, dystans, enkoder)
    except KeyboardInterrupt:
        print("Zatrzymanie zbierania danych.")
    finally:
        if ser.is_open:
            ser.close()
        print("Zamknięto port UART.")

# Funkcja główna do rysowania wykresu
def plot_data(csv_plik, plot_dystans, plot_enkoder):
    global stop_plotting
    uart_data = odczyt_uart(csv_plik)

    czas_list = deque(maxlen=100)
    dystans_list = deque(maxlen=100)
    enkoder_list = deque(maxlen=100)

    # Tworzenie wykresu
    fig, ax = plt.subplots(figsize=(8, 6))

    def on_close(event):
        """Callback, gdy okno wykresu jest zamykane."""
        global stop_plotting
        stop_plotting = True
        plt.close(fig)  # Zamknięcie okna matplotlib

    fig.canvas.mpl_connect('close_event', on_close)  # Rejestracja zdarzenia zamknięcia

    try:
        while not stop_plotting:
            czas, dystans, enkoder = next(uart_data)
            czas_list.append(czas)
            dystans_list.append(dystans)
            enkoder_list.append(enkoder)

            ax.clear()
            ax.grid(True)

            if plot_dystans:
                ax.plot(czas_list, dystans_list, label='Dystans', color='blue')
            if plot_enkoder:
                ax.plot(czas_list, enkoder_list, label='Enkoder', color='red')

            ax.set_xlabel('Czas [s]')
            ax.set_ylabel('Wartości')
            ax.legend()

            ax.text(0.5, 1.0, f"Dystans: {dystans_list[-1] if dystans_list else '--'} cm, Enkoder: {enkoder_list[-1] if enkoder_list else '--'}",
                    transform=ax.transAxes, fontsize=12, ha='center', va='bottom', color='blue')

            plt.pause(0.1)
    except KeyboardInterrupt:
        print("Przerwano przez użytkownika.")
    except StopIteration:
        print("Brak więcej danych do odczytu.")
    finally:
        plt.close(fig)
        print("Zamknięto wykres.")

# Funkcja do wyboru opcji przed uruchomieniem wykresu
def choose_plot_options():
    def start_plot():
        plot_dystans = var_dystans.get()
        plot_enkoder = var_enkoder.get()

        if not plot_dystans and not plot_enkoder:
            messagebox.showerror("Błąd", "Musisz wybrać przynajmniej jedną zmienną do wykresu.")
        else:
            root.destroy()  # Zamknięcie okna wyboru
            plot_data("data.csv", plot_dystans, plot_enkoder)

    root = tk.Tk()
    root.title("Wybór danych do wykresu")

    var_dystans = tk.BooleanVar(value=True)
    var_enkoder = tk.BooleanVar(value=True)

    tk.Label(root, text="Wybierz dane do wykresu:").pack()
    tk.Checkbutton(root, text="Dystans", variable=var_dystans).pack()
    tk.Checkbutton(root, text="Enkoder", variable=var_enkoder).pack()
    tk.Button(root, text="Uruchom wykres", command=start_plot).pack()

    root.mainloop()

# Uruchomienie programu
if __name__ == "__main__":
    choose_plot_options()
