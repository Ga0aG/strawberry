import tkinter as tk
from tkinter import messagebox, simpledialog
import json
import os
from datetime import datetime

DATA_FILE = "todo.json"

class TodoApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Todo List")

        self.tasks = []
        self.completed_tasks = []

        # Load data from JSON file
        self.load_data()

        # Create UI elements
        self.create_widgets()
        self.update_task_list()

    def create_widgets(self):
        self.task_frame = tk.Frame(self.root)
        self.task_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.completed_frame = tk.Frame(self.root)
        self.completed_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

        self.task_listbox = tk.Listbox(self.task_frame, selectmode=tk.SINGLE)
        self.task_listbox.pack(fill=tk.BOTH, expand=True)
        self.task_listbox.bind("<Double-1>", self.complete_task)

        self.completed_listbox = tk.Listbox(self.completed_frame)
        self.completed_listbox.pack(fill=tk.BOTH, expand=True)

        self.add_task_button = tk.Button(self.root, text="Add Task", command=self.add_task)
        self.add_task_button.pack()

    def add_task(self):
        task_name = simpledialog.askstring("Task", "Enter Task Name:")
        if task_name:
            task = {
                "name": task_name,
                "created_time": str(datetime.now()),
                "index": len(self.tasks),
                "completed": False
            }
            self.tasks.append(task)
            self.save_data()
            self.update_task_list()

    def complete_task(self, event):
        selected_index = self.task_listbox.curselection()
        if selected_index:
            index = selected_index[0]
            task = self.tasks.pop(index)
            task["completed_time"] = str(datetime.now())
            self.completed_tasks.append(task)
            self.save_data()
            self.update_task_list()

    def update_task_list(self):
        self.task_listbox.delete(0, tk.END)
        self.completed_listbox.delete(0, tk.END)

        # Update pending tasks
        self.tasks.sort(key=lambda x: x["index"])
        for task in self.tasks:
            self.task_listbox.insert(tk.END, task["name"])

        # Update completed tasks
        self.completed_tasks.sort(key=lambda x: x["completed_time"], reverse=True)
        for task in self.completed_tasks:
            self.completed_listbox.insert(tk.END, f"{task['name']} (Completed: {task['completed_time']})")

    def load_data(self):
        if os.path.exists(DATA_FILE):
            with open(DATA_FILE, 'r') as f:
                data = json.load(f)
                self.tasks = data.get("tasks", [])
                self.completed_tasks = data.get("completed_tasks", [])

    def save_data(self):
        with open(DATA_FILE, 'w') as f:
            json.dump({"tasks": self.tasks, "completed_tasks": self.completed_tasks}, f)


if __name__ == "__main__":
    root = tk.Tk()
    app = TodoApp(root)
    root.protocol("WM_DELETE_WINDOW", root.quit)
    root.mainloop()