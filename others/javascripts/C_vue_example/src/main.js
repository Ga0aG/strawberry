import { createApp } from 'vue';
import App from './App.vue';

const app = createApp(App); // 使用 createApp 创建一个新的 Vue 应用实例，传入根组件 App

app.mount('#app'); // 将应用实例挂载到 DOM 元素上，ID 为 'app'，启动应用
