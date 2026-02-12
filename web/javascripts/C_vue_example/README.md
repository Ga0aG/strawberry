# Vue Example Project

## 简介
这是一个展示Vue基本用法的简单项目，包含计数器和待办清单两个功能。

## 项目特点
- 使用Vue 3的Composition API
- ref() 用于响应式状态管理
- v-model 双向数据绑定
- 事件处理指令（@click, @keyup等）
- 列表渲染和条件渲染
- scoped style 作用域样式

## 安装依赖
```bash
npm install
```

## 开发
```bash
npm run dev
```

## 构建
```bash
npm run build
```

## 项目结构
```
C_vue_example/
├── src/
│   ├── App.vue          # 主组件（单文件组件）
│   └── main.js          # 应用入口
├── index.html           # HTML模板
├── package.json         # 项目配置
└── webpack.config.js    # Webpack配置
```

## 主要特性说明

### 1. 单文件组件（SFC）
Vue的单文件组件（.vue）将模板、脚本和样式放在一个文件中

### 2. Composition API
- ref(): 创建响应式数据
- 函数式组件编写方法
- 灵活的逻辑组织

### 3. 模板语法
- {{ }}: 文本插值
- v-model: 双向数据绑定
- @click: 事件绑定
- v-for: 列表渲染
- :class: 动态class绑定
- v-if: 条件渲染

### 4. scoped Style
- 样式只作用于当前组件
- 避免样式污染

## 学习价值
- 理解Vue3的Composition API
- 学习单文件组件的使用
- 掌握Vue的响应式系统
- 理解双向数据绑定的概念
