ovsdb采用sha1算法加密，所以手动修改的db必须重新加密一遍。

修改db流程如下：

1：复制conf.db到当前目录下

2：运行gen_yaml.py, 该脚本会在当前目录下创建output目录，
并且将conf.db中对应行数的json db转换为"行号.yaml"文件到
output目录中。

3：去output里修改相应的yaml

4：运行gen_db.py，会在当前目录生成修改好的db，文件名为output.db
