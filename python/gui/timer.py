import argparse
import pandas as pd
import json
import threading
import time
import tkinter as tk
from collections import defaultdict
from datetime import datetime
from tkinter import messagebox, simpledialog
from tkinter import ttk
from typing import Optional

# Usage: python3 timer.py --fileName /home/ga/Documents/daily/202510.csv --targetFileName /home/ga/Documents/daily/202510-target.csv targetTodayFileName /home/ga/Documents/daily/home/ga/Documents/daily/today_tasks.json [--resetToday]

TODAY_TASK = "today_tasks.json"

class TimerApp:
    def __init__(self, root: tk.Tk, args):
        self.root = root
        self.root.title("计时器")
        self.args = args

        # Timer variables
        self.is_running = False
        self.start_time = 0
        self.task_name = ""

        self.records = pd.DataFrame(columns=["start_time", "end_time", "task_name"])
        self.targetInfo = pd.DataFrame(columns=["task_name", "estimate_time(day)", "done"])

        # 今日任务数据结构
        self.today = datetime.now()
        self.today_tasks = []  # 每个元素是字典: {"task_name": "", "target_hours": 0, "completed_seconds": 0}
        self.today_total_worked = 0  # 今日已工作总时长(秒)

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

        # Timer GUI Layout - 改为显示今日剩余时间
        self.countdown_time_label = tk.Label(self.timer_frame, text=self.format_time(self.today_countdown()), font=("Helvetica", 48))
        self.countdown_time_label.pack(pady=20)

        self.task_label = tk.Label(self.timer_frame, text="当前任务:", anchor="w", justify="left")
        self.task_label.pack(pady=20)

        # 任务列表框架
        self.tasks_frame = ttk.Frame(self.timer_frame)
        self.tasks_frame.pack(pady=10, fill='both', expand=True)

        # 创建滚动条
        scrollbar = ttk.Scrollbar(self.tasks_frame)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        # 创建任务列表的Canvas和Frame
        self.tasks_canvas = tk.Canvas(self.tasks_frame, yscrollcommand=scrollbar.set, height=100)
        self.tasks_canvas.pack(side=tk.LEFT, fill='both', expand=True)
        scrollbar.config(command=self.tasks_canvas.yview)

        self.tasks_inner_frame = ttk.Frame(self.tasks_canvas)
        self.tasks_canvas.create_window((0, 0), window=self.tasks_inner_frame, anchor="nw")

        # 绑定事件以更新滚动区域
        self.tasks_inner_frame.bind("<Configure>", lambda e: self.tasks_canvas.configure(scrollregion=self.tasks_canvas.bbox("all")))

        # 存储任务行组件
        self.task_rows = []

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

        self.pause_button = tk.Button(self.timer_frame, text="暂停计时", command=self.pause_timer, state=tk.DISABLED)
        self.pause_button.pack(pady=5)

        # 底部下拉选框和添加按钮
        self.bottom_frame = ttk.Frame(self.timer_frame)
        self.bottom_frame.pack(pady=10, fill=tk.X)

        self.task_var = tk.StringVar()
        self.task_combo = ttk.Combobox(self.bottom_frame, textvariable=self.task_var)
        self.task_combo.pack(side=tk.LEFT, padx=5)

        self.target_time_var = tk.StringVar()
        self.target_time_entry = tk.Entry(self.bottom_frame, textvariable=self.target_time_var, width=10)
        self.target_time_entry.pack(side=tk.LEFT, padx=5)
        tk.Label(self.bottom_frame, text="小时").pack(side=tk.LEFT)

        self.add_task_button = tk.Button(self.bottom_frame, text="添加任务", command=self.add_today_task)
        self.add_task_button.pack(side=tk.LEFT, padx=5)

        # 更新下拉框选项
        self.update_task_combo()

        # Records display
        self.record_text = tk.Text(self.records_frame, width=60, height=22, state=tk.DISABLED)
        self.record_text.pack(pady=5)

        # Statistics display
        self.statistics_text = tk.Text(self.statistics_frame, width=60, height=22, state=tk.DISABLED)
        self.statistics_text.pack(pady=5)

        # Daily statistics display
        self.daily_text = tk.Text(self.daily_frame, width=60, height=22, state=tk.DISABLED)
        self.daily_text.pack(pady=5)

        self.record_per_type = defaultdict(int)
        self.record_per_day = defaultdict(int)
        self.update_records_display(reset=True)

        # 初始显示今日任务
        self.update_today_tasks_display()
        threading.Thread(target=self.run_timer).start()

    def today_countdown(self):
        now = datetime.now()
        end_of_day = datetime(self.today.year, self.today.month, self.today.day, 23, 59, 59)
        remaining_time = end_of_day - now
        remaining_seconds = remaining_time.total_seconds()
        return remaining_seconds

    def update_task_combo(self):
        """更新下拉框的任务选项"""
        # 从targetInfo和已有记录中获取所有任务名称
        target_tasks = self.targetInfo["task_name"].unique().tolist()
        record_tasks = self.records["task_name"].unique().tolist()
        all_tasks = list(set(target_tasks + record_tasks))
        self.task_combo['values'] = all_tasks
        if all_tasks:
            self.task_combo.set(all_tasks[0])

    def add_today_task(self):
        """添加今日任务"""
        task_name = self.task_var.get()
        target_time = self.target_time_var.get()

        if not task_name:
            messagebox.showwarning("警告", "请输入任务名称!")
            return

        if not target_time or not target_time.replace('.', '').isdigit():
            messagebox.showwarning("警告", "请输入有效的目标时长!")
            return

        # 检查任务是否已存在
        for task in self.today_tasks:
            if task['task_name'] == task_name:
                messagebox.showwarning("警告", "该任务已存在!")
                return

        # 添加新任务
        target_hours = float(target_time)
        self.today_tasks.append({
            'task_name': task_name,
            'target_hours': target_hours,
            'completed_seconds': 0
        })

        # 更新任务显示
        self.update_today_tasks_display()

        # 清空输入
        self.target_time_var.set("")
        with open(args.targetTodayFileName, "w") as file:
            file.write(json.dumps(self.today_tasks))

    def update_today_tasks_display(self):
        """更新今日任务显示"""
        # 清除现有任务行
        for widget in self.tasks_inner_frame.winfo_children():
            widget.destroy()
        self.task_rows = []

        # 计算今日已工作总时长
        self.today_total_worked = 0
        for task in self.today_tasks:
            self.today_total_worked += task['completed_seconds']

        # 添加每个今日任务
        for task in sorted(self.today_tasks, key=lambda x: x['completed_seconds'] - x['target_hours']*3600):
            self.add_task_row(task['task_name'], task['target_hours'], task['completed_seconds'])

        # 更新剩余时间显示
        self.countdown_time_label.config(text=self.format_time(self.today_countdown()))

        # 更新canvas的滚动区域
        self.tasks_inner_frame.update_idletasks()
        self.tasks_canvas.configure(scrollregion=self.tasks_canvas.bbox("all"))

    def add_task_row(self, task_name, target_hours, completed_seconds):
        """添加一个任务行"""
        row_frame = ttk.Frame(self.tasks_inner_frame)
        row_frame.pack(fill=tk.X, pady=2)

        # 任务名称
        name_label = tk.Label(row_frame, text=task_name, width=20, anchor="w")
        name_label.pack(side=tk.LEFT, padx=5)

        # 今日工作时长/目标时长
        time_text = f"{self.format_time(completed_seconds)} / {target_hours:.2f}小时"
        countdown_time_label = tk.Label(row_frame, text=time_text, width=30, anchor="w")
        countdown_time_label.pack(side=tk.LEFT, padx=5)

        # 播放按钮
        play_button = tk.Button(
            row_frame,
            text="开始任务",
            width=8,
            command=lambda t=task_name: self.start_specific_task(t),
            state=tk.NORMAL if not self.is_running else tk.DISABLED
        )
        play_button.pack(side=tk.RIGHT, padx=5)

        # 存储行组件用于更新
        self.task_rows.append({
            'frame': row_frame,
            'name_label': name_label,
            'countdown_time_label': countdown_time_label,
            'play_button': play_button,
            'task_name': task_name
        })

    def start_specific_task(self, task_name):
        """开始特定的任务"""
        if not self.is_running:
            self.task_name = task_name
            self.task_label.config(text=f"当前任务: {self.task_name}")
            # time_string = time.strftime('%Y-%m-%d', time.localtime(time.time()))
            # if time_string != self.today_time_string:
            #     self.today_time_string = time_string
            #     self.elapsed_time = 0
            #     # 重置今日任务
            #     for task in self.today_tasks:
            #         task['completed_seconds'] = 0
            #     self.update_today_tasks_display()

            self.is_running = True
            self.start_time = time.time()
            self.pause_button.config(state=tk.NORMAL)

            # 禁用所有播放按钮
            for row in self.task_rows:
                row['play_button'].config(state=tk.DISABLED)

    def load_records(self):
        try:
            self.records = pd.read_csv(self.args.fileName)
            if not self.args.resetToday:
                with open(self.args.targetTodayFileName, "r") as file:
                    today_target = json.loads(file.read())
                if len(today_target):
                    self.today_tasks = today_target
            if self.args.targetFileName:
                self.targetInfo = pd.read_csv(self.args.targetFileName)
        except Exception as e:
            self.records = pd.DataFrame(columns=["start_time", "end_time", "task_name"])
            import traceback
            print(f"Failed to load records: {traceback.format_exc()}")

    def start_timer(self):
        if not self.is_running:
            self.task_name = simpledialog.askstring("输入任务类型", "请输入任务类型:")
            if not self.task_name:
                messagebox.showinfo("提示", "任务类型不能为空！")
                return
            self.task_label.config(text=f"当前任务: {self.task_name}")
            # time_string = time.strftime('%Y-%m-%d', time.localtime(time.time()))
            # if time_string != self.today_time_string:
            #     self.today_time_string = time_string
            #     self.elapsed_time = 0
            #     # 重置今日任务
            #     for task in self.today_tasks:
            #         task['completed_seconds'] = 0
            #     self.update_today_tasks_display()

            self.is_running = True
            self.start_time = time.time()
            self.pause_button.config(state=tk.NORMAL)

            # 禁用所有播放按钮
            for row in self.task_rows:
                row['play_button'].config(state=tk.DISABLED)

    def run_timer(self):
        while True:
            today_remaining_seconds = self.today_countdown()
            self.countdown_time_label.config(text=self.format_time(today_remaining_seconds))
            if self.is_running:
                duration = time.time() - self.start_time
                elapsed_time = duration + self.elapsed_time
                task_remaining_seconds = sum([max(0, task["target_hours"]*3600-task["completed_seconds"]) for task in self.today_tasks])
                # 更新当前任务的时间显示
                self.task_label.config(text=f"当前任务: {self.task_name}, 用时: {self.format_time(duration, cn=True)}, \n今日用时: {self.format_time(elapsed_time, cn=True)}, \n剩余用时： {self.format_time(task_remaining_seconds, cn=True)}")
                # 更新今日剩余时间
                current_task_seconds = duration
                for task in self.today_tasks:
                    if task['task_name'] == self.task_name:
                        current_task_seconds += task['completed_seconds']
                        break
                # 更新当前任务的时间显示
                for row in self.task_rows:
                    if row['task_name'] == self.task_name:
                        task_obj = next((t for t in self.today_tasks if t['task_name'] == self.task_name), None)
                        if task_obj:
                            current_seconds = task_obj['completed_seconds'] + duration
                            current_hours = current_seconds / 3600
                            time_text = f"{self.format_time(current_seconds)}小时 / {task_obj['target_hours']:.2f}小时"
                            row['countdown_time_label'].config(text=time_text)
                        break
            time.sleep(1)

    def format_time(self, elapsed, cn: bool = False, dayHour: Optional[int] = None):
        sign = "" if elapsed >= 0 else "-"
        abs_elapsed = abs(elapsed)
        days, remainder = divmod(int(abs_elapsed), 3600*dayHour) if dayHour is not None else (0, int(abs_elapsed))
        hours, remainder = divmod(remainder, 3600)
        minutes, seconds = divmod(remainder, 60)
        if cn:
            return f"{int(days)}天 {int(hours)} 小时 {int(minutes)}分钟 {int(seconds)}秒" if dayHour is not None else f"{int(hours)}小时 {int(minutes)}分钟 {int(seconds)}秒"
        else:
            return f"{hours:02}:{minutes:02}:{seconds:02}"

    def pause_timer(self):
        if self.is_running:
            self.elapsed_time += time.time() - self.start_time
            # 更新今日任务的完成时间
            for task in self.today_tasks:
                if task['task_name'] == self.task_name:
                    task['completed_seconds'] += time.time() - self.start_time
                    break
            self.save_record()
            self.is_running = False
            self.pause_button.config(state=tk.DISABLED)
            self.update_records_display()
            self.update_today_tasks_display()

            # 启用所有播放按钮
            for row in self.task_rows:
                row['play_button'].config(state=tk.NORMAL)
            with open(args.targetTodayFileName, "w") as file:
                file.write(json.dumps(self.today_tasks))

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
            task_name = row['task_name']
            self.record_per_type[task_name] += row['end_time'] - row['start_time']
            dayString = time.strftime('%Y-%m-%d', time.localtime(row['start_time']))
            self.record_per_day[dayString] += row['end_time'] - row['start_time']
            if task_name not in self.targetInfo["task_name"].to_list():
                self.targetInfo.loc[len(self.targetInfo)] = [task_name, 0, False]
                if self.args.targetFileName:
                    self.targetInfo.to_csv(self.args.targetFileName, index_label=False)
        targetEstimateTime: dict = self.targetInfo.set_index('task_name').to_dict()['estimate_time(day)']
        isTargetFinished: dict = self.targetInfo.set_index('task_name').to_dict()['done']

        sorted_targets = dict(sorted(targetEstimateTime.items(), key=lambda x: (isTargetFinished[x[0]], self.record_per_type.get(x[0], 0)-x[1]*3600)))
        sum_distance = sum([estimateTime-min(estimateTime, self.record_per_type.get(task_name,0)/3600/self.args.dayHour) for task_name, estimateTime in sorted_targets.items() if not isTargetFinished[task_name]])*3600*self.args.dayHour
        self.statistics_text.insert(tk.END, f"In total({self.args.dayHour}h): {self.format_time(sum(self.record_per_type.values()), cn=True, dayHour=self.args.dayHour)}\nRemain time: {self.format_time(sum_distance, cn=True, dayHour=self.args.dayHour)}\n\n")
        for task, estimate_time in sorted_targets.items():
            prefix = "[X]" if isTargetFinished[task] else "[ ]"
            self.statistics_text.insert(tk.END, f"{prefix} {task}: {self.format_time(self.record_per_type.get(task, 0), cn=True, dayHour=self.args.dayHour)} / {estimate_time}天\n")

        self.statistics_text.config(state=tk.DISABLED)

        # Update daily statics
        self.daily_text.config(state=tk.NORMAL)
        self.daily_text.delete(1.0, tk.END)
        for dayString, duration in self.record_per_day.items():
            self.daily_text.insert(1.0, f"{dayString}: {self.format_time(duration, cn=True)}\n")
        self.daily_text.config(state=tk.DISABLED)

    def quit_app(self):
        if self.is_running:
            self.save_record()
        else:
            self.root.quit()

if __name__ == "__main__":
    root = tk.Tk()
    parser = argparse.ArgumentParser()
    parser.add_argument("--fileName", default="records.csv")
    parser.add_argument("--targetFileName", default="")
    parser.add_argument("--targetTodayFileName", default="")
    parser.add_argument("--dayHour", default=8)
    parser.add_argument("--resetToday", default=False, action="store_true")
    args = parser.parse_args()
    app = TimerApp(root, args)
    root.protocol("WM_DELETE_WINDOW", app.quit_app)
    root.mainloop()