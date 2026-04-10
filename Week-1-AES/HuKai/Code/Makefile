# 编译器
CXX = g++

# 编译选项
# -std=c++11 : 使用 C++11 标准
# -O2        : 开启二级优化（对 AES-NI 性能至关重要）
# -maes      : 启用 AES 指令集
# -Wall      : 显示所有警告
CXXFLAGS = -std=c++11 -O2 -maes -Wall

# 目标文件名 (Windows下可改为 main.exe)
TARGET = main

# 源文件
SRCS = aes.cpp main.cpp

# 默认目标
all: $(TARGET)

# 链接规则
$(TARGET): $(SRCS)
	$(CXX) $(SRCS) -o $(TARGET) $(CXXFLAGS)
	@echo "编译成功! 运行 ./$(TARGET)"

# 清理规则 (输入 make clean 执行)
clean:
	rm -f $(TARGET)
	@echo "已清理编译文件"

# 强制重新编译
rebuild: clean all

	
