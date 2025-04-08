// app.js
const express = require('express');
const UserController = require('./controllers/UserController');

const app = express();
const userController = new UserController();

// 配置中间件
app.set('view engine', 'ejs');
app.use(express.urlencoded({ extended: true }));

// 路由配置
app.get('/users', userController.showUsers.bind(userController));
app.post('/users', userController.addUser.bind(userController));

// 启动服务器
app.listen(3000, () => {
  console.log('服务器运行在 http://localhost:3000');
});