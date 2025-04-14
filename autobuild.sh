# 删除旧的build目录
rm -rf build

# 重新创建build目录并进入
mkdir build && cd build

# 运行cmake配置
cmake ..

# 编译项目
make