// controllers/UserController.js
const UserModel = require('../models/UserModel');

class UserController {
  constructor() {
    this.userModel = new UserModel();
  }

  // 显示用户列表
  showUsers(req, res) {
    const users = this.userModel.getAllUsers();
    res.render('userView', { users });
  }

  // 处理添加用户
  addUser(req, res) {
    const { name } = req.body;
    if (name) {
      this.userModel.addUser({ name });
    }
    res.redirect('/users');
  }
}

module.exports = UserController;