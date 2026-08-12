# Linux Shell Basics

本篇文章主要介绍 Bash Shell 脚本编程的核心语法，包含详细的用例讲解。Linux 基础命令可参考前文 [Linux Concise Tutorial](Linux简明教程.md)。

## Table of Contents

- [1. Shell 脚本的基本结构](#1-shell-脚本的基本结构)
- [2. 变量](#2-变量)
- [3. 条件判断](#3-条件判断)
- [4. 循环](#4-循环)
- [5. 函数](#5-函数)
- [6. 数组](#6-数组)
- [7. 字符串操作](#7-字符串操作)
- [8. 输入输出重定向](#8-输入输出重定向)
- [9. 错误处理与调试](#9-错误处理与调试)
- [10. 综合实战案例](#10-综合实战案例)

---

## 1. Shell 脚本的基本结构

### Shebang 行

每个 Shell 脚本的第一行必须指定解释器：

```bash
#!/bin/bash
# 这是注释
echo "Hello, World!"
```

常见的 Shebang 写法：

```bash
#!/bin/bash          # 最常用，Bash 解释器
#!/usr/bin/env bash  # 更可移植的写法，从 PATH 中查找 bash
#!/bin/sh            # POSIX shell，兼容性最好但功能较少
#!/bin/zsh           # Zsh 解释器
```

### 运行脚本的三种方式

假设脚本保存为 `backup.sh`：

```bash
# 方式1：直接用 bash 解释器运行（不需执行权限）
bash backup.sh

# 方式2：赋予执行权限后直接运行
chmod +x backup.sh
./backup.sh

# 方式3：source 命令（在当前 shell 环境中运行，会影响当前环境变量）
source backup.sh
# 或简写为：
. backup.sh
```

> **注意**：`./backup.sh` 和 `bash backup.sh` 会在子 shell 中运行，脚本中的变量不会污染当前 shell。而 `source` 在当前 shell 运行，脚本中定义的变量和 `cd` 等操作会保留。

### 入门案例：备份脚本

```bash
#!/bin/bash

# 定义源目录和目标目录, 获取当前日期
SOURCE_DIR="/home/user/documents"
DEST_DIR="/home/user/backup"
DATE=$(date +"%Y%m%d")

# 创建目标目录（如果不存在）
mkdir -p $DEST_DIR

# 打包源目录并命名为备份文件
tar -zcvf $DEST_DIR/backup_$DATE.tar.gz $SOURCE_DIR

# 显示备份完成信息
echo "备份完成：$DEST_DIR/backup_$DATE.tar.gz"
```

### 注释

```bash
# 单行注释：以 # 开头

# 多行注释方式1：使用 : ' ... '
: '
这是多行注释
可以跨越多行
'

# 多行注释方式2：使用 heredoc（不会执行）
<<'COMMENT'
这是另一种多行注释方式
适用于注释大段代码
COMMENT
```

---

## 2. 变量

### 定义和使用变量

**核心规则**：定义变量时 `=` 两端不能有空格；使用变量时加 `$`。

```bash
#!/bin/bash
# 定义变量（等号两边不能有空格！）
name="Alice"
age=25
PI=3.14159

# 使用变量（推荐用双引号包裹）
echo "Name: $name"
echo "Age: ${age}"          # 推荐用 ${} 包裹变量名，更安全
echo "PI: ${PI}"

# 变量未定义时的默认值
echo ${undefined_var:-"默认值"}  # 输出：默认值
```

### 特殊变量（位置参数）

Shell 提供了一系列内置的特殊变量，用于获取脚本参数和状态：

| 变量   | 含义                         | 示例                         |
| ------ | ---------------------------- | ---------------------------- |
| `$0`   | 脚本名称                     | `./script.sh`                |
| `$1`~`$9` | 第1~9个位置参数           | 执行 `./script.sh a b c` → `$1` = a, `$2` = b |
| `${10}` | 第10个及以上的位置参数（需要花括号） | `${10}` = 第10个参数       |
| `$#`   | 参数个数                     | `./script.sh a b c` → `$#` = 3 |
| `$@`   | 所有参数（每个参数单独引用） | 推荐用于遍历参数             |
| `$*`   | 所有参数（合并为一个字符串） | 不推荐，会丢失空格信息       |
| `$?`   | 上一条命令的退出状态码       | `0` = 成功，非0 = 失败       |
| `$$`   | 当前 Shell 进程 PID          | `12345`                      |
| `$!`   | 最近后台命令的 PID           | `12346`                      |
| `$_`   | 上一条命令的最后一个参数     |                              |

```bash
#!/bin/bash
echo "脚本名: $0"
echo "第一个参数: $1"
echo "第二个参数: $2"
echo "参数总数: $#"
echo "所有参数(独立): $@"
echo "当前 PID: $$"

# 遍历所有参数（推荐方式）
for arg in "$@"; do
    echo "参数: $arg"
done
```

### 读取用户输入

```bash
#!/bin/bash

# 基本输入
echo -n "请输入你的名字: "
read name
echo "你好, ${name}!"

# 使用 -p 提示符（更简洁）
read -p "请输入年龄: " age

# 隐藏输入（输入密码）
read -s -p "请输入密码: " password
echo ""  # 换行
echo "密码已记录"

# 带超时的输入（-t 秒）
read -t 5 -p "5秒内输入你的选择(y/n): " choice
echo "你选择了: ${choice:-超时}"

# 限制输入字符数（-n）
read -n 1 -p "按任意键继续..." key
echo ""
```

### 算术运算

```bash
#!/bin/bash
a=10
b=3

# 使用 $(( )) 进行整数运算（推荐）
echo $(( a + b ))    # 加法 → 13
echo $(( a - b ))    # 减法 → 7
echo $(( a * b ))    # 乘法 → 30
echo $(( a / b ))    # 整数除法 → 3
echo $(( a % b ))    # 取余 → 1
echo $(( a ** b ))   # 幂运算 → 1000

# 自增/自减
(( a++ ))
echo $a              # 11
(( b-- ))
echo $b              # 2

# 浮点数运算需要使用 bc 命令
echo "scale=2; 10 / 3" | bc    # → 3.33
echo "scale=4; sqrt(2)" | bc   # 平方根

# 使用 let 命令（老旧写法，不推荐）
let "c = a + b"
```

### 环境变量 vs 局部变量

```bash
#!/bin/bash

# 局部变量（仅当前脚本可用）
local_var="我是局部变量"

# 环境变量（子进程也能访问）
export GLOBAL_VAR="我是全局变量"

# 查看所有环境变量
env
printenv

# 常用系统环境变量
echo "HOME: $HOME"
echo "PATH: $PATH"
echo "USER: $USER"
echo "SHELL: $SHELL"
echo "PWD: $PWD"
```

### 命令替换

将命令的输出赋值给变量：

```bash
#!/bin/bash

# 方式1：使用 $()（推荐）
current_date=$(date +"%Y-%m-%d")
file_count=$(ls -1 | wc -l)
echo "今天是: $current_date"
echo "当前目录有 $file_count 个文件"

# 方式2：使用反引号 ``（老旧写法，不推荐，难以嵌套）
current_user=`whoami`

# 嵌套命令替换
# 查找最大的 3 个文件
largest_files=$(ls -lhS $(find . -type f) 2>/dev/null | head -3)
echo "$largest_files"
```

---

## 3. 条件判断

### if / elif / else 语句

```bash
#!/bin/bash
read -p "请输入年龄: " age

if [ $age -lt 13 ]; then
    echo "儿童"
elif [ $age -lt 18 ]; then
    echo "青少年"
elif [ $age -lt 60 ]; then
    echo "成年人"
else
    echo "老年人"
fi
```

### 整数比较运算符

| 运算符 | 含义       | 示例                    |
| ------ | ---------- | ----------------------- |
| `-eq`  | 等于       | `[ $a -eq $b ]`         |
| `-ne`  | 不等于     | `[ $a -ne $b ]`         |
| `-gt`  | 大于       | `[ $a -gt $b ]`         |
| `-ge`  | 大于等于   | `[ $a -ge $b ]`         |
| `-lt`  | 小于       | `[ $a -lt $b ]`         |
| `-le`  | 小于等于   | `[ $a -le $b ]`         |

### 字符串比较运算符

| 运算符 | 含义             | 示例                         |
| ------ | ---------------- | ---------------------------- |
| `=` 或 `==` | 字符串相等   | `[ "$a" = "$b" ]`            |
| `!=`   | 字符串不等       | `[ "$a" != "$b" ]`           |
| `-z`   | 字符串长度为 0（空） | `[ -z "$str" ]`          |
| `-n`   | 字符串长度不为 0  | `[ -n "$str" ]`              |
| `\<`   | 字典序小于 ([[ ]])| `[[ "$a" < "$b" ]]`        |
| `\>`   | 字典序大于 ([[ ]])| `[[ "$a" > "$b" ]]`        |

```bash
#!/bin/bash
str1="hello"
str2="world"

# 空字符串检查
if [ -z "$str1" ]; then
    echo "str1 是空字符串"
fi

# 字符串相等判断
if [ "$str1" = "hello" ]; then
    echo "str1 等于 hello"
fi

# 注意：变量一定要用双引号包裹，否则变量为空时会报语法错误
if [ "$maybe_empty" = "target" ]; then
    echo "匹配"
fi
```

### 文件测试运算符

```bash
#!/bin/bash
FILE="/etc/passwd"

# 文件类型判断
[[ -f "$FILE" ]] && echo "$FILE 是普通文件"
[[ -d "$FILE" ]] && echo "$FILE 是目录"
[[ -e "$FILE" ]] && echo "$FILE 存在（文件或目录）"
[[ -L "$FILE" ]] && echo "$FILE 是符号链接"
[[ -b "$FILE" ]] && echo "$FILE 是块设备文件"
[[ -c "$FILE" ]] && echo "$FILE 是字符设备文件"
[[ -p "$FILE" ]] && echo "$FILE 是管道文件"
[[ -S "$FILE" ]] && echo "$FILE 是 socket 文件"

# 文件权限判断
[[ -r "$FILE" ]] && echo "$FILE 可读"
[[ -w "$FILE" ]] && echo "$FILE 可写"
[[ -x "$FILE" ]] && echo "$FILE 可执行"

# 文件属性判断
[[ -s "$FILE" ]] && echo "$FILE 文件大小 > 0"
[[ -O "$FILE" ]] && echo "你是 $FILE 的所有者"
[[ -N "$FILE" ]] && echo "$FILE 自上次读取后被修改过"

# 文件比较
[[ "$FILE1" -nt "$FILE2" ]] && echo "file1 比 file2 新"
[[ "$FILE1" -ot "$FILE2" ]] && echo "file1 比 file2 旧"
[[ "$FILE1" -ef "$FILE2" ]] && echo "file1 和 file2 指向同一文件（硬链接）"
```

### 组合条件（AND / OR）

```bash
#!/bin/bash
age=25
name="Alice"

# 使用 && (AND) 和 || (OR)
if [ $age -gt 18 ] && [ $age -lt 60 ]; then
    echo "工作年龄"
fi

# 使用 -a (AND) 和 -o (OR)（单中括号内）
if [ $age -gt 18 -a "$name" = "Alice" ]; then
    echo "成年 Alice"
fi

# 使用 [[ ]] 可直接用 && 和 ||
if [[ $age -gt 18 && "$name" == "Alice" ]]; then
    echo "成年 Alice（推荐写法）"
fi

# 多个条件组合
if [[ ($age -gt 18 && $age -lt 60) || "$name" == "Admin" ]]; then
    echo "条件满足"
fi

# 取反
if ! [ -f "config.txt" ]; then
    echo "config.txt 不存在"
fi
```

### `[ ]` vs `[[ ]]` 的区别

| 特性           | `[ ]` (POSIX test)    | `[[ ]]` (Bash 扩展)           |
| -------------- | --------------------- | ----------------------------- |
| 字符串比较     | `=` / `!=`             | `==` / `!=` / `=~` (正则)     |
| 逻辑运算       | `-a` / `-o` / `!`      | `&&` / `\|\|` / `!`           |
| 模式匹配       | 不支持                | 支持 `==` 通配符, `=~` 正则   |
| 变量为空       | 必须加引号 `"$var"`   | 不需要加引号                  |
| `<` `>` 比较   | 需要转义 `\<` `\>`     | 不需要转义                    |

```bash
# [[ ]] 支持通配符匹配
name="hello.txt"
if [[ $name == *.txt ]]; then
    echo "是 .txt 文件"
fi

# [[ ]] 支持正则表达式
phone="123-456-7890"
if [[ $phone =~ ^[0-9]{3}-[0-9]{3}-[0-9]{4}$ ]]; then
    echo "电话号码格式正确"
fi
```

### case 语句（多分支选择）

```bash
#!/bin/bash
read -p "请输入操作系统名称(linux/macos/windows): " os

case $os in
    linux|ubuntu|debian|centos)
        echo "你使用的是 Linux 系统"
        package_manager="apt"
        ;;
    macos|mac|darwin)
        echo "你使用的是 macOS 系统"
        package_manager="brew"
        ;;
    windows|win)
        echo "你使用的是 Windows 系统"
        package_manager="choco"
        ;;
    *)
        echo "未知操作系统: $os"
        exit 1
        ;;
esac
echo "包管理器: $package_manager"
```

### 三元表达式风格

```bash
# 使用 && 和 || 实现类似三元表达式的效果
[ $age -ge 18 ] && echo "成年" || echo "未成年"

# 更复杂的条件赋值
file="${1:-default.txt}"  # 如果 $1 未设置或为空，使用默认值
```

---

## 4. 循环

### for 循环

```bash
#!/bin/bash

# 遍历列表
echo "=== 遍历列表 ==="
for color in red blue green yellow; do
    echo "颜色: $color"
done

# 使用花括号展开（Brace Expansion）
echo "=== 数字范围 ==="
for i in {1..5}; do
    echo "数字: $i"
done

# 带步长的范围
echo "=== 带步长的范围 ==="
for i in {0..10..2}; do
    echo "偶数: $i"
done

# 遍历文件（通配符）
echo "=== 遍历 .txt 文件 ==="
for file in *.txt; do
    [ -f "$file" ] && echo "处理文件: $file"
done

# C 语言风格的 for 循环
echo "=== C 风格 for ==="
for ((i = 1; i <= 5; i++)); do
    echo "计数: $i"
done

# 遍历字符串中的每个字符
str="hello"
for ((i = 0; i < ${#str}; i++)); do
    echo "字符: ${str:$i:1}"
done

# 使用 seq 命令生成序列
for i in $(seq 1 5); do
    echo "seq: $i"
done
```

### while 循环

```bash
#!/bin/bash

# 基本倒计时
echo "=== 倒计时 ==="
count=5
while [ $count -gt 0 ]; do
    echo "倒计时: $count"
    count=$((count - 1))
    sleep 1
done
echo "发射!"

# 无限循环 + break
echo "=== 菜单循环 ==="
while true; do
    read -p "输入数字(0退出): " num
    if [ "$num" -eq 0 ]; then
        echo "退出"
        break
    fi
    echo "你输入了: $num"
done

# 逐行读取文件
echo "=== 读取文件 ==="
while IFS= read -r line; do
    echo "行内容: $line"
done < /etc/hosts

# 读取命令输出
while read -r user home; do
    echo "用户: $user, 家目录: $home"
done < <(awk -F: '{print $1, $6}' /etc/passwd | head -5)
```

### until 循环

`until` 与 `while` 相反：条件为**假**时执行，为真时退出。

```bash
#!/bin/bash

# 等待文件出现
echo "等待 config.lock 文件被删除..."
until [ ! -f "config.lock" ]; do
    echo "文件仍存在，等待中..."
    sleep 2
done
echo "锁文件已释放，继续执行"

# 达到目标值
num=1
until [ $num -gt 5 ]; do
    echo "数字: $num"
    num=$((num + 1))
done
```

### 循环控制：break 和 continue

```bash
#!/bin/bash

# break：立即退出循环
echo "=== break 示例 ==="
for i in {1..10}; do
    if [ $i -eq 6 ]; then
        echo "遇到 6，退出循环"
        break
    fi
    echo "数字: $i"
done

# continue：跳过本次迭代，进入下一次
echo "=== continue 示例 ==="
for i in {1..10}; do
    if (( i % 2 == 0 )); then
        continue  # 跳过偶数
    fi
    echo "奇数: $i"
done
```

---

## 5. 函数

### 定义和调用函数

```bash
#!/bin/bash

# 方式1：标准写法
function greet() {
    echo "Hello, $1!"
}

# 方式2：省略 function 关键字（推荐）
say_hello() {
    echo "你好, $1!"
}

# 调用函数（直接写函数名，不用括号）
greet "Alice"
say_hello "Bob"
```

### 函数参数

函数内部使用 `$1`, `$2`, `$@` 等获取参数 — 与脚本参数语法完全一致。

```bash
#!/bin/bash

# 计算两个数的和
add() {
    local result=$(( $1 + $2 ))
    echo $result  # 通过 echo 返回结果
}

# 获取函数返回值（捕获 echo 输出）
sum=$(add 10 20)
echo "10 + 20 = $sum"

# 函数可以访问所有传入参数
print_args() {
    echo "参数个数: $#"
    echo "所有参数: $@"
    for arg in "$@"; do
        echo "  -> $arg"
    done
}
print_args "apple" "banana" "cherry" "date"
```

### 返回值：return vs echo

```bash
#!/bin/bash

# return：返回退出状态码（0-255），表示成功/失败
is_even() {
    if (( $1 % 2 == 0 )); then
        return 0  # 成功（偶数）
    else
        return 1  # 失败（奇数）
    fi
}

if is_even 42; then
    echo "42 是偶数"
else
    echo "42 是奇数"  # 不会执行
fi

# echo：返回计算数据，用 $() 捕获
get_square() {
    echo $(( $1 * $1 ))
}
result=$(get_square 7)
echo "7 的平方 = $result"

# 同时返回数据和状态码
divide() {
    if [ "$2" -eq 0 ]; then
        echo "错误: 除数不能为0" >&2  # 错误信息输出到 stderr
        return 1
    fi
    echo $(( $1 / $2 ))              # 结果输出到 stdout
    return 0
}

if quotient=$(divide 10 2); then
    echo "10 / 2 = $quotient"
fi

if ! quotient=$(divide 10 0); then
    echo "除法失败"
fi
```

### 局部变量

使用 `local` 关键字限制变量作用域，避免污染全局命名空间。

```bash
#!/bin/bash

# 全局变量
GLOBAL_NAME="全局值"

demo_local() {
    local LOCAL_NAME="局部值"    # 只在函数内部可见
    GLOBAL_NAME="被修改的全局值"  # 修改了全局变量

    echo "函数内部 LOCAL_NAME: $LOCAL_NAME"
    echo "函数内部 GLOBAL_NAME: $GLOBAL_NAME"
}

demo_local
echo "函数外部 LOCAL_NAME: ${LOCAL_NAME:-未定义}"  # 未定义
echo "函数外部 GLOBAL_NAME: $GLOBAL_NAME"          # 被修改了
```

### 递归函数

```bash
#!/bin/bash

# 计算阶乘
factorial() {
    if [ "$1" -le 1 ]; then
        echo 1
    else
        local prev=$(factorial $(( $1 - 1 )))
        echo $(( $1 * prev ))
    fi
}

echo "5! = $(factorial 5)"   # 输出: 5! = 120

# 斐波那契数列
fibonacci() {
    if [ "$1" -le 1 ]; then
        echo "$1"
    else
        local a=$(fibonacci $(( $1 - 1 )))
        local b=$(fibonacci $(( $1 - 2 )))
        echo $(( a + b ))
    fi
}

echo "fib(10) = $(fibonacci 10)"  # 输出: fib(10) = 55
```

---

## 6. 数组

### 索引数组（Indexed Array）

```bash
#!/bin/bash

# 定义数组（多种方式）
fruits=("apple" "banana" "cherry")
fruits[3]="orange"          # 追加元素
fruits+=("grape" "mango")   # 批量追加

# 访问数组元素
echo "第一个: ${fruits[0]}"
echo "第三个: ${fruits[2]}"
echo "所有元素: ${fruits[@]}"
echo "所有元素(用*): ${fruits[*]}"   # 合并为一个字符串
echo "数组长度: ${#fruits[@]}"
echo "索引列表: ${!fruits[@]}"

# 遍历数组
echo "=== 遍历数组 ==="
for fruit in "${fruits[@]}"; do
    echo "  - $fruit"
done

# 遍历索引和值
echo "=== 带索引遍历 ==="
for index in "${!fruits[@]}"; do
    echo "  [$index] = ${fruits[$index]}"
done

# 切片（提取子数组）
echo "第2-4个元素: ${fruits[@]:1:3}"

# 删除元素
unset fruits[1]           # 删除索引1（banana）
echo "删除后: ${fruits[@]}"

# 删除整个数组
unset fruits
```

### 关联数组（Associative Array，类似字典）

> **需要 Bash 4.0+**

```bash
#!/bin/bash

# 声明关联数组
declare -A person

# 赋值
person[name]="Alice"
person[age]=25
person[city]="Beijing"

# 另一种赋值方式
declare -A scores=(
    [math]=95
    [english]=88
    [chinese]=92
)

# 访问
echo "姓名: ${person[name]}"
echo "数学: ${scores[math]}"

# 遍历所有键值对
echo "=== 个人信息 ==="
for key in "${!person[@]}"; do
    echo "  $key = ${person[$key]}"
done

# 遍历所有值
echo "=== 所有分数 ==="
for value in "${scores[@]}"; do
    echo "  $value"
done

# 检查键是否存在
if [[ -v scores[math] ]]; then
    echo "数学成绩已记录"
fi
```

---

## 7. 字符串操作

Bash 提供了强大的字符串（参数展开）操作。

```bash
#!/bin/bash
str="Hello, World! Hello, Bash!"

# --- 长度 ---
echo "字符串长度: ${#str}"

# --- 子串截取 ---
echo "从位置7开始: ${str:7}"        # World! Hello, Bash!
echo "前5个字符: ${str:0:5}"        # Hello
echo "倒数5个: ${str: -5}"          # Bash!  (注意空格)

# --- 模式匹配删除 ---
filename="backup.2024.tar.gz"
echo "去掉最短后缀: ${filename%.*}"        # backup.2024.tar（去掉 .gz）
echo "去掉最长后缀: ${filename%%.*}"       # backup（去掉 .2024.tar.gz）
echo "去掉最短前缀: ${filename#*.}"        # 2024.tar.gz（去掉 backup.）
echo "去掉最长前缀: ${filename##*.}"       # gz（去掉 backup.2024.tar.）

# --- 搜索替换 ---
msg="apple banana apple cherry"
echo "替换第一个: ${msg/apple/orange}"      # orange banana apple cherry
echo "替换所有: ${msg//apple/orange}"       # orange banana orange cherry
echo "删除所有apple: ${msg//apple/}"        #  banana  cherry

# --- 大小写转换 ---
text="Hello World"
echo "大写: ${text^^}"            # HELLO WORLD
echo "小写: ${text,,}"            # hello world
echo "首字母大写: ${text^}"       # Hello World（已是大写）
lower="hello world"
echo "首字母大写: ${lower^}"      # Hello world
```

---

## 8. 输入输出重定向

### 标准流

| 文件描述符 | 名称   | 默认指向 | 说明                     |
| ---------- | ------ | -------- | ------------------------ |
| `0`        | stdin  | 键盘     | 标准输入                 |
| `1`        | stdout | 终端屏幕 | 标准输出                 |
| `2`        | stderr | 终端屏幕 | 标准错误                 |

### 基本重定向

```bash
# 输出重定向（覆盖）
echo "Hello" > file.txt        # stdout → 文件（覆盖）
echo "Hello" 1> file.txt       # 同上（显式写法）

# 输出重定向（追加）
echo "World" >> file.txt       # stdout → 文件（追加）

# 错误重定向
ls nonexistent 2> error.log    # stderr → 文件

# 同时重定向 stdout 和 stderr
command > output.log 2>&1      # 都写入同一个文件
command &> output.log           # 同上（Bash 简写）

# 追加模式
command >> output.log 2>&1     # 追加写入
command &>> output.log          # 同上（Bash 简写）

# 丢弃输出
command > /dev/null 2>&1       # 静默运行，不显示任何输出
```

### 输入重定向和 Here Document

```bash
# 输入重定向
wc -l < file.txt               # 统计行数

# Here Document（多行字符串输入）
cat << 'EOF'
这是多行文本
变量不会展开（因为有引号）
$HOME 保持原样
EOF

# 不带引号的 heredoc（变量会展开）
cat << EOF
当前用户: $USER
当前目录: $PWD
家目录: $HOME
EOF

# 使用 heredoc 写入文件
cat > config.ini << 'EOF'
[database]
host=localhost
port=3306
user=root
EOF

# Here String（将字符串作为 stdin）
grep "pattern" <<< "search in this string"
read first second <<< "hello world"
echo "$first"   # hello
echo "$second"  # world
```

### 管道（Pipeline）

```bash
# 基本管道：将前一个命令的 stdout 作为后一个命令的 stdin
ls -l | grep ".txt"
ps aux | grep "nginx" | awk '{print $2}'
cat /var/log/syslog | grep "error" | wc -l

# 管道 + tee：同时输出到文件和终端
./long_script.sh | tee output.log               # 覆盖
./long_script.sh | tee -a output.log            # 追加

# 管道处理 stderr
./script.sh 2>&1 | grep "error"                 # stderr 也通过管道
./script.sh |& grep "error"                     # 同上（Bash 简写）
```

---

## 9. 错误处理与调试

### 安全模式设置

```bash
#!/bin/bash

# 推荐在每个脚本开头加上以下设置
set -euo pipefail

# 各选项含义：
# set -e : 任何命令失败（返回非0）时立即退出
# set -u : 使用未定义变量时报错退出
# set -o pipefail : 管道中任意一个命令失败，整个管道都算失败
# set -x : 调试模式（打印每个命令再执行）
```

### 调试技巧

```bash
#!/bin/bash

# 方式1：运行时开启调试
bash -x script.sh           # 整个脚本调试

# 方式2：脚本内部局部调试
set -x                      # 开启调试
# 这部分代码会被逐行打印
x=10
y=20
echo $(( x + y ))
set +x                      # 关闭调试

# 方式3：打印关键信息
echo "DEBUG: 当前变量 x=$x, y=$y" >&2  # 输出到 stderr

# 方式4：使用 trap 捕获错误
trap 'echo "错误发生在第 $LINENO 行"; exit 1' ERR

# 方式5：退出时自动清理
tempfile=$(mktemp)
trap 'rm -f "$tempfile"; echo "已清理临时文件"' EXIT
```

### 错误处理模式

```bash
#!/bin/bash
set -euo pipefail

# 模式1：检查命令执行结果
if ! command -v docker &> /dev/null; then
    echo "错误: docker 未安装" >&2
    exit 1
fi

# 模式2：自定义 die 函数
die() {
    echo "[FATAL] $1" >&2
    exit 1
}

[ -f "config.yaml" ] || die "配置文件 config.yaml 不存在！"
[ -d "/data" ] || die "数据目录 /data 不存在！"

# 模式3：检查上一条命令的退出状态
cp source.txt dest.txt
if [ $? -ne 0 ]; then
    echo "复制失败！" >&2
fi

# 模式4：捕获脚本中断信号（Ctrl+C）
trap 'echo "脚本被中断"; exit 130' INT
trap 'echo "脚本被终止"; exit 143' TERM
```

---

## 10. 综合实战案例

### 案例1：交互式菜单系统

```bash
#!/bin/bash
set -euo pipefail

show_menu() {
    echo "========================================="
    echo "          系统管理工具 v1.0"
    echo "========================================="
    echo "  1) 查看磁盘使用情况"
    echo "  2) 查看内存使用情况"
    echo "  3) 查看正在运行的进程"
    echo "  4) 查看系统运行时间"
    echo "  5) 退出"
    echo "========================================="
}

while true; do
    show_menu
    read -p "请输入选择 [1-5]: " choice

    case $choice in
        1)
            echo ""
            echo "--- 磁盘使用情况 ---"
            df -h
            echo ""
            read -p "按回车键继续..."
            ;;
        2)
            echo ""
            echo "--- 内存使用情况 ---"
            free -h
            echo ""
            read -p "按回车键继续..."
            ;;
        3)
            echo ""
            echo "--- Top 10 进程 (按CPU) ---"
            ps aux --sort=-%cpu | head -11
            echo ""
            read -p "按回车键继续..."
            ;;
        4)
            echo ""
            echo "--- 系统运行时间 ---"
            uptime
            echo ""
            read -p "按回车键继续..."
            ;;
        5)
            echo "再见!"
            exit 0
            ;;
        *)
            echo "无效选择，请重新输入"
            sleep 1
            ;;
    esac
done
```

### 案例2：批量文件处理脚本

```bash
#!/bin/bash
set -euo pipefail

# 批量重命名 .jpeg 文件为 .jpg
BATCH_RENAME() {
    local count=0
    for file in *.jpeg; do
        # 如果没有匹配的文件，跳过
        [ -e "$file" ] || continue

        new_name="${file%.jpeg}.jpg"
        mv -v "$file" "$new_name"
        count=$((count + 1))
    done
    echo "共重命名 $count 个文件"
}

# 批量压缩图片（需要 ImageMagick）
BATCH_RESIZE() {
    local max_width=${1:-800}
    local count=0

    for file in *.jpg *.png; do
        [ -e "$file" ] || continue
        output="resized_${file}"
        convert "$file" -resize "${max_width}x" "$output"
        count=$((count + 1))
        echo "已处理: $file -> $output"
    done
    echo "共处理 $count 个文件"
}

# 按扩展名统计文件数量
COUNT_BY_EXT() {
    echo "--- 文件类型统计 ---"
    find . -maxdepth 1 -type f | sed 's/.*\.//' | sort | uniq -c | sort -rn
}

# 主菜单
echo "批量文件处理工具"
echo "1) 批量重命名 .jpeg → .jpg"
echo "2) 批量压缩图片"
echo "3) 统计文件类型"
read -p "选择操作: " op

case $op in
    1) BATCH_RENAME ;;
    2) BATCH_RESIZE ;;
    3) COUNT_BY_EXT ;;
    *) echo "无效选择" ;;
esac
```

### 案例3：简单日志分析脚本

```bash
#!/bin/bash
set -euo pipefail

LOG_FILE="${1:-/var/log/syslog}"

# 检查日志文件是否存在
if [ ! -f "$LOG_FILE" ]; then
    echo "错误: 日志文件 $LOG_FILE 不存在" >&2
    exit 1
fi

echo "========== 日志分析报告 =========="
echo "文件: $LOG_FILE"
echo "大小: $(du -h "$LOG_FILE" | cut -f1)"
echo "总行数: $(wc -l < "$LOG_FILE")"
echo ""

# 统计各日志级别
echo "--- 日志级别统计 ---"
echo -n "ERROR: "; grep -c "ERROR" "$LOG_FILE" || echo "0"
echo -n "WARN:  "; grep -c "WARN" "$LOG_FILE" || echo "0"
echo -n "INFO:  "; grep -c "INFO" "$LOG_FILE" || echo "0"

# 最近 10 条错误
echo ""
echo "--- 最近 10 条 ERROR ---"
grep "ERROR" "$LOG_FILE" | tail -10

# 按小时统计日志量
echo ""
echo "--- 按小时分布 ---"
awk '{print $3}' "$LOG_FILE" | cut -d: -f1 | sort | uniq -c | sort -rn | head -10
```

### 案例4：守护进程监控脚本

```bash
#!/bin/bash
set -euo pipefail

# 监控指定进程，如果不存在则自动重启
MONITOR_PROCESS="nginx"
RESTART_CMD="sudo systemctl restart nginx"
CHECK_INTERVAL=10   # 检查间隔（秒）
MAX_RETRIES=3       # 最大重试次数

retry_count=0

log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1"
}

while true; do
    if pgrep -x "$MONITOR_PROCESS" > /dev/null; then
        # 进程正在运行，重置重试计数
        retry_count=0
    else
        log "WARNING: $MONITOR_PROCESS 未运行!"

        if [ $retry_count -lt $MAX_RETRIES ]; then
            log "尝试重启 (第 $((retry_count + 1))/$MAX_RETRIES 次)..."
            $RESTART_CMD
            retry_count=$((retry_count + 1))
        else
            log "FATAL: 已达到最大重试次数 $MAX_RETRIES，停止监控"
            exit 1
        fi
    fi

    sleep "$CHECK_INTERVAL"
done
```

---

## 附录：快捷参考表

### 常用测试汇总

```bash
# 整数比较
[ $a -eq $b ] [ $a -ne $b ] [ $a -gt $b ] [ $a -lt $b ] [ $a -ge $b ] [ $a -le $b ]

# 字符串
[ -z "$s" ]  [ -n "$s" ]  [ "$s1" = "$s2" ]  [ "$s1" != "$s2" ]

# 文件
[ -e "$f" ]  [ -f "$f" ]  [ -d "$d" ]  [ -r "$f" ]  [ -w "$f" ]  [ -x "$f" ]  [ -s "$f" ]

# 逻辑
[ cond1 -a cond2 ]  [ cond1 -o cond2 ]  [ ! cond ]
[[ cond1 && cond2 ]]  [[ cond1 || cond2 ]]  [[ ! cond ]]
```

### 预定义退出码

| 退出码 | 含义                         |
| ------ | ---------------------------- |
| `0`    | 成功                         |
| `1`    | 一般错误                     |
| `2`    | Shell 内置命令使用错误       |
| `126`  | 命令找到了但不可执行         |
| `127`  | 命令未找到                   |
| `128+n`| 进程收到信号 n 退出          |
| `130`  | 收到 Ctrl+C (SIGINT = 2)     |
| `137`  | 收到 SIGKILL (9)             |

---

> **参考资源**:
> - [Bash Reference Manual (GNU)](https://www.gnu.org/software/bash/manual/)
> - [ShellCheck](https://www.shellcheck.net/) — 在线 Shell 脚本静态检查工具
> - [explainshell.com](https://explainshell.com/) — 解释 Shell 命令
