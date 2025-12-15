# 只在 robot5 上部署
ansible-playbook -i inventory.ini playbook.yml --limit robot5