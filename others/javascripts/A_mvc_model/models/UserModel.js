// models/UserModel.js
class UserModel {
    constructor() {
      this.users = [
        { id: 1, name: "张三" },
        { id: 2, name: "李四" }
      ];
    }

    // 获取所有用户
    getAllUsers() {
      return this.users;
    }

    // 添加新用户
    addUser(user) {
      const newUser = { id: Date.now(), ...user };
      this.users.push(newUser);
      return newUser;
    }
  }

  module.exports = UserModel;