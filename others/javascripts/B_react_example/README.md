# React Example Project

## 简介
这是一个展示React基本用法的简单项目，包含计数器和待办清单两个功能。

## 项目特点
- 使用React 18的函数组件和Hooks
- useState Hook用于状态管理
- 事件处理（onClick, onChange等）
- 列表渲染和条件渲染
- CSS样式设计

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
B_react_example/
├── src/
│   ├── App.jsx          # 主组件
│   ├── index.jsx        # 应用入口
│   └── index.css        # 样式文件
├── index.html           # HTML模板
├── package.json         # 项目配置
└── webpack.config.js    # Webpack配置
```

## 主要特性说明

### 1. 函数组件
React采用函数组件作为基础，简洁易懂

### 2. Hooks
- useState: 管理组件状态
- 支持多个状态变量

### 3. 事件处理
- onClick: 按钮点击事件
- onChange: 输入框变化事件
- onKeyPress: 键盘事件

### 4. 条件渲染
- 使用三元表达式
- 列表渲染使用map()和key属性

## 学习价值
- 理解React组件的基本概念
- 学习状态管理的基础用法
- 掌握函数组件的编写方法

## bundle.js 是由 webpack 通过以下流程生成的：

生成流程：

源代码                webpack处理                   输出
├─ src/index.jsx
├─ src/App.jsx      ──> 编译&打包 ──>  dist/bundle.js
├─ src/index.css                      (在内存或硬盘)
└─ node_modules/*

### 具体步骤：

1. 入口点 (Entry): ./src/index.jsx - webpack 从这个文件开始
2. 转译 (Loaders):
- babel-loader: 将 JSX/ES6+ 转译成浏览器可理解的 JavaScript
- css-loader: 处理 CSS 导入
- style-loader: 将 CSS 注入到 HTML 中
3. 打包 (Bundling):
- 将所有依赖的模块（index.jsx → App.jsx、index.css 等）打包成一个文件
4. 输出 (Output):
- 生成到 dist/bundle.js（生产模式）
- 或在内存中（开发模式，npm run dev）

在你的项目中：

生产环境 - 运行 npm run build # 生成 dist/bundle.js

开发环境 - 运行 npm run dev # webpack-dev-server 在内存中生成 bundle.js。 支持热更新（修改代码时自动重新编译）

### 为什么 devServer 能访问 bundle.js？

在开发模式下，webpack-dev-server 不会写入硬盘，而是把编译好的文件存在内存中，通过 web server 直接提供：
- 当你访问 http://localhost:3000/bundle.js 时，它从内存中读取并返回
- 当你修改代码时，webpack 自动重新编译，浏览器自动刷新（hot reload）

所以你现在运行 npm run dev 后，webpack 就会自动编译并生成 bundle.js 在内存中供网页使用

## 状态和 CSS 的对应关系

在 React 中，通过 className 属性将 HTML 元素与 CSS 样式连接：

  <element className="css-class-name">

  App.jsx 中的对应关系：

  | 状态           | JSX 代码                                      | className          | CSS 样式      | 效果             |
  |----------------|-----------------------------------------------|--------------------|---------------|------------------|
  | count          | <span className="count">{count}</span>        | .count             | 紫色加粗      | 显示计数器数值   |
  | input 值       | <input type="text" value={input} />           | input[type="text"] | 边框样式      | 输入框的样式     |
  | todos 数组     | <ul className="todo-list">                    | .todo-list         | 列表样式      | 待办列表容器     |
  | todo.completed | className={todo.completed ? 'completed' : ''} | .completed         | 透明度+删除线 | 完成的项显示灰色 |

  具体例子：

  1. 计数器状态 (count)

  // App.jsx
  <span className="count">{count}</span>

  // index.css
  .count {
    color: #667eea;        /* 紫色 */
    font-weight: bold;     /* 加粗 */
    font-size: 1.5em;      /* 放大 */
  }

  2. 待办完成状态 (todo.completed)

  // App.jsx - 根据完成状态动态添加 class
  <li className={todo.completed ? 'completed' : ''}>
    <span>{todo.text}</span>
  </li>

  // 如果 completed=true，输出的 HTML 为：
  <li class="completed">
    <span>买菜</span>
  </li>

  // 如果 completed=false，输出的 HTML 为：
  <li>
    <span>买菜</span>
  </li>

  /* index.css - completed 类的样式 */
  .todo-list li.completed {
    opacity: 0.6;  /* 变透明 */
  }

  .todo-list li.completed span {
    text-decoration: line-through;  /* 删除线 */
    color: #999;                    /* 灰色 */
  }

## 标签说明

CSS 选择器的三种主要类型：

| 符号 | 类型       | 用途                    | 例子             |
|------|------------|-------------------------|------------------|
| .    | 类选择器   | 选择 class="..." 的元素 | .count           |
| 无   | 元素选择器 | 选择特定的 HTML 标签    | button, body, h1 |
| #    | ID 选择器  | 选择 id="..." 的元素    | #root            |

在你的项目中的例子：

1. 元素选择器（无前缀）

body {
font-family: '...';
background: linear-gradient(...);
}

button {
background: linear-gradient(...);
color: white;
border: none;
}

h1 {
color: #333;
margin-bottom: 30px;
}
这些作用于所有 <body>、<button>、<h1> 标签。

2. 类选择器（. 开头）

.container {
max-width: 600px;
margin: 0 auto;
}

.count {
color: #667eea;
font-weight: bold;
}

.completed {
opacity: 0.6;
}
这些作用于 class="container" 这样的元素

3. 对应的 JSX 代码

// 元素选择器直接作用
<button onClick={() => setCount(count + 1)}>+</button>
// ↑ 所有 button 都会获得 button 的 CSS 样式

// 类选择器通过 className 作用
<span className="count">{count}</span>
// ↑ 只有这个 span 获得 .count 的样式

<div className="container">...</div>
// ↑ 只有这个 div 获得 .container 的样式

优先级和使用场景：

元素选择器（button）
    ↓ 通用，作用于所有 <button> 标签

类选择器（.btn-primary）
    ↓ 更具体，可以针对特定的元素子集
    ↓ 优先级更高，会覆盖元素选择器的样式

ID 选择器（#submit）
    ↓ 最具体，一般用于唯一的元素
    ↓ 优先级最高
