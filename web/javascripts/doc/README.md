# 文档目录

本目录包含React和Vue框架的对比分析和学习材料。

## 文档列表

### 1. React vs Vue 对比指南 (`React_vs_Vue.md`)

详细的对比文档，包含：

- **核心特性对比**：编程范式、模板语法、状态管理、事件处理、列表渲染
- **实际应用对比**：项目结构、运行方式、代码示例
- **性能对比**：包大小、运行时性能、内存使用
- **开发体验对比**：优缺点分析
- **应用场景**：何时选择React或Vue
- **代码示例对比**：计数器和待办清单的实现对比
- **进阶主题**：生态系统和官方工具
- **最终建议**：选择框架的指导意见

## 相关项目

### B_react_example
最简单的React项目示例，包含：
- 计数器功能
- 待办清单功能
- 使用React 18函数组件和Hooks
- 基于Webpack构建

**快速开始：**
```bash
cd B_react_example
npm install
npm run dev
```

### C_vue_example
最简单的Vue项目示例，包含：
- 计数器功能
- 待办清单功能
- 使用Vue 3 Composition API
- 单文件组件（.vue）
- 基于Webpack构建

**快速开始：**
```bash
cd C_vue_example
npm install
npm run dev
```

## 学习路径建议

### 第一阶段：理解框架设计
1. 阅读 `React_vs_Vue.md` 的前两部分
2. 理解两个框架的核心设计理念
3. 理解为什么会有这些差异

### 第二阶段：亲身体验
1. 运行 `B_react_example` 项目
2. 运行 `C_vue_example` 项目
3. 修改代码，观察界面变化
4. 比较两个项目的开发体验

### 第三阶段：深入学习
1. 学习官方文档
2. 查看 `React_vs_Vue.md` 的进阶内容
3. 探索生态系统和官方工具

### 第四阶段：实战应用
1. 选择一个框架深入学习
2. 用它构建小型项目
3. 逐步扩大项目复杂度

## 快速参考

### 何时使用React
- ✅ 大型企业应用
- ✅ 需要极高灵活性
- ✅ 跨平台开发（React Native）
- ✅ 复杂状态管理

### 何时使用Vue
- ✅ 中小型项目
- ✅ 快速原型开发
- ✅ 团队初级开发者较多
- ✅ 重视开发效率
- ✅ 前端为主的项目

## 关键概念对比速查表

| 概念 | React | Vue |
|-----|------|-----|
| 组件文件 | `.jsx` | `.vue` |
| 状态声明 | `useState()` | `ref()` |
| 数据修改 | 不可变（创建新对象） | 可变（直接修改） |
| 模板语法 | JSX | Vue Template |
| 事件绑定 | `onClick={}` | `@click` |
| 双向绑定 | 无原生支持 | `v-model` |
| 条件渲染 | `&&` 或三元表达式 | `v-if/v-show` |
| 列表渲染 | `.map()` | `v-for` |
| 样式绑定 | `className` | `:class` |
| 样式隔离 | CSS Modules或BEM | scoped style |

## 进阶阅读

- [Vue官方的React对比](https://vuejs.org/guide/extras/comparison.html#react)
- [React官方文档](https://react.dev)
- [Vue官方文档](https://vuejs.org)
- [Next.js - React全栈框架](https://nextjs.org)
- [Nuxt - Vue全栈框架](https://nuxt.com)

## 常见问题

### Q: React和Vue哪个好？
A: 两个都很好，只是应用场景和设计哲学不同。选择取决于项目需求和团队情况。

### Q: 学了React后学Vue容易吗？
A: 是的。核心概念相通，主要是语法和思维方式的调整。

### Q: 生产环境应该用哪个？
A: 两个都可以。大公司更多用React，初创和中小公司更多用Vue。

### Q: 能同时使用两个框架吗？
A: 不推荐。同时维护两个框架的项目成本很高。

---

**最后更新：** 2025年12月
