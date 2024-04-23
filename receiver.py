import argparse
import time

import matplotlib.animation as animation
import matplotlib.pyplot as plt
import serial

# def read_from_arduino(arduino):
#     data = arduino.readline()
#     if data:
#         line = data.decode("utf-8")
#
#         print(line)
#         if "=" in line:
#             temp = float(line[15:20])
#             return time.time(), temp
#         elif "Fault" in line:
#             print("Fault")
#
#     return None
# def read_save(arduino, save_path):
#     a = read_from_arduino(arduino)
#     if a is None:
#         return None, None
#     t, temp = a
#     with open(save_path, "a") as f:
#         f.write(f"{t},{temp}\n")
#     return t, temp


def read_from_arduino(arduino):
    line = arduino.readline().decode("utf-8")
    if line:
        print(line)
        if "Data:" in line:
            elements = line.split(",")
            temp = float(elements[1])
            setpoint = float(elements[2])
            heater_percent = float(elements[3])
            heater_PWM = float(elements[4])
            return time.time(), temp, setpoint, heater_percent, heater_PWM
        elif "Fault" in line:
            print("Fault")
    return None


def read_save(arduino, save_path):
    a = read_from_arduino(arduino)
    if a is None:
        return None, None, None, None, None
    t, temp, setpoint, heater_percent, heater_PWM = a
    with open(save_path, "a") as f:
        f.write(f"{t},{temp},{setpoint},{heater_percent},{heater_PWM}\n")
    return t, temp, setpoint, heater_percent, heater_PWM


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default="COM6", help="Arduino port")
    parser.add_argument("--baud", default=115200, help="Arduino baud rate")
    parser.add_argument(
        "--save-path",
        default="data.csv",
        help="Path to save data",
    )
    parser.add_argument("--live-plot", default=True, help="Toggle live plot")
    parser.add_argument(
        "--live-plot-n",
        default=0,
        help="Max number of points to plot",
    )
    args = parser.parse_args()
    port = args.port
    baud = args.baud
    save_path = args.save_path
    live_plot = args.live_plot
    live_plot_n = args.live_plot_n

    date = time.strftime("%Y-%m-%d_%H-%M-%S")
    save_path = f"{date}_{save_path}"
    print(f"Saving data to {save_path}")

    arduino = serial.Serial(port, baud, timeout=0.01)

    print(f"live_plot: {live_plot}")
    if live_plot == True:
        print("Creating figure")
        fig, axs = plt.subplots(nrows=2)
        ts = []
        temps = []
        sets = []
        heaters = []
        percents = []

        def animate(i, ts, temps, sets, heaters):
            t, temp, setpoint, heater_percent, heater_PWM = read_save(
                arduino, save_path
            )
            if t is None:
                return
            ts.append(t)
            temps.append(temp)
            sets.append(setpoint)
            heaters.append(heater_PWM)
            percents.append(heater_percent)
            if live_plot_n > 0:
                ts = ts[-live_plot_n:]
                temps = temps[-live_plot_n:]
                sets = sets[-live_plot_n:]
                heaters = heaters[-live_plot_n:]
            axs[0].clear()
            axs[0].plot(ts, temps, color="k")
            axs[0].plot(ts, sets, color="red")
            axs[1].clear()
            axs[1].plot(ts, heaters, color="k")
            axs[1].plot(ts, percents, color="r")
            arduino.flush()

        ani = animation.FuncAnimation(
            fig,
            animate,
            fargs=(ts, temps, sets, heaters),
            interval=40,
        )
        plt.show()
        print("showing figure")
    else:
        while True:
            t, temp, setpoint, heater_percent, heater_PWM = read_save(
                arduino, save_path
            )
            if t is None:
                continue
            print(t, temp)
