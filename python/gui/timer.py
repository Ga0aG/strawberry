import argparse
import pandas as pd
import threading
import time
import tkinter as tk
from collections import defaultdict
from tkinter import messagebox, simpledialog
from tkinter import ttk
from typing import Optional

class TimerApp:
    def __init__(self, root: tk.Tk, args):
        self.root = root
        self.root.title("计时器")
        self.args = args

        # Timer variables
        self.is_running = False
        self.is_paused = False
        self.start_time = 0
        self.task_name = ""

        self.records = pd.DataFrame(columns=["start_time", "end_time", "task_name"])

        # Load existing records
        self.load_records()
        self.elapsed_time = 0 # today elapsed_time
        self.today_time_string = time.strftime('%Y-%m-%d', time.localtime(time.time()))
        time_match = False
        for _, row in self.records.iterrows():
            if not time_match and time.strftime('%Y-%m-%d', time.localtime(row['start_time'])) == self.today_time_string:
                time_match = True
            if time_match:
                self.elapsed_time += (row["end_time"] - row["start_time"])

        # Create a notebook (tabbed interface)
        self.notebook = ttk.Notebook(root)
        self.notebook.pack(pady=10, expand=True, fill='both')

        # Create frames for each tab
        self.timer_frame = ttk.Frame(self.notebook)
        self.records_frame = ttk.Frame(self.notebook)
        self.statistics_frame = ttk.Frame(self.notebook)
        self.daily_frame = ttk.Frame(self.notebook)

        # Add tabs to the notebook
        self.notebook.add(self.timer_frame, text='计时器')
        self.notebook.add(self.records_frame, text='记录')
        self.notebook.add(self.statistics_frame, text='类型统计')
        self.notebook.add(self.daily_frame, text='日期统计')

        # Timer GUI Layout
        self.time_label = tk.Label(self.timer_frame, text=self.format_time(self.elapsed_time), font=("Helvetica", 48))
        self.time_label.pack(pady=20)

        self.task_label = tk.Label(self.timer_frame, text="当前任务:", anchor="w", justify="left")
        self.task_label.pack(pady=20)

        self.start_button = tk.Button(self.timer_frame, text="开始计时", command=self.start_timer)
        self.start_button.pack(pady=5)

        self.pause_button = tk.Button(self.timer_frame, text="暂停计时", command=self.pause_timer, state=tk.DISABLED)
        self.pause_button.pack(pady=5)

        self.stop_button = tk.Button(self.timer_frame, text="停止计时", command=self.stop_timer, state=tk.DISABLED)
        self.stop_button.pack(pady=5)

        # Records display
        self.record_text = tk.Text(self.records_frame, width=60, height=22, state=tk.DISABLED)
        self.record_text.pack(pady=5)

        # Statistics display
        self.statistics_text = tk.Text(self.statistics_frame, width=60, height=22, state=tk.DISABLED)
        self.statistics_text.pack(pady=5)

        # Daily statistics display
        self.daily_text = tk.Text(self.daily_frame, width=60, height=22, state=tk.DISABLED)
        self.daily_text.pack(pady=5)

        # self.quit_button = tk.Button(self.timer_frame, text="退出", command=self.quit_app)
        # self.quit_button.pack(pady=20)

        self.record_per_type = defaultdict(int)
        self.record_per_day = defaultdict(int)
        self.update_records_display(reset=True)

    def load_records(self):
        try:
            self.records = pd.read_csv(self.args.fileName)
        except:
            self.records = pd.DataFrame(columns=["start_time", "end_time", "task_name"])

    def start_timer(self):
        if not self.is_running:
            self.task_name = simpledialog.askstring("输入任务类型", "请输入任务类型:")
            if not self.task_name:
                messagebox.showinfo("提示", "任务类型不能为空！")
                return
            self.task_label.config(text=f"当前任务: {self.task_name}")
            time_string = time.strftime('%Y-%m-%d', time.localtime(time.time()))
            if time_string != self.today_time_string:
                self.today_time_string = time_string
                self.elapsed_time = 0
            self.is_running = True
            self.is_paused = False
            self.start_time = time.time()
            self.start_button.config(state=tk.DISABLED)
            self.pause_button.config(state=tk.NORMAL)
            self.stop_button.config(state=tk.NORMAL)

            # Start a thread to avoid blocking the GUI
            threading.Thread(target=self.run_timer).start()

    def run_timer(self):
        while self.is_running:
            if not self.is_paused:
                duration = time.time() - self.start_time
                elapsed_time = duration + self.elapsed_time
                self.time_label.config(text=self.format_time(elapsed_time))
                self.task_label.config(text=f"当前任务: {self.task_name}, 用时: {self.format_time(duration, cn=True)}")
            time.sleep(1)

    def format_time(self, elapsed, cn: bool = False, dayHour: Optional[int] = None):
        days, remainder = divmod(int(elapsed), 3600*dayHour) if dayHour is not None else (0, int(elapsed))
        hours, remainder = divmod(remainder, 3600)
        minutes, seconds = divmod(remainder, 60)
        if cn:
            return f"{int(days)}天 {int(hours)} 小时 {int(minutes)}分钟 {int(seconds)}秒" if dayHour is not None else f"{int(hours)}小时 {int(minutes)}分钟 {int(seconds)}秒"
        else:
            return f"{hours:02}:{minutes:02}:{seconds:02}"

    def pause_timer(self):
        if self.is_running and not self.is_paused:
            self.elapsed_time += time.time() - self.start_time
            self.save_record()
            self.is_paused = True
            self.pause_button.config(text="恢复计时")
            self.update_records_display()
        else:
            self.start_time = time.time()
            task_name = simpledialog.askstring("输入任务类型", "请输入任务类型:")
            self.task_name = task_name if task_name else self.task_name
            self.task_label.config(text=f"当前任务: {self.task_name}")
            self.is_paused = False
            self.pause_button.config(text="暂停计时")

    def stop_timer(self):
        if self.is_running:
            if not self.is_paused:
                self.save_record()
            self.update_records_display()

            self.is_running = False
            self.start_button.config(state=tk.NORMAL)
            self.pause_button.config(state=tk.DISABLED)
            self.stop_button.config(state=tk.DISABLED)
            self.task_name = ""
            self.task_label.config(text=f"当前任务:")
            time_string = time.strftime('%Y-%m-%d', time.localtime(time.time()))
            if time_string != self.today_time_string:
                self.time_label.config(text="00:00:00")

    def save_record(self):
        end_time = time.time()
        self.records.loc[len(self.records)] = [self.start_time, end_time, self.task_name]
        self.records.iloc[[-1]].to_csv(self.args.fileName, index_label=False, header=False, mode='a')

    def update_records_display(self, reset: bool = False):
        self.record_text.config(state=tk.NORMAL)
        if reset:
            self.record_text.delete(1.0, tk.END)
            for index, row in self.records.iterrows():
                self.record_text.insert(tk.END if index==0 else 1.0, f"{time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(row['start_time']))}: {row['task_name']}, {self.format_time(row['end_time']-row['start_time'], cn=True)}\n")
        else:
            row = self.records.iloc[-1]
            self.record_text.insert(tk.END if self.records.size==1 else 1.0, f"{time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(row['start_time']))}: {row['task_name']}, {self.format_time(row['end_time']-row['start_time'], cn=True)}\n")
        self.record_text.config(state=tk.DISABLED)
        self.update_statistics(reset)

    def update_statistics(self, reset: bool = False):
        self.statistics_text.config(state=tk.NORMAL)
        # Clear the statistics text area
        self.statistics_text.delete(1.0, tk.END)
        if reset:
            # Calculate and display statistics
            self.record_per_type = defaultdict(int)
            self.record_per_day = defaultdict(int)
            for _, row in self.records.iterrows():
                self.record_per_type[row['task_name']] += row['end_time'] - row['start_time']
                dayString = time.strftime('%Y-%m-%d', time.localtime(row['start_time']))
                self.record_per_day[dayString] += row['end_time'] - row['start_time']
        else:
            row = self.records.iloc[-1]
            self.record_per_type[row['task_name']] += row['end_time'] - row['start_time']
            dayString = time.strftime('%Y-%m-%d', time.localtime(row['start_time']))
            self.record_per_day[dayString] += row['end_time'] - row['start_time']

        sorted_stats = sorted(self.record_per_type.items(), key=lambda x: x[1], reverse=True)
        self.statistics_text.insert(tk.END, f"In total({self.args.dayHour}h): {self.format_time(sum(self.record_per_type.values()), cn=True, dayHour=self.args.dayHour)}\n")

        for task, duration in sorted_stats:
            self.statistics_text.insert(tk.END, f"{task}: {self.format_time(duration, cn=True)}\n")

        self.statistics_text.config(state=tk.DISABLED)

        # Update daily statics
        self.daily_text.config(state=tk.NORMAL)
        self.daily_text.delete(1.0, tk.END)
        for dayString, duration in self.record_per_day.items():
            self.daily_text.insert(1.0, f"{dayString}: {self.format_time(duration, cn=True)}\n")
        self.daily_text.config(state=tk.DISABLED)

    def quit_app(self):
        if self.is_running:
            if not self.is_paused:
                self.save_record()
        else:
            self.root.quit()

if __name__ == "__main__":
    root = tk.Tk()
    parser = argparse.ArgumentParser()
    parser.add_argument("--fileName", default="records.csv")
    parser.add_argument("--dayHour", default=8)
    args = parser.parse_args()
    app = TimerApp(root, args)
    root.protocol("WM_DELETE_WINDOW", app.quit_app)
    root.mainloop()