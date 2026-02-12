<template>
  <div class="container">
    <h1>Vue Example</h1>

    <!-- Counter -->
    <div class="section">
      <h2>Counter</h2>
      <p>Count: <span class="count">{{ count }}</span></p>
      <button @click="count++">+</button>
      <button @click="count--">-</button>
      <button @click="count = 0">Reset</button>
    </div>

    <!-- Todo List -->
    <div class="section">
      <h2>Todo List</h2>
      <div class="input-group">
        <input
          v-model="input"
          type="text"
          placeholder="Add a new task..."
          @keyup.enter="addTodo"
        />
        <button @click="addTodo">Add</button>
      </div>

      <ul class="todo-list">
        <li
          v-for="todo in todos"
          :key="todo.id"
          :class="{ completed: todo.completed }"
        >
          <input
            type="checkbox"
            v-model="todo.completed"
          />
          <span>{{ todo.text }}</span>
          <button @click="deleteTodo(todo.id)">Delete</button>
        </li>
      </ul>
    </div>
  </div>
</template>

<script>
import { ref } from 'vue';

export default {
  name: 'App',
  setup() {
    const count = ref(0);
    const todos = ref([]);
    const input = ref('');

    const addTodo = () => {
      if (input.value.trim()) {
        todos.value.push({
          id: Date.now(),
          text: input.value,
          completed: false,
        });
        input.value = '';
      }
    };

    const deleteTodo = (id) => {
      todos.value = todos.value.filter(todo => todo.id !== id);
    };

    return {
      count,
      todos,
      input,
      addTodo,
      deleteTodo,
    };
  },
};
</script>

<style scoped>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen',
    'Ubuntu', 'Cantarell', 'Fira Sans', 'Droid Sans', 'Helvetica Neue',
    sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  background: linear-gradient(135deg, #42b983 0%, #2c5f2d 100%);
  min-height: 100vh;
  padding: 20px;
}

.container {
  max-width: 600px;
  margin: 0 auto;
  background: white;
  border-radius: 10px;
  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.2);
  padding: 30px;
}

h1 {
  color: #333;
  margin-bottom: 30px;
  text-align: center;
  font-size: 2em;
}

.section {
  margin-bottom: 40px;
  padding-bottom: 30px;
  border-bottom: 1px solid #eee;
}

.section:last-child {
  border-bottom: none;
}

h2 {
  color: #555;
  margin-bottom: 20px;
  font-size: 1.3em;
}

p {
  color: #666;
  margin-bottom: 15px;
  font-size: 1.1em;
}

.count {
  color: #42b983;
  font-weight: bold;
  font-size: 1.5em;
}

button {
  background: linear-gradient(135deg, #42b983 0%, #2c5f2d 100%);
  color: white;
  border: none;
  padding: 10px 20px;
  margin-right: 10px;
  margin-top: 10px;
  border-radius: 5px;
  cursor: pointer;
  font-size: 1em;
  transition: transform 0.2s, box-shadow 0.2s;
}

button:hover {
  transform: translateY(-2px);
  box-shadow: 0 5px 15px rgba(66, 185, 131, 0.4);
}

button:active {
  transform: translateY(0);
}

.input-group {
  display: flex;
  gap: 10px;
  margin-bottom: 20px;
}

input[type="text"] {
  flex: 1;
  padding: 10px;
  border: 2px solid #ddd;
  border-radius: 5px;
  font-size: 1em;
  transition: border-color 0.3s;
}

input[type="text"]:focus {
  outline: none;
  border-color: #42b983;
}

.todo-list {
  list-style: none;
}

.todo-list li {
  display: flex;
  align-items: center;
  padding: 12px;
  background: #f9f9f9;
  margin-bottom: 8px;
  border-radius: 5px;
  transition: background 0.2s;
}

.todo-list li:hover {
  background: #f0f0f0;
}

.todo-list li.completed {
  opacity: 0.6;
}

.todo-list li.completed span {
  text-decoration: line-through;
  color: #999;
}

.todo-list input[type="checkbox"] {
  margin-right: 10px;
  width: 18px;
  height: 18px;
  cursor: pointer;
}

.todo-list span {
  flex: 1;
  color: #333;
}

.todo-list button {
  background: #ff6b6b;
  padding: 5px 10px;
  margin: 0;
  font-size: 0.9em;
}

.todo-list button:hover {
  background: #ff5252;
}
</style>
